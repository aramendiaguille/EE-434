/* Program     : LAB09
   Description : Analog-to-Digital Converter
   For         : AT89C4051 @ 12MHz
   Filename    : LAB09.c
   Compiler    : Keil uVision 4 - C51 Compiler
   Author      : Charles Weaver
   Date        : 11/1/12 
/****************************************************************************/
/*                               Pinout
P1.7 -----------> Segment A
P1.6 -----------> Segment B
P1.5 -----------> Segment C
P1.4 -----------> Segment D
P1.3 -----------> Segment E
P1.2 -----------> Segment F
P1.1 -----------> Segment G
GND  -----------> Segment DP (DISP1 only)
P1.0 -----------> PNP base -----------> (DISP1)  
P3.4 -----------> PNP base -----------> (DISP2)
P3.5 -----------> PNP base -----------> (DISP3)
P3.7 -----------> PNP base -----------> (DISP4)
P3.1 -----------> ADC CLOCK (CLK)
P3.0 -----------> ADC DATA OUT (DO)
P3.2 -----------> ADC CHIP SELECT (CS)                                      */

/****************************************************************************
            HEADERS, DEFINITIONS, GLOBAL VARIABLES/CONSTANTS
*****************************************************************************/
#include <regx51.h>
#include <intrins.h>
  
#define ADC_CLK    P3_1
#define ADC_DATA   P3_0
#define ADC_SELECT P3_2
#define DISP1      P1_0   
#define DISP2      P3_4   
#define DISP3      P3_5   
#define DISP4      P3_7    
#define DISP_PORT  P1
#define P1_0MASK   0x01  
#define ON         0
#define OFF        1

#define DISP_OFF     0
#define INSTANT_DISP 1
#define CONTIN_DISP  2
#define ADC_RES      256 // 2^n, ADC0831 => 8-bit => n = 8
  
typedef unsigned char   U8;
typedef unsigned short U16;
typedef unsigned long  U32;
U16     inst_rd;
U8      instflag;
U8      adcflag = 0;
U8 code disp_digits[]={0x03, 0x9F, 0x25, 0x0D, 0x99, 0x49, 0x41, 0x1F, 0x01, 0x19};  // seven segment display digits, digits[0 through 9] = 0 through 9 on display

/****************************************************************************
                        FUNCTION PROTOTYPES
*****************************************************************************/
void init();
void ADC_setflag();
U8   ADC_read();
void delay( U8 loop );
void disp_out( U16 volts );
void delay50ms( U8 times );
U32  ADC_conversion();
/****************************************************************************
                     INTERRUPT SERVICE ROUTINES
*****************************************************************************/
void ext1_ISR() interrupt 2{
  delay50ms( 8 );
  if( P3_3 == 0 ) ADC_setflag();  
}

/****************************************************************************
                                MAIN
*****************************************************************************/
void main(){    
  init();

  while(1)
    {     
      if( adcflag == INSTANT_DISP )
        {
         if( instflag == 1 ) inst_rd = ADC_conversion();
         instflag = 0;
         disp_out( inst_rd );
        }

      if( adcflag == CONTIN_DISP ) disp_out( ADC_conversion() );
      if( adcflag == DISP_OFF )
        {
         DISP1      = OFF;
         DISP2      = OFF;
         DISP3      = OFF;
         DISP4      = OFF;
         ADC_SELECT = OFF;
        }
    }     
}
/****************************************************************************
                        FUNCTION IMPLEMENTATION
*****************************************************************************/
void init(){ /* init serial, interrupts, timers */
  EX1      = 1;
  IT1      = 1;
  EA       = 1;
  ADC_DATA = 1; // Initialize pin connected to DO for input  
}

U8 ADC_read(){
  U8 k;
  U8 bdata j;

  ADC_SELECT = OFF;
  ADC_SELECT = ON;
  _nop_();
  _nop_();
  ADC_CLK    = 1;
  _nop_();
  _nop_();
  ADC_CLK    = 0;

  for( k = 0; k < 9; k++ )
    {
      _nop_();
      _nop_();
      ADC_CLK = 1;
      _nop_();
      _nop_();
      ADC_CLK = 0;
      _nop_();
      j <<= 1;
      if( ADC_DATA ) j++;
      
    }
  ADC_SELECT = OFF;
  ADC_CLK    = 1;
  _nop_();
  _nop_();
  return j;
}

void ADC_setflag(){   // Options for displaying ADC measurement
  switch ( adcflag )
    {
      case 0:
      adcflag = INSTANT_DISP;
      instflag = 1;
      break;
      case 1:
      adcflag = CONTIN_DISP;
      break;
      case 2:
      adcflag = DISP_OFF;
      break;
      default:
      adcflag = DISP_OFF;
    }
}

U32 ADC_conversion(){
  U32 adcval = 5000; // Reference voltage of 5V scaled by 1000
  U8  j      = ADC_read();
  adcval    *= j;
  adcval    /= ADC_RES;
  return adcval;  
}

void disp_out( U16 volts ){  /* Output volts to seven segment displays */
  U8 k;

  // disp 1
  k  = (DISP_PORT & P1_0MASK);
  k |= disp_digits[ volts/1000 ];
  DISP_PORT = k;    
	DISP1 = ON;                                
	delay( 2 );
	DISP1 = OFF;

  // disp 2
  k  = (DISP_PORT & P1_0MASK);
  k |= disp_digits[ (volts/100)%10 ];
  DISP_PORT = k;  
	DISP2 = ON;                                
  delay( 2 );
	DISP2 = OFF;

  // disp 3
  k  = (DISP_PORT & P1_0MASK);
  k |= disp_digits[ (volts%100)/10 ];
  DISP_PORT = k;  
	DISP3 = ON;                                 
	delay( 2 );
	DISP3 = OFF;

  // disp 4
  k  = (DISP_PORT & P1_0MASK);
  k |= disp_digits[ volts%10 ];
  DISP_PORT = k;  
	DISP4 = ON;                               
	delay( 2 );
	DISP4 = OFF;
}

void delay( U8 loop ){  /* delay (timed with performance analyzer) */
  U8 k;

  while( loop-- )
		  for( k = 228; k > 0; k-- );
}

void delay50ms( U8 times ){
  while( times-- )
    {
      TH0 = 0x3C;
      TL0 = 0xB0;
      TR0 = 1;
      while( !TF0 );
      TF0 = 0;
      TR0 = 0;
    }
}
