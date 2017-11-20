// Lab #5 - EE 626
// Ben Hutchins
//Include bacon.
#include "reg_c51.h"
extern char _getkey (void);
extern char getchar (void);
extern char putchar (char);
#define LOWBYTE(v)   ((unsigned char) (v))
#define HIGHBYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8))
typedef unsigned char byte;

//Declare all variables
char disp = 0;
char disp0, disp1, disp2, disp3, disp4; //Display variables for 7-segment display.
//Display variables are listed MSB to LSB.
unsigned char count = 10;
unsigned char sound = 0; //don't make noise yet!
unsigned int waitCount;
unsigned int T0Reload;

//Print a string via Serial
void sPrint(char *chrPoint){
	while(*chrPoint != '\0'){
		putchar(*chrPoint);
		chrPoint++; 
	}
}
//
//Serial setup: 8-bit, 9600 Baud @ 11.0592 MHz, 0 start bit, 1 stop bit
void serial_setup(void){
	PCON &= 0x7F;		//Disable double-baud rate
	SCON  = 0x50;                   /* SCON: mode 1, 8-bit UART, enable rcvr    */
	TMOD |= 0x20;                   /* TMOD: timer 1, mode 2, 8-bit reload      */
	TH1   = 0xFD;   // 9600 baud = 3 machine cycles. 0x00 - 0x03 = 0xFD
	TR1   = 1;                      /* TR1:  timer 1 run                        */
	TI    = 1;                      /* TI:   set TI to send first char of UART  */
}
//
//Convert a hex word to an ASCII char. Assume value contained in least significant 4 bits.
char WordToASCII(char in){
	char out;
	if(in <= 0x09)							{ out = in + 0x30; } //is number 0-9
	if(in >= 0x0A && in <= 0x0F){ out = in + 0x37; } //is char   A-F
	return out;
}
//
//Return a char string with the hex value in ASCII
void printHex(unsigned char in){
	unsigned char out1;
	unsigned char out2;
	out1 = WordToASCII( in & 0x0F );
	out2 = in;
	out2 &= 0xF0;
	out2 = out2 >> 4;
	out2 = WordToASCII( out2 );
	putchar(out2);
	putchar(out1);
}
//
//interrupt routine for Timer 0.
void T0Int(void) interrupt 1 {
	//Stop counting to reload the 16-bit timer0.
	TR0 = 0;
	TL0 = LOWBYTE(T0Reload);
	TH0 = HIGHBYTE(T0Reload);
	TR0 = 1;
		//if sound desired, toggle output sound bit P3_7
		if(sound == 1){ 
			if(P3_7){P3_7 = 0;} else{P3_7 = 1;} 
		}
		//After waiting the alloted amount of time, disable the frequency generator.
		//if(waitCount != 0){waitCount--;}else{TR0 = 0;}
}
//Toggle sound.
void startButton(void) interrupt 0{
	//Toggle the Timer if displaying. (Using XOR operator, ^)
	sound ^= 1;
}
//
//Push correct data to dataline for 7-segment displays.
void blipNum(char in){
	switch (in) {
	case 0x00: P1 &= 0xC0; P1 |= 0x40; break;
	case 0x01: P1 &= 0xF9; P1 |= 0x79; break;
	case 0x02: P1 &= 0xA4; P1 |= 0x24; break;
	case 0x03: P1 &= 0xB0; P1 |= 0x30; break;
	case 0x04: P1 &= 0x99; P1 |= 0x19; break;
	case 0x05: P1 &= 0x92; P1 |= 0x12; break;
	case 0x06: P1 &= 0x82; P1 |= 0x02; break;
	case 0x07: P1 &= 0xF8; P1 |= 0x78; break;
	case 0x08: P1 &= 0x80; P1 |= 0x00; break;
	case 0x09: P1 &= 0x90; P1 |= 0x10; break;
	}
}
//
//Refresh display
void refreshDisp(void){
		int i;
		//LSB
		i = 0; blipNum(disp3); P1_7 = 0; while(i < 50){i++;} P1_7 = 1; 
		i = 0; blipNum(disp2); P3_5 = 0; while(i < 50){i++;} P3_5 = 1; 
		i = 0; blipNum(disp1); P3_4 = 0; while(i < 50){i++;} P3_4 = 1; 
		i = 0; blipNum(disp0); P3_3 = 0; while(i < 50){i++;} P3_3 = 1; 
		//MSB
}
//
//Play the desired tone frequency for the desired number of tenths of seconds. While playing tone, display the frequency on the display.
void playTone(unsigned int freq, unsigned int tenths){
	waitCount = (tenths * freq * 2) / 10;
	T0Reload = 0xFFFF;
	T0Reload = T0Reload - (46080 / freq)*10 + 15;
	
	sound = 1;
	TR0 = 1;
	
	//Sit here. 
	while(1){
		refreshDisp();
	}
	
	/*
	while(TR0){
		refreshDisp();
	}
	sound = 0;
	*/
}
//

//main menu
void menu1(void){	
	unsigned int freq = 0;
	unsigned int tenths = 0;
	//char serialChar;
	//bit getChars;

	sPrint("\r\nFrequency   :\t");
	disp0 = getchar() - 0x30;
	freq = 10*freq + disp0;
	disp1 = getchar() - 0x30;
	freq = 10*freq + disp1;
	disp2 = getchar() - 0x30;
	freq = 10*freq + disp2;
	disp3 = getchar() - 0x30;
	freq = 10*freq + disp3;

	/*
	getChars = 1;
	while(getChars == 1){
		serialChar = getchar();
		if( 0x30 <= serialChar && serialChar <= 0x39 ){
			freq = 10*freq + (serialChar - 0x30);
		}
		else{getChars = 0;}
	}
	*/
	/*
	sPrint("\r\n10ths of sec:\t");
	getChars = 1;
	while(getChars == 1){
		serialChar = getchar();
		if( 0x30 <= serialChar && serialChar <= 0x39 ){
			tenths = 10*tenths + (serialChar - 0x30);
		}
		else{getChars = 0;}
	}
	*/
	playTone(freq,tenths);
}

//Setup serial I/O, and go to main menu.
void main (void){
	TMOD |= 0x01;        /* TMOD: timer 0, mode 1, 16-bit      */
	EA = 1;					//enable global interrupts
	ET0 = 1;				//Timer 0 interrupt enable. Interrupt #1.
	EX0 = 1;				//external #0 interrupt enable. Interrupt #0.

	serial_setup(); //Enable serial IO
		
	//Main program loop
	while(1){menu1();}
}


