#include "reg51.h"
#define ON 0
#define OFF 1
#define LOWBYTE(v)   ((unsigned char) (v))
#define HIGHBYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8)) // Thanks Ben Hutchins!

extern char _getkey (void);
extern char putchar (char);
void sPrint(char *chrPoint);
void delay(int);
void display(int, int);

sbit DIG0 = P1^7;  // SFR for P1.7
sbit DIG1 = P3^5;	 // SFR for P3.5
sbit DIG2 = P3^4;	 // SFR for P3.4
sbit DIG3 = P3^3;	 // SFR for P3.3

sbit START = P3^2;	// button interrupt on pin 6
sbit BUZZ = P3^7;	// speaker output on pin 11

int value(int _data);
void _RefreshDisplay(void);
unsigned int reload;
int output;

int w,x,y,z;
	
void main (void)
{
	unsigned int frequency;
	
	/* Timer setup: timer0 is for function generation, timer1 is for UART */
	IT0 = 1; // set external interrupt 0 to trigger on falling edge (TCON register)
	EX0 = 1; // enable external interrupt 0 (P3.2, pin 6)
	ET0 = 1; // enable timer0 interrupt
	//EX1 = 1; // enable external interrupt 1 (P3.3, pin 7 - keep for future use)
	EA = 1; // enable interrupts
	PCON &= 0x07F;		// clear SMOD bit (double the baud rate)
	TMOD = 0x21;			// Set timer 0 to 16-bit software-controlled; set timer 1 to 8-bit auto-reload
	TH1 = -3;   			// set timer 1 to -3 for 9600 baud rate	
	TR1 = 1;					// Start timer 1
	SCON = 0x52;			// Set UART to 8 bit mode and enable the receiver

	DIG0 = OFF;
	DIG1 = OFF;
	DIG2 = OFF;
	DIG3 = OFF;
	BUZZ = 0;
	output = 0;
	
	sPrint("\nEnter frequency 10-9999 Hz: ");
	while(1)
	{
		w = _getkey() - '0';
		putchar(w + '0');
		x = _getkey() - '0';
		putchar(x + '0');
		y = _getkey() - '0';
		putchar(y + '0');
		z = _getkey() - '0';
		putchar(z + '0');
		frequency = ((w) * 1000) + ((x) * 100) + ((y) * 10) + (z);
		reload = 0xFFFF - (46080 / frequency) * 10 + 30; // sets the timer for the frequency
		sPrint("\nPush button to toggle tone. Enter a new frequency at any time: ");
	}
}

void sPrint(char *chrPoint)
{
    while(*chrPoint != '\0')
		{
			putchar(*chrPoint);
      chrPoint++; 
    }
}

void tone(void) interrupt 0
{
	delay(20); // debounce
	if(START == 0)
	{
		TR0 = !TR0; //for some reason this isn't turning the tone off. Only on.
	}
}
void toneGen(void) interrupt 1 
{
	TR0 = 0;
	TH0 = HIGHBYTE(reload);
	TL0 = LOWBYTE(reload);
	TR0 = 1;
	BUZZ = !BUZZ;
	_RefreshDisplay();
}

void _RefreshDisplay(void)
{
	P1 = value(w);
	DIG0 = OFF;
	DIG1 = OFF;
	DIG2 = OFF;
	DIG3 = ON;
	P1 = value(x);
	DIG0 = OFF;
	DIG1 = OFF;
	DIG2 = ON;
	DIG3 = OFF;
	P1 = value(y);
	DIG0 = OFF;
	DIG1 = ON;
	DIG2 = OFF;
	DIG3 = OFF;
	P1 = value(z);
	DIG0 = ON;
	DIG1 = OFF;
	DIG2 = OFF;
	DIG3 = OFF;
	DIG0 = OFF;
}

int value(int _data)
{
	if(_data == 0)
	{
		return 0xC0;
	}
	else if(_data == 1)
	{
		return 0xF9;
	}
	else if(_data == 2)
	{
		return 0xA4;
	}
	else if(_data == 3)
	{
		return 0xB0;
	}
	else if(_data == 4)
	{
		return 0x99;
	}
	else if(_data == 5)
	{
		return 0x92;
	}
	else if(_data == 6)
	{
		return 0x82;
	}
	else if(_data == 7)
	{
		return 0xF8;
	}
	else if(_data == 8)
	{
		return 0x80;
	}
	else if(_data == 9)
	{
		return 0x98;
	}
	else
	{
		return 0xC0;
	}
}

void delay(int _milliseconds) // Stops timer, loads in 0xFFFF - 922, which *should* leave 1ms left on the timer. Start timer, then repeat when timer overflows.
{
	int _i;
	for(_i=0; _i<_milliseconds; _i++)
	{
		TR0 = 0;
		TH0 = 0xFC;
		TL0 = 0x65;
		TR0 = 1;
		while(TH0 != 0x00);
	}
}