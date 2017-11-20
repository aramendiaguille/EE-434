/* Program     : LAB11
   Description : DC Motor Control
   For         : AT89C4051 @ 11.0592MHz
   Filename    : LAB11.c
   Compiler    : Keil uVision 4 - C51 Compiler
   Author      : Charles Weaver
   Date        : 11/14/12 
/****************************************************************************/
/*Pinout
P1.7 -----------> Segment A
P1.6 -----------> Segment B
P1.5 -----------> Segment C
P1.4 -----------> Segment D
P1.3 -----------> Segment E
P1.2 -----------> Segment F
P1.1 -----------> Segment G                                  
P3.7 -> PWM OUT TO 2N7000 GATE
P3.5 -> ADC DATA OUT
P3.4 -> ADC CLOCK
P3.3 -> ADC CHIP SELECT
P3.3 -> DISP3
P1.0 -> DISP2
P3.2 -> DISP1
P3.1 -> T1IN  (P11)
P3.0 -> R1OUT (P12)                                                         */
/****************************************************************************
            HEADERS, DEFINITIONS, GLOBAL VARIABLES/CONSTANTS
*****************************************************************************/
#include <regx51.h>
#include <intrins.h>

#define DISP1      P3_2   
#define DISP2      P1_0   
#define DISP3      P3_3 
#define DISP_PORT  P1
#define P1_0MASK   0x01  
#define ADC_CLK    P3_4
#define ADC_DATA   P3_5
#define ADC_SELECT P3_3
#define PWM_OUT    P3_7 
#define ON         0
#define OFF        1
#define AMP_GAIN   33
  
typedef unsigned char   U8;
typedef unsigned short U16;
typedef signed   short S16;
typedef unsigned long  U32;

S16 duty;
S16 present; 
U8  set_point;
S16 error;
S16 voltage;
U8 hold;
U16 present_back_emf;
U8 code newline[] = { 0x0A, 0x0D, 0x00 };
U8 code prompt[] = "ENTER VOLTAGE: ";
U8 code disp_digits[]={0x03, 0x9F, 0x25, 0x0D, 0x99, 0x49, 0x41, 0x1F, 0x01, 0x19};

/****************************************************************************
                        FUNCTION PROTOTYPES
*****************************************************************************/
U8   getchar();
U8   ADC_read();
U8   ascii_num_to_hex( U8 num );
U16  get_volt();
void ADC_clk();
void putchar( U8 ch );
void reset();
void init();
void prnt_string_serial( U8 *string );
void delay( U8 t );
void disp_out( U16 val );
void disp_back_emf( S16 duty_cycle );
/****************************************************************************
                     INTERRUPT SERVICE ROUTINES
*****************************************************************************/
void timer0_ISR() interrupt 1{
  TR0 =    0;
  TH0 = 0xFF;
  if( PWM_OUT ){ PWM_OUT = 0; TL0 = duty;        }
  else         { PWM_OUT = 1; TL0 = 0xFF - duty; }
  TR0 =    1;
}
/****************************************************************************
                                MAIN
*****************************************************************************/
void main(){    
  init();
  voltage = get_volt();
  duty = ((10*voltage)/24);

  TR0 = 1;
  PT0 = 1;
  delay( 255 );
  set_point = duty - ADC_read();  
  while(1)
    {
	 //delay( 50 );
     present = duty - (3 * ADC_read()); 
     error = set_point - present;
     disp_back_emf( duty );

     if     ( duty + error > 255 ) duty  = 255;
     else if( duty + error < 0 )   duty  = 0;
     else                              duty += error;
    }     
}
/****************************************************************************
                        FUNCTION IMPLEMENTATION
*****************************************************************************/
void init(){
  PWM_OUT  = 0;
  SCON     = 0x50;
  TMOD     = 0x21;
  TH1      = 0xFD;
  TR1      = 1;
  TI       = 1;
  EA       = 1;
  ADC_DATA = 1;
  ET0      = 1;
  prnt_string_serial( newline );
  prnt_string_serial( prompt );  
}

void disp_back_emf( S16 duty_cycle ){
  U8 k;
  k = ADC_read();
  present_back_emf = ((duty_cycle * 40)/17) - ((95*k)/AMP_GAIN);
  disp_out( present_back_emf );
}

U8 ADC_read(){
  U8 k;
  U8 bdata j;

  ADC_SELECT = ON;
  ADC_clk();
  for( k = 0; k < 9; k++ ) { ADC_clk(); j <<= 1; if( ADC_DATA ) j++; }
  ADC_SELECT = OFF;
  ADC_CLK = 1;
  _nop_();
  _nop_();
  return j;
}

void disp_out( U16 val ){ 
  U8 k;

  // disp 1
  k  = (DISP_PORT & P1_0MASK);
  k |= disp_digits[ val/100 ];
  DISP_PORT = k;    
	DISP1 = ON;                                
	delay( 2 );
	DISP1 = OFF;

  // disp 2
  k  = (DISP_PORT & P1_0MASK);
  k |= disp_digits[ (val/10)%10 ];
  DISP_PORT = k;  
	DISP2 = ON;                                
  delay( 2 );
	DISP2 = OFF;

  // disp 3
  k  = (DISP_PORT & P1_0MASK);
  k |= disp_digits[ val%10 ];
  DISP_PORT = k;  
	DISP3 = ON;                                 
	delay( 2 );
	DISP3 = OFF;
}

void ADC_clk(){
  _nop_();
  _nop_();
  ADC_CLK = 1;
  _nop_();
  _nop_();
  ADC_CLK = 0;
  _nop_();
}

void prnt_string_serial( U8 *string ){
  while( *string ) putchar( *string++ );
}

U8 getchar(){
  while( !RI );
  RI = 0;
  return SBUF;
}
                                                                     
void putchar( U8 ch ){
  while( !TI );
  TI = 0;
  SBUF = ch;
}

U16 get_volt(){
  U8 k;
  U16 volt = 0;
  
  k = getchar();
  if( ascii_num_to_hex(k) == 0xFF ) reset();
  putchar(k);
  volt += ( ascii_num_to_hex(k)*100 );
  k = getchar();
  if( k != 0x2E ) reset();
  putchar(k);
  k = getchar();
  if( ascii_num_to_hex(k) == 0xFF ) reset();
  putchar(k);
  volt += ( ascii_num_to_hex(k)*10 );
  k = getchar();
  if( ascii_num_to_hex(k) == 0xFF ) reset();
  putchar(k);
  volt += ascii_num_to_hex(k);
  while( getchar() != 0x0D );
  return volt;
}

U8 ascii_num_to_hex( U8 num ){
  if( num >= 0x30 && num <= 0x39 ) return (num & 0x0F); 
  else return 0xFF; 
}

void reset(){
  ((void (code *) (void)) 0x0000)();
}

void delay( U8 t ){  
  U8 k;
  while( t-- ) for( k = 230; k > 0; k-- );
}
