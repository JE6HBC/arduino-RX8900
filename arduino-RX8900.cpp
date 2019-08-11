/*
arduino-RX8900.cpp 
*/


#include "arduino-RX8900-lib.h"

unsigned char SEC     = 0x00;//0x00-0x59
unsigned char MIN     = 0x00;//0x00-0x59
unsigned char HOUR    = 0x00;//0x00-0x23
unsigned char WEEK    = 0x01;//SUN=0x01,MON=0x02,TUE=0x04,WED=0x08,THU=0x10,FRI=0x20,SAT=0x40
unsigned char DAY     = 0x01;//0x00-0x28～0x31)
unsigned char MONTH   = 0x01;//JUN=0x01,FEB=0x02,MAR=0x03,APR=0x04,MAY=0x05,JUN=0x06,JLY=0x07,AUG=0x08,SEP=0x09,OCT=0x10,NOV=0x11,DEC=0x12
unsigned char YEAR    = 0x00;//0x00-0x99
unsigned char RAM;
unsigned char MIN_Alarm;
unsigned char HOUR_Alarm;
unsigned char WEEK_DAY_Alarm;
unsigned char Timer_Counter_0;
unsigned char Timer_Counter_1;
unsigned char Extension_Register;
unsigned char Flag_Register;
unsigned char Control_Register;
unsigned char TEMP;
float         TEMP_RESULT;
unsigned char Backup_Function;
unsigned char DUMMY;
unsigned char OLD_SEC = 0;
unsigned char OLD_Timer_Counter_0 = 0;
String MONTH_S = "Jan.Feb.Mar.Apr.May.Jun.Jly.Aug.Sep.dmy.dmy.dmy.dmy.dmy.dmy.Oct.Nov.Dec.";
String WEEK_S  = "Sun.Mon.Tue.Wed.Thu.Fri.Sat.";
unsigned char RX_DATA;
unsigned long RX_RESULT;
unsigned char RX_FLAG;
unsigned char DATA_OK_FLAG;
unsigned char BAD_DATA_FLAG;
unsigned char ALARM_BUFFER;
unsigned char str_data[8];
unsigned long WEEK_ALARM_BUFFER;
unsigned int TIMER_VALUE;
enum SYORI_FLAG SYORI_FLAG;


//*******************************************************

void COUNT_START(void)
{  
  ByteWrite(SEC_reg,0x00);
  RESET_RX8900();
}
//--------------------------------------------------------
void INIT_RX8900A(void)
{
  ByteWrite(Extension_Register_reg,0x08);//set WEEK ALARM , 1Hz to FOUT
  ByteWrite(Flag_Register_reg,     0x00);//reset all flag
  RESET_RX8900();
}
//--------------------------------------------------------
void RegisterRead(void)
{
  Wire.beginTransmission(RX8900A_ADRS);
  Wire.write(SEC_reg);//set 0x00(SEC)Register address
  Wire.endTransmission(false);
  Wire.requestFrom(RX8900A_ADRS, Control_Register_reg); //set 0x0F(Control Register)Register address
  SEC = Wire.read();                //0x00
  MIN = Wire.read();                //0x01
  HOUR = Wire.read();               //0x02
  WEEK = Wire.read();               //0x03
  DAY = Wire.read();                //0x04
  MONTH = Wire.read();              //0x05
  YEAR = Wire.read();               //0x06
  RAM = Wire.read();                //0x07
  MIN_Alarm = Wire.read();          //0x08
  HOUR_Alarm = Wire.read();         //0x09
  WEEK_DAY_Alarm = Wire.read();     //0x0A
  Timer_Counter_0 = Wire.read();    //0x0B
  Timer_Counter_1 = Wire.read();    //0x0C
  Extension_Register = Wire.read(); //0x0D
  Flag_Register = Wire.read();      //0x0E
  Control_Register = Wire.read();   //0x0F
  Wire.beginTransmission(RX8900A_ADRS);
  Wire.write(TEMP_reg);             //set TEMP_reg 0x17
  Wire.endTransmission(false);
  Wire.requestFrom(RX8900A_ADRS, 2);//Backup_Function_reg ?` TEMP_reg
  TEMP = Wire.read();               //0x17
  Backup_Function = Wire.read();    //0x18
}
//------------------------------------------------------
void RESET_RX8900(void)
{
  ByteWrite(Control_Register_reg,(ByteRead(Control_Register_reg) | 0b00000001));
}
//---------------------------
void SET_AIE(void)           //ALARM INTERRUPT ENABLE
{
  ByteWrite(Control_Register_reg,(ByteRead(Control_Register_reg) | 0b00001000));
}
//---------------------------
void RESET_AIE(void)         //ALARM INTERRUPT DISABLE
{
  ByteWrite(Control_Register_reg,(ByteRead(Control_Register_reg) & 0b11110111));
}
//---------------------------
void RESET_AF(void)          //RESET ALARM FLAG
{
  ByteWrite(Flag_Register_reg,(ByteRead(Flag_Register_reg) & 0b11110111));
}
//---------------------------
char IS_AF(void)             //IS ALARM TIME?
{
  return (ByteRead(Flag_Register_reg) & 0b00001000);
}
//---------------------------
void SET_TIE(void)           //TIMER INTERRUPT ENABLE
{
  ByteWrite(Control_Register_reg,(ByteRead(Control_Register_reg) | 0b00010000));
}
//---------------------------
void RESET_TIE(void)         //TIMER INTERRUPT DISABLE
{
  ByteWrite(Control_Register_reg,(ByteRead(Control_Register_reg) & 0b11101111));
}
//---------------------------
void RESET_TF(void)          //RESET TIMER FLAG
{
  ByteWrite(Flag_Register_reg,(ByteRead(Flag_Register_reg) & 0b11101111));
}
//---------------------------
char IS_TF(void)             //IS TIMER COUNT UP?
{
  return (ByteRead(Flag_Register_reg) & 0b00010000);
}
//---------------------------
void SET_TE(void)            //TIMER ENABLE
{
  ByteWrite(Extension_Register_reg,(ByteRead(Extension_Register_reg) | 0b00010000));
}
//---------------------------
void RESET_TE(void)          //TIMER DISABLE
{
  ByteWrite(Extension_Register_reg,(ByteRead(Extension_Register_reg) & 0b11101111));
}
//---------------------------
void SET_TSEL_1S(void)       //SET TIMER INTERVAL 1sec
{
  ByteWrite(Extension_Register_reg,((ByteRead(Extension_Register_reg) & 0b11111100)|0b00000010));
}
//----------------------------
void PRINT_YES(void)
{
  #ifdef TERMINAL
    Serial.println();
  #endif
  Serial.print("YES");
}
//----------------------------
void PRINT_NO(void)
{
  #ifdef TERMINAL
    Serial.println();
  #endif
  Serial.print("NO");
}
//========================================================================
void READ_AND_TX(void)
{
  RegisterRead();
  if(SEC != OLD_SEC) //COMPARE SEC to OLD_SEC,
  {
    TX_DATE();       //IF SEC++, DISPLAY DATE
    OLD_SEC = SEC;
  }
}
//---------------------------------------------------------------------
void READ_AND_TX_2(void)
{
  RegisterRead();
  if(IS_TF())SYORI_FLAG = TIMER_OVER;
  if(SEC != OLD_SEC) //COMPARE SEC to OLD SEC
  {
    /* DISP COUNT DOWN
    if(TIMER_VALUE > 0)TIMER_VALUE--; //DECREMENT TIMER VALUE
    TX_COUNTER();                     //DISPLAY TIMER VALUE
    */
    TX_DATE();
    OLD_SEC = SEC;
  }
}
//============================================================
void ALARM_SET_SYORI(void)
{
  int i;
  unsigned long result = 0;
  unsigned char uc;
  WEEK_DAY_Alarm = 0;
  Serial.println();
  Serial.println("SET ALARM DAY.(YES=1,NO=2)and 'CR'");
  Serial.println("SUN.?");
loop1:
  result = RX_ALL();
  switch(result)
  {
    case 0x01:
      WEEK_DAY_Alarm = SUN;
      PRINT_YES();
      break;
    case 0x02:
      WEEK_DAY_Alarm = NO_WEEK;
      PRINT_NO();
      break;
    default:
      goto loop1;
      break;
  }
  Serial.println();
  Serial.println("MON.?");
loop2:
  result = RX_ALL();
  switch(result)
  {
    case 0x01:
      WEEK_DAY_Alarm = WEEK_DAY_Alarm += MON;
      PRINT_YES();
      break;
    case 0x02:
      PRINT_NO();
      break;;
    default:
      goto loop2;
      break;
  }
  Serial.println();
  Serial.println("TUE.?");
loop3:
  result = RX_ALL();
  switch(result)
  {
    case 0x01:
      WEEK_DAY_Alarm = WEEK_DAY_Alarm += TUE;
      PRINT_YES();;
      break;
    case 0x02:
      PRINT_NO();;
      break;;
    default:
      goto loop3;
      break;
  }
  Serial.println();
  Serial.println("WED.?");
loop4:
  result = RX_ALL();
  switch(result)
  {
    case 0x01:
      WEEK_DAY_Alarm = WEEK_DAY_Alarm += WED;
      PRINT_YES();
      break;
    case 0x02:
      PRINT_NO();
      break;;
    default:
      goto loop4;
      break;
  }
  Serial.println();
  Serial.println("THU.?");
loop5:
  result = RX_ALL();
  switch(result)
  {
    case 0x01:
      WEEK_DAY_Alarm = WEEK_DAY_Alarm += THU;
      PRINT_YES();
      break;
    case 0x02:
      PRINT_NO();
      break;;
    default:
      goto loop5;
      break;
  }
  Serial.println();
  Serial.println("FRI.?");
loop6:
  result = RX_ALL();
  switch(result)
  {
    case 0x01:
      WEEK_DAY_Alarm = WEEK_DAY_Alarm += FRI;
      PRINT_YES();
      break;
    case 0x02:
      PRINT_NO();
      break;;
    default:
      goto loop6;
      break;
  }
  Serial.println();
  Serial.println("SAT.?");
loop7:
  result = RX_ALL();
  switch(result)
  {
    case 0x01:
      WEEK_DAY_Alarm = WEEK_DAY_Alarm += SAT;
      PRINT_YES();
      break;
    case 0x02:
      PRINT_NO();
      break;
    default:
      goto loop7;
      break;
  }
  Serial.println();
  Serial.println();
  ByteWrite(WEEK_DAY_Alarm_reg,WEEK_DAY_Alarm);
  ALARM_BUFFER = WEEK_DAY_Alarm;
  Serial.println("ALARM SET ON --");
  if(WEEK_DAY_Alarm == 0)
  {
    Serial.println("-- THERE IS NO ALARM DAY --");
    Serial.println();
    return;
  }
  else
  {
    for(uc=0;uc<7;uc++)
    {
      if(((WEEK_DAY_Alarm >> uc) & 0x01)==0x01) 
      {
        Serial.print(WEEK_S.substring((uc*4),(uc*4)+4));    
        Serial.print(" ");
      }
    }
  }
  Serial.println();
  Serial.println("SET ALARM HOUR(0?`23)");
  while(INPUT_HOUR() == -1);
  ByteWrite(HOUR_Alarm_reg,HOUR);
  Serial.println();
  Serial.println("SET ALARM MINUTE(0?`59)");
  while(INPUT_MIN() == -1);
  ByteWrite(MIN_Alarm_reg,MIN);
  Serial.println();
  Serial.println("ALARM SET ON");
  for(uc=0;uc<7;uc++)
  {
    if(((WEEK_DAY_Alarm >> uc) & 0x01)==0x01)
    {
      Serial.print(WEEK_S.substring((uc*4),(uc*4)+4));    
      Serial.print(" ");
    }
  }
  Serial.println();
  Serial.print(ByteRead(HOUR_Alarm_reg),HEX);
  Serial.print(":");
  uc = ByteRead(MIN_Alarm_reg);
  if(uc < 0x0A)          //IF 0 to 9
  {
    Serial.print('0');  //ADD '0' to DISPLAY
    Serial.println(uc,HEX);
  }
  else Serial.println(uc,HEX); 
  Serial.println();
}
//--------------------------------------------------------
void TIMER_SET_SYORI(void)
{
  unsigned int  ui;
  Serial.println("DOWN TIMER MODE (0 min 1 sec ?` 59 min 59 sec) ");
  Serial.println("SET MINUTE (0?`59) and 'CR'");
  while(Serial.available() > 0)ui = Serial.read();
  while(INPUT_MIN() == -1);
  ui = ((((MIN >> 4)& 0x0F) * 10) + (MIN & 0x0F)) * 60;
  Serial.println("SET SECOND (0?`59) and 'CR'");
  while(INPUT_MIN() == -1);
  ui = ui + ((((MIN >> 4) & 0x0F) * 10) + (MIN & 0x0F));
  SET_TSEL_1S();
  RESET_TE();
  TIMER_VALUE = ui;
  ByteWrite(Timer_Counter_0_reg,(unsigned char)( ui       & 0xFF));
  ByteWrite(Timer_Counter_1_reg,(unsigned char)((ui >> 8) & 0x0F));
  Serial.print("TOTAL SECONDS = ");
  Serial.println(ui);
}
//**********************************************************
void SYORI_ALL(void)
{
  unsigned char uc;
  switch(SYORI_FLAG)
  {
    case TOKEI:
      RESET_TE();
      RESET_TF();
      RESET_TIE();
      SYORI_FLAG = TOKEI_COUNT;
      break;
    case TOKEI_COUNT:
      READ_AND_TX();
      break;
    case ALARM_SET:
      Serial.println();
      Serial.println("ALARM SET");
      ALARM_SET_SYORI();
      SET_AIE();
      RESET_AF();
      SYORI_FLAG = TOKEI_COUNT;
      break;
    case ALARM_ON:
      Serial.println("ALARM ON");
      ByteWrite(WEEK_DAY_Alarm_reg,ALARM_BUFFER); //RELOAD ALARM DAY of WEEK
      SET_AIE();
      RESET_AF();
      SYORI_FLAG = TOKEI_COUNT;
      break;
    case ALARM_OFF:
      Serial.println("ALARM OFF");
      ALARM_BUFFER = ByteRead(WEEK_DAY_Alarm_reg);
      ByteWrite(WEEK_DAY_Alarm_reg,NO_WEEK); //RESET ALL DAY of WEEK
      RESET_AIE();
      RESET_AF();
      SYORI_FLAG = TOKEI_COUNT;
      break;
    case ALARM_STOP:  //INT-PIN LET Hi-Z
      Serial.println("ALARM STOP");
      RESET_AF();
      SYORI_FLAG = TOKEI_COUNT;
      break;
    case TIMER_SET:
      Serial.println("TIMER SET");
      TIMER_SET_SYORI();
      SET_TIE();
      RESET_TF();
      RESET_TE();
      SYORI_FLAG = TOKEI_COUNT;
      break;
    case TIMER_START:
      Serial.println("TIMER START");
      TIMER_VALUE = ((unsigned int)ByteRead(Timer_Counter_1_reg) * 60)+ (unsigned int)ByteRead(Timer_Counter_0_reg);
      RESET_TE();
      SET_TSEL_1S();
      SET_TIE();
      SET_TE();
      SYORI_FLAG = TIMER_COUNT;
      break;
    case TIMER_COUNT:
      READ_AND_TX_2();
      break;
    case TIMER_STOP:
      Serial.println("TIMER STOP");
      RESET_TE();
      RESET_TIE();
      RESET_TF();
      SYORI_FLAG = TIMER_HALT;
      break;
    case TIMER_HALT:
      SYORI_FLAG = TOKEI_COUNT;
      break;
    case TIMER_OVER:  //INT-PIN LET LOW 7.813mS THEN Hi-Z
      Serial.println("TIMER COUNT OVER ! ");
      Serial.println();
      Serial.println();
      RESET_TE();
      RESET_TF();
      RESET_TIE();
      SYORI_FLAG = TIMER_HALT;                                    //
      break;
    defalut:
      break;
  }
}
//*********************************************************
void MODE_SELECT(void)
{
  unsigned long result = 0;
  result = Serial.read();
  switch(result)
  {
    case 'A': //TOKEI
    case 'a':
      SYORI_FLAG = TOKEI;
      break;
    case 'S': //ALARM SET
    case 's':
      SYORI_FLAG = ALARM_SET;
      break;
    case 'D': //ALARM ON
    case 'd':
      SYORI_FLAG = ALARM_ON;
      break;
    case 'F': //ALARM OFF
    case 'f':
      SYORI_FLAG = ALARM_OFF;
      break;
    case 'G': //ALARM STOP
    case 'g':
      SYORI_FLAG = ALARM_STOP;
      break;
    case 'H': //TIMER SET
    case 'h':
      SYORI_FLAG = TIMER_SET;
      break;
    case 'J': //TIMER START
    case 'j':
      SYORI_FLAG = TIMER_START;
      break;
    case 'K': //TIMER HALT
    case 'k':
      SYORI_FLAG = TIMER_STOP;
      break;  
    default:
      break;
  }
}
//*******************************************************
void TX_VALUE(unsigned char data)
{
  if (data < 0x0F)Serial.print('0'); 
  Serial.print(data, HEX);
}
//--------------------------------------------------------
unsigned char GET_WEEK(unsigned char data)
{
  unsigned char c = 0;
  while((data & 0x01)!= 0x01)
  {
    c++;
    data = data >> 1;  
  }
  return c;
}
//-------------------------------------------------------
unsigned long RX_ALL(void)
{
  unsigned long rx_result = 0;
  unsigned char rx_flag   = 0;
  unsigned char rx_data   = 0;
  while(rx_flag == 0)
  {
    if(Serial.available() > 0)
    {
      rx_data = Serial.read();
      if(rx_data == 0x0D)
      {
        rx_flag = 1;
      }
      else 
      {
        if((rx_data >= '0') && (rx_data <= '9'))
        {
          rx_result = rx_result * 16 + (rx_data - 0x30);
          rx_flag = 0;
        }
        else
        {
          rx_result = rx_result * 16 ;
          rx_flag = 0;
        }
      }
    }
  }
  rx_data = Serial.read();//read 0x0A(LF)for dummy
  return rx_result;
}
//-------------------------------------------------------
int INPUT_YEAR(void)
{
  unsigned long result = 0;
  result = RX_ALL() & 0xFFFF;
  if((result > 0x2099)||(result < 0x2000))
  {
    Serial.println("fail");
    return -1;
  }
  else
  {
    YEAR = (unsigned char)(result & 0xFF);
    Serial.println(result,HEX);
  }
  return 0;
}
//--------------------------------------------------------
int INPUT_MONTH(void)
{
  unsigned long result = 0;
  result = RX_ALL() & 0xFF;
  if((result > 0x12)||(result == 0x00))
  {
    Serial.println("fail");
    return -1;
  }
  else
  {
    Serial.print(result,HEX);
    Serial.print("    ");
    MONTH = (unsigned char)result;
    Serial.println(MONTH_S.substring((MONTH - 1) * 4,((MONTH - 1) * 4) + 4));
  }
  return 0;
}
//-------------------------------------------------------
int INPUT_DAY(void)
{
  unsigned long result = 0;
//  result = RX_ALL()& 0x3F;
  switch(MONTH)
  {
    case 0x01: //HAS 31 DAYS
    case 0x03:
    case 0x05:
    case 0x07:
    case 0x08:
    case 0x10:
    case 0x12:
      Serial.println("INPUT DAY ( 1 to 31 ) and 'CR'");
      result = RX_ALL()& 0x3F;
      if((result > 0x31)||(result == 0))
      {
        Serial.println("fail");
        return -1;
      }
      else
      {
        DAY = (unsigned char)result;
      }
      break;
    case 0x02: 
      if((YEAR % 4)== 0) //LEAP YEAR(URUU DOSHI)
      {
        Serial.println("INPUT DAY (1to29) and 'CR'");
        result = RX_ALL()& 0x3F;
        if((result > 0x29)||(result == 0))
        {
          Serial.println("fail");
          return -1;
        }
        else
        {
          DAY = (unsigned char)result;
        }
      }
      else //COMMON YEAR
      {
        Serial.println("INPUT DAY (1to28) and 'CR'");
        result = RX_ALL()& 0x3F;
        if((result > 0x28)||(result == 0))
        {
          Serial.println("fail");
          return -1;
        }
        else
        {
          DAY = (unsigned char)result;
        }
      }
      break;
    case 0x04: //HAS 30 DAYS
    case 0x06:
    case 0x09:
    case 0x11:
      Serial.println("INPUT DAY (1to30) and 'CR'");
      result = RX_ALL()& 0x3F;
      if((result > 0x30)||(result == 0))
      {
        Serial.println("fail");
        return -1;  
      }
      else
      {
        DAY = (unsigned char)result;
      }
      break;
    default:
      Serial.println("fail");
      return -1;
      break;
  }
  Serial.println(DAY,HEX);
  return 0;
}
//------------------------------------------------------
int INPUT_WEEK(void)
{
  unsigned long result = 0;
  result = RX_ALL() & 0x07; //GET 3BIT
  if(result > 0x06)
  {
    Serial.println("fail");
    return -1;  
  }
  else
  {
    Serial.print(result,HEX);
    Serial.print("    ");
    WEEK = 0x01;
    WEEK = WEEK << result;
    Serial.println(WEEK_S.substring((GET_WEEK(WEEK)*4),((GET_WEEK(WEEK)*4)+ 4)));
  }
  return 0;
}
//--------------------------------------------------------
int INPUT_HOUR(void)
{
  unsigned long result = 0;
  result = RX_ALL() & 0x3F;
  if(result > 0x23)
  {
    Serial.println("fail");
    return -1;
  }
  else
  {
    Serial.println(result,HEX);
    HOUR = (unsigned char)result;
  }
  return 0;
}
//-------------------------------------------------------
int INPUT_MIN(void)
{
  unsigned long result = 0;
  unsigned char ret_flag = 0;
  result = RX_ALL() & 0x7F;
  if(result > 0x59)
  {
    Serial.println("fail");
    ret_flag = -1;
  }
  else
  {
    Serial.println(result,HEX);
    MIN = (unsigned char)result;
  }
  ret_flag = 0;
  return ret_flag;
}
//*******************************************************
void INPUT_DATE(void)
{
  Serial.println("INPUT YEAR (2000 to 2099) and 'CR'");
  while(INPUT_YEAR()== -1);
  Serial.println("INPUT MONTH ( 1 to 12) and 'CR'");
  while(INPUT_MONTH() == -1);
//  Serial.println("INPUT DAY ( 1 to 31 ) and 'CR'");
  while(INPUT_DAY() == -1);
  Serial.println("INPUT WEEK (Sun.= 0  Mon.= 1  Tue.= 2  Wed.= 3  Thu.= 4  Fri.= 5  Sat = 6)and 'CR'");
  while(INPUT_WEEK() == -1);
  Serial.println("INPUT HOUR (0 to 23)and 'CR'");
  while(INPUT_HOUR() == -1);
  Serial.println("INPUT MINUTE (0 to 59)and 'CR'");
  while(INPUT_MIN() == -1);
}
//*****************************************************
void SET_DATE(void)
{
  ByteWrite(YEAR_reg,YEAR);
  ByteWrite(MONTH_reg,MONTH);
  ByteWrite(DAY_reg,DAY);
  ByteWrite(WEEK_reg,WEEK);
  ByteWrite(HOUR_reg,HOUR);
  ByteWrite(MIN_reg,MIN);
}
//*****************************************************
void TX_DATE(void)
{
  #ifdef TERMINAL  //ESC SEQUENCE
    if(IS_AF())                      Serial.print("\033[33m");//IF AF BIT = 1(ALARM TIME) YELLOW 
    else                             Serial.print("\033[39m");//NOT YET ALARM TIME            WHITE
  #endif
  Serial.print("20");
  if(YEAR < 0x0F)Serial.print('0');
  Serial.print(YEAR,HEX);
  Serial.print(" ");
//
  Serial.print(MONTH_S.substring((MONTH-1)*4,((MONTH-1)*4) + 4));
//
  if(DAY < 0x0F)Serial.print('0');
  Serial.print(DAY,HEX);
  Serial.print(" ");
//
  Serial.print(WEEK_S.substring((GET_WEEK(WEEK)*4),((GET_WEEK(WEEK)*4)+ 4)));
  Serial.print("   ");
//
  if(HOUR < 0x0F)Serial.print('0');
  Serial.print(HOUR,HEX);
  Serial.print(":");
  if(MIN < 0x0F)Serial.print('0');
  Serial.print(MIN,HEX);
  Serial.print("'");
  if(SEC < 0x0F)Serial.print('0');
  Serial.print(SEC,HEX);
  Serial.print('"');
  Serial.print("   ");
//
  TEMP_RESULT = ((float)TEMP * 2 - 187.19)/ 3.218;
  Serial.print(TEMP_RESULT);
  Serial.print("??     ");
//
  if(IS_AF())
  {
    Serial.println("ALARM !! ALARM !! ALARM !! ");
  }
  else                  
  {
    Serial.println();
  }
}
//******************************************************* 
void TX_COUNTER(void)
{
  if(SYORI_FLAG != TIMER_HALT)
  {
    if((TIMER_VALUE / 60) < 10)Serial.print("0");
    Serial.print(TIMER_VALUE / 60);
    Serial.print("'");
    if((TIMER_VALUE % 60)< 10)Serial.print("0");
    Serial.print(TIMER_VALUE % 60);
    Serial.println('"');  
  }
}
//*******************************************************
void ByteWrite(byte reg, byte data)
{
  Wire.beginTransmission(RX8900A_ADRS);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

byte ByteRead(byte reg)
{
  byte data = 0;
  Wire.beginTransmission(RX8900A_ADRS);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(RX8900A_ADRS, 1);
  data = Wire.read();      //RECEIVE 1BYTE
  return data;
}
