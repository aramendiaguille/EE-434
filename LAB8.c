
//Created by Guillermo Aramendia 18/10/2013

#include "reg51.h"

#include <stdio.h>

sbit Pbit3_2   = P3 ^ 2;//Start button

sbit Pbit3_7   = P3 ^ 7;//Start button

#define lowByte(x) ((unsigned char)(x));

#define highByte(x) (((unsigned int)(x))>>8);



//Global Variables

char notes[] = "ccggaagffeeddc "; // a space represents a rest

unsigned int freq;

unsigned int reloadValue;

//functions

void timer0() interrupt 1 {

	//Turn timer off

	TR0 = 0;

	

	//Set values for given period

	TH0 = highByte(reloadValue);

	TL0 = lowByte(reloadValue);

	//Turn timer back on

	TR0 = 1;

	//Toggle Pin

#pragma ASM

	CPL	P3.7

#pragma ENDASM

	

}
void playdo() {
int i,j;   
freq=261;reloadValue = (0xFFFF-(46080/freq)*10)+15;TR0=1;  
	for( i=0;i<30000;i++)
	{i++;
		for(j=0;j<2;j++)
		{j++;}
	}
}
void playre() {
 int i,j;  
freq=348;reloadValue = (0xFFFF-(46080/freq)*10)+15;TR0=1;  
	for( i=0;i<30000;i++)
	{i++;
	for(j=0;j<3;j++)
		{j++;}
	}
}
void playmi() {
int i,j;   
freq=392;reloadValue = (0xFFFF-(46080/freq)*10)+15;TR0=1;  
	for( i=0;i<30000;i++)
	{i++;
	for(j=0;j<3;j++)
		{j++;}
	}
}
void playfa() {
int i,j;   
freq=435;reloadValue = (0xFFFF-(46080/freq)*10)+15;TR0=1;  
	for( i=0;i<30000;i++)
	{i++;
	for(j=0;j<3;j++)
		{j++;}
	}
}
void playsol() {
int i,j;   
freq=479;reloadValue = (0xFFFF-(46080/freq)*10)+15;TR0=1; 
	for( i=0;i<30000;i++)
	{i++;
	for(j=0;j<3;j++)
		{j++;}
	}
}
void playla() {
int i,j;   
freq=522;reloadValue = (0xFFFF-(46080/freq)*10)+15;TR0=1; 
	for( i=0;i<30000;i++)
	{i++;
	for(j=0;j<3;j++)
		{j++;}
	}
}
void main()

{
  int a=0;
    /*------------------------------------------------

     Setup timer interrupts

     ------------------------------------------------*/

    

    TMOD &= 0xF0; // Timer 0 mode 1 with hardware gate

    TMOD |= 0x09; // GATE0=1; C/T0#=0; M10=0; M00=1;

    ET0=1; // enable timer0 interrupt

    EA=1; // enable interrupts

    

    /*------------------------------------------------

     Setup the serial port for 9600 baud

     ------------------------------------------------*/

    PCON &= 0x7F;			// CLEAR SMOD BIT ON PCON REGISTER
    SCON = 0x52;			// SET UP SCON REGISTER FOR 8-BIT UART MODE
    TMOD &= 0x0F;			// Clear Timer 1 values, keep Timer 0
    TMOD |= 0x20;     // SET UP TIMER 1 FOR 8-BIT AUTO-RELOAD TIMER/COUNTER
    TH1 = 0x0FD;			// SET TIMER HIGH TO #0FDH SO THAT TIMER 1 COUNTS THREE TIMES (FOR SETTING 9600 BAUD RATE)
    TL1 = 0x0FD;
    TR1 = 1;          // START TIMER 1
                //TR0=1;//enable timer	 
       playmi();
	   playsol();
	   playfa();
	   playdo();
	   playdo();
	   playmi();
	   playfa();
	   playsol();
	   playmi();
	   playmi();
	   playsol();
	   playmi();
	   playfa();
	   playdo();
	   playdo();
	   playdo();
	   playfa();
	   playsol();
	   playmi();
	   while(Pbit3_2==1 || a!=1)
	   {TR0=0;}
	   
}
