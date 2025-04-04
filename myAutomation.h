/* This is an automation example file.
 *  The presence of a file called "myAutomation.h" brings EX-RAIL code into
 *  the command station.
 *  The automation may have multiple concurrent tasks.
 *  A task may 
 *  - Act as a ROUTE setup macro for a user to drive over 
 *  - drive a loco through an AUTOMATION 
 *  - automate some cosmetic part of the layout without any loco.
 *  
 *  At startup, a single task is created to execute the startup sequence.
 *  This task may simply follow a route, or may START  
 *  further tasks (that is.. send a loco out along a route).
 *  
 *  Where the loco id is not known at compile time, a new task 
 *  can be created with the command:
 *  </ START [cab] route> 
 *  
 *  A ROUTE, AUTOMATION or SEQUENCE are internally identical in ExRail terms  
 *  but are just represented differently to a Withrottle user:
 *  ROUTE(n,"name") - as Route_n .. to setup a route through a layout
 *  AUTOMATION(n,"name") as Auto_n .. to send the current loco off along an automated journey
 *  SEQUENCE(n) is not visible to Withrottle.
 *  
 */
HAL_IGNORE_DEFAULTS

#include "IODevice.h"
#include "IO_HALDisplay.h"



HAL(MCP23017,200,16,0X27)
HAL(PCA9685,100,16,0X40)



SERVO_TURNOUT(TRN1,100,330,290,Slow,"Access")
SERVO_TURNOUT(TRN2,101,300,250,Slow,"Exit")
SERVO_TURNOUT(TRN3,102,230,300,Fast,"Header")


SIGNAL(104,0,105) //stn exit
SIGNAL(106,0,107) //entry
SIGNAL(108,0,109) //stn
SIGNAL(110,0,111) //through


//Start of demonstration layout sequences
// taken from the big picture
//Turnout Aliases
ALIAS(TRN1, 100)
ALIAS(TRN2, 101)
ALIAS(TRN3, 102)
//Sensor Aliases
ALIAS(SNS1_TRN1_APP, 204)
ALIAS(SNS2_MAIN_TRN1_EX, 202)
ALIAS(SNS3_STN, 205)
ALIAS(SNS4_MAIN_TRN2_APP, 200)
ALIAS(SNS5_STN_TRN2_APP, 201)
ALIAS(SNS6_TRN2_EX, 206)
//Signal Aliases
ALIAS(SIG1_TRN1_APP, 106)
ALIAS(SIG2_TRN2_GO, 110)
ALIAS(SIG3_STN_EX, 104)
ALIAS(STN_STOP,108)
//Block Aliases
ALIAS(BLK1_TRN1_APP, 1)
ALIAS(BLK2_MAIN_HOLD, 2)
ALIAS(BLK3_STN, 3)
ALIAS(BLK4_TRN2_EX, 4)
ALIAS(BLK3_STN_EX, 5)
//Sequence Aliases
ALIAS(BLK1_EXIT, 1)
ALIAS(BLK1_BLK2, 2)
ALIAS(BLK1_BLK3, 3)
ALIAS(BLK2_BLK4, 4)
ALIAS(BLK3_BLK4, 5)
ALIAS(BLK4_BLK1, 6)
ALIAS(CHOOSE_BLK2, 60)

ONTHROW(TRN1) THROW(TRN3) DONE 
ONCLOSE(TRN1) CLOSE(TRN3) DONE
// Start up with turnouts/points closed and signals red
CLOSE(TRN1)
CLOSE(TRN2)
CLOSE(TRN3)
RED(SIG1_TRN1_APP)
RED(SIG2_TRN2_GO)
RED(SIG3_STN_EX)

//JMRI_SENSOR(200,7)



AUTOSTART
// Reserve main blocks for loco locations.
RESERVE(BLK1_TRN1_APP)  //Loco 1
RESERVE(BLK2_MAIN_HOLD) //Loco 2
RESERVE(BLK4_TRN2_EX)   //Loco 3
// Set locos into correct locations

SENDLOCO(2604, BLK1_EXIT) // Before TRN 1
SENDLOCO(6417, BLK2_BLK4) // Before TRN 2
SENDLOCO(4331, BLK4_BLK1) // After TRN 2

POWERON 
DONE // Complete setup of layout.


//Main sequences to get the trains rolling after each other
// Sequence to exit block 1, and choose whether to go to the station or continue on main
SEQUENCE(BLK1_EXIT)
  IF(CHOOSE_BLK2)
    UNLATCH(CHOOSE_BLK2)
    FOLLOW(BLK1_BLK2)
  ELSE
    LATCH(CHOOSE_BLK2)
    FOLLOW(BLK1_BLK3)
  ENDIF

// Sequence to go from block 1 to block 2
SEQUENCE(BLK1_BLK2)
  RESERVE(BLK2_MAIN_HOLD)
  PRINT("RESERVED BLK2_MAIN_HOLD(2)")
  IFTHROWN(TRN1)
    RED(SIG1_TRN1_APP)
    CLOSE(TRN1)
    DELAY(2000)
  ENDIF
  GREEN(SIG1_TRN1_APP)
  FWD(20)
  AFTER(SNS2_MAIN_TRN1_EX)
  RED(SIG1_TRN1_APP)
  PRINT("SENSOR SNS2_MAIN_TRN1_EX(202)")
  FREE(BLK1_TRN1_APP)
  PRINT("FREE BLK1_TRN1_APP(1)")
  FOLLOW(BLK2_BLK4)

// Sequence to go from block 1 to block 3
SEQUENCE(BLK1_BLK3)
  RESERVE(BLK3_STN)
  PRINT("RESERVED BLK3_STN(3)")
  IFCLOSED(TRN1)
    RED(SIG1_TRN1_APP)
    THROW(TRN1)
    DELAY(2000)
  ENDIF
  GREEN(SIG1_TRN1_APP)
  FWD(10)
  RED(STN_STOP)
  AT(SNS3_STN)
  PRINT("SENSOR SNS3_STN(205)")
  STOP
  FREE(BLK1_TRN1_APP)
  PRINT("FREE BLK1_TRN1_APP(1)")
  DELAYRANDOM(30000, 60000)
  PRINT("Move!!!")
  GREEN(STN_STOP)
  FWD(10)
  //RED(STN_STOP)
  FOLLOW(BLK3_BLK4)

// Sequence to go from block 2 to block 4
SEQUENCE(BLK2_BLK4)
  RESERVE(BLK4_TRN2_EX)
  PRINT("RESERVED BLK4_TRN2_EX(4)")
  IFTHROWN(TRN2)
    RED(SIG2_TRN2_GO)
    RED(SIG3_STN_EX)
    CLOSE(TRN2)
    DELAY(2000)
  ENDIF
  GREEN(SIG2_TRN2_GO)
  FWD(20)
  AFTER(SNS6_TRN2_EX)
  PRINT("AFTER SENSOR SNS6_TRN2_EX(206)")
  FREE(BLK2_MAIN_HOLD)
  PRINT("FREE BLK2_MAIN_HOLD(2)")
  FOLLOW(BLK4_BLK1)

// Sequence to go from block 3 to block 4
SEQUENCE(BLK3_BLK4)
 AFTER(SNS5_STN_TRN2_APP)
  RESERVE(BLK4_TRN2_EX) 
  PRINT("SENSOR SNS5_STN_TRN2_APP(201)")
  PRINT("RESERVED BLK4_TRN2_EX(4)")
  IFCLOSED(TRN2)
    RED(SIG2_TRN2_GO)
    RED(SIG3_STN_EX)
    THROW(TRN2)
    DELAY(2000)
  ENDIF
  GREEN(SIG3_STN_EX)
  FWD(20)
  AFTER(SNS6_TRN2_EX)
  PRINT("AFTER SENSOR SNS6_TRN2_EX(206)")
  FREE(BLK3_STN)
  PRINT("FREE BLK3_STN(3)")
  FOLLOW(BLK4_BLK1)

// Sequence to move from block 4 back to block 1
SEQUENCE(BLK4_BLK1)
  RESERVE(BLK1_TRN1_APP)
  PRINT("RESERVE BLK1_TRN1_APP")
  FWD(30)
  AFTER(SNS1_TRN1_APP)
  PRINT("AFTER SENSOR SNS1_TRN1_APP(204)")
  FREE(BLK4_TRN2_EX)
  PRINT("FREE BLK4_TRN2_EX(4)")
  FOLLOW(BLK1_EXIT)


/*
// This is the startup sequence, 
AUTOSTART
POWERON        // turn on track power
SENDLOCO(3,1) // send loco 3 off along route 1
SENDLOCO(10,2) // send loco 10 off along route 2
DONE     // This just ends the startup thread, leaving 2 others running.

/* SEQUENCE(1) is a simple shuttle between 2 sensors      
 *  S20 and S21 are sensors on arduino pins 20 and 21 
 *  S20                    S21                   
 *  === START->================
 */
/*
   SEQUENCE(1) 
     DELAY(10000)   // wait 10 seconds
     FON(3)       // Set Loco Function 3, Horn on
     DELAY(1000)    // wait 1 second
     FOFF(3)      // Horn off
     FWD(80)      // Move forward at speed 80
     AT(210)       // until we hit sensor id 21
     STOP         // then stop
     DELAY(5000)    // Wait 5 seconds
     FON(2)       // ring bell
     REV(60)      // reverse at speed 60
     AT(200)       // until we get to S20
     STOP         // then stop
     FOFF(2)      // Bell off 
     FOLLOW(1)    // and follow sequence 1 again
   */
/* SEQUENCE(2) is an automation example for a single loco Y shaped journey
 *  S31,S32,S33 are sensors, T4 is a turnout
 *  
 *  S33                      T4                            S31
 *  ===-START->=============================================
 *                          //
 *  S32                    //
 *  ======================//
 *  
 *  Train runs from START to S31, back to S32, again to S31, Back to start.
 */
/*
  SEQUENCE(2)
   FWD(60)     // go forward at DCC speed 60 
   AT(310) STOP  // when we get to sensor 31 
   DELAY(10000)  // wait 10 seconds 
   THROW(4)    // throw turnout for route to S32
   REV(45)     // go backwards at speed 45
   AT(320) STOP  // until we arrive at sensor 32
   DELAY(5000)   // wait 5 seconds
   FWD(50)     // go forwards at speed 50
   AT(310) STOP  // and stop at sensor 31
   DELAY(5000)   // wait 5 seconds 
   CLOSE(4)    // set turnout closed
   REV(50)     // reverse back to S3
   AT(330) STOP
   DELAY(20000)  // wait 20 seconds 
   FOLLOW(2)   // follow sequence 2... ie repeat the process
 */

 ONOVERLOAD(A)
    SCREEN(0,0,"OVERLOAD")
    SCREEN(0,1,"POWER OFF")
    PRINT("Overload Detected on A - Turn Off Power")
    SET_TRACK(A, NONE)
    AFTEROVERLOAD(A)
      SCREEN(0,0,"Restored District A")
      SCREEN(0,1,"  POWER ON")
      PRINT("Overload cleared on District A - Power Restored")
      DELAY(2000)
      SCREEN(0,0,"")
      SCREEN(0,1,"")
DONE

ROUTE(10,"Power Reset")
    SCREEN(0,1,"Reseting Power")
    SET_TRACK(A, MAIN)
    POWERON
    DELAY(5000)
    SCREEN(0,1,"")
DONE