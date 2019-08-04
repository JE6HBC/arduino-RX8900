
#include <arduino-RX8900.h>

//USE +5V PWR
//AE-RX8900 SCL  =  A5
//AE-RX8900 SDA  =  A4
//AE-RX8900 FOE  = +5V
//AE-RX8900 VBAT = +5V

//"TEMP" IS NOT OUTSIDE TEMP. IT IS INTERNAL CORE TEMP OF RX8900.
//WHEN TIMER COUNT OVER,RX8900A's INT-PIN LET LOW 7.813mS THEN Hi-Z 

/*
 COMMAND 
 A = TOKEI
 S = ALARM SET
 D = ALARM ON
 F = ALARM OFF
 G = ALARM STOP
 H = TIMER SET
 J = TIMER START
 K = TIMER HALT

 USE SERIAL MONITOR,　SET 'CR' & 9600bps
 USE TERMINAL SOFT, SET "LOCAL ECHO"
 */

#include "Wire.h"

void setup()
{
  delay(100);
  Wire.begin();
  Serial.begin(9600);
  Serial.println("READY!");
  Serial.println("KEYBOARD FUNCTIONS ARE  ");
  Serial.println("A = TOKEI. S = ALARM SET."); 
  Serial.println("D = ALARM ON. F = ALARM OFF. G = ALARM STOP.");
  Serial.println("H = TIMER SET. J = TIMER START. K = TIMER HALT.");
  Serial.println();
  INIT_RX8900A(); 
  INPUT_DATE();            
 // SYORI_FLAG = TOKEI;
  SET_DATE();
  COUNT_START();
}
//*******************************************************
void loop()
{
  MODE_SELECT();
  SYORI_ALL();
}
