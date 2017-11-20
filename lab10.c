/*----------------------------------------------------------------------------
 Head Files.
----------------------------------------------------------------------------*/
#include <stdio.h>
#include <ctype.h>
#include <REGx051.h>
#include <intrins.h>

/*----------------------------------------------------------------------------
 Global Variables.
----------------------------------------------------------------------------*/
int inton, intoff;
/*----------------------------------------------------------------------------
 Interrupt Service Routines.
----------------------------------------------------------------------------*/
void timer0(void) interrupt 1
{
	TR0 = 0;
	TH0 = 0xff;
	if (P3_5 == 1)
	{
		P3_5 = 0;		
		TL0 = intoff;		
	 }
	 else
	 {
	 	P3_5 = 1;
		TL0 = inton;
	 }
	TR0 = 1;
		
}

/*----------------------------------------------------------------------------
Functions.
----------------------------------------------------------------------------*/
/*
void readValue()
{
	//declare variables
	int j;
	unsigned char value;



	//initilize pins
	P3_3 = 1;	//chip select
	P1_7 = 1;	//clock
	P3_4 = 1;	//data transfer line

	//clear chip select
	P3_3 = 1;
	//two nops
	_nop_();
	_nop_();
	//set clock
	P1_7 = 1;
	//two nops
	_nop_();
	_nop_();
	//clr clock
	P1_7 = 0;

	//loop (9 times)
	for (j = 0; j < 9; j++)
	{ 
		//two nops
		_nop_();
		_nop_();
		//set clock
		P1_7 = 1;
		//two nops
		_nop_();
		_nop_();
		//clr clock
		P1_7 = 1;
		//two nops
		_nop_();
		_nop_();
		//shift left variable
		value << 1;
		//read data
		if (P3_4 == 1)
		{
			value ++;	
		}
		//loop back
	}

	//set clock high
	P1_7 = 1;
	//set chipselect high
	P3_3 = 1;

}
*/
float calculate (float volt)
{
	//declare variables
	float x;

	x = volt/9.00;

return x;
}


/*----------------------------------------------------------------------------
 Main Program.
----------------------------------------------------------------------------*/
int main()
{
	//declare variables
	int val;
	int j;
	int choice [3];
	float voltage;
	float v1;
	float v2;
	float v3;
	float dutycycle; 
		
	int t1, t2, t3, t4, t5;
	int t11, t33, t44;
	
	

	//initialize serial port
   	TMOD = 0x21;
   	SCON = 0x52;
   	TH1 = 0xFD;
   	TR1 = 1;

	//enable interrupts
//	EA = 1;
//	ET0 = 1;

	


	//output prompt
	putchar ('\n');
   	puts("****Enter Voltage (0.00 to 9.99) : ");
	

	//get and load input
	val = getchar ();
	choice [0] = val;
	val = getchar ();
	val = getchar ();
	choice [1] = val;
	val = getchar ();
	choice [2] = val;
/*
	puts("\n");

	for (j = 0; j < 3; j++)
	{
	 	putchar (choice [j]);
	}
*/ 
	 //readValue();

	 //changing from char to ints
	 for (j = 0; j < 3; j++)
	 {
	  	choice[j] = choice[j] - 0x30;
	 }
	 
	 //change to a decimal number
	 v1 = choice [0] * 1.00;
	 v2 = choice [1] * 0.10;
	 v3 = choice [2] * 0.01;
	 voltage = v1 + v2 + v3;

	 	 
	 dutycycle = calculate (voltage);
	 inton = 131*dutycycle;
	 intoff = 131*(1-dutycycle);

	 intoff = 255-intoff+15;
	 inton = 255-inton+15;

	 

///tests///////
	 t1 = inton/100;		//hundredth value
	 t2 = inton - (t1*100);
	 t3 = t2/10;			//tens value
	 t4 = t2 - (t3*10);		//ones value


	 t11 = t1 + 0x30;
	 t33 = t3 + 0x30;
	 t44 = t4 + 0x30;

	 putchar (t11);
	 putchar (t33);
	 putchar (t44);

	 t1 = intoff/100;		//hundredth value
	 t2 = intoff - (t1*100);
	 t3 = t2/10;			//tens value
	 t4 = t2 - (t3*10);		//ones value


	 t11 = t1 + 0x30;
	 t33 = t3 + 0x30;
	 t44 = t4 + 0x30;

	 putchar (t11);
	 putchar (t33);
	 putchar (t44);
	 
/////////////////////////////////////////////

	//set the clock to high
	P3_5 = 1;

	//time for high
	TH0 = 0xff;
	TL0 = inton;
	TR0 = 1;
  

while(1){}
return;
}