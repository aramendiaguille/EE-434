// INCLUDES
#include <stdio.h>
#include <ctype.h>
#include <REGx051.h>

//interrupt service routines
void timer0(void) interrupt 1
	{
		//stuff
		ACC = 0x45;		
	}
void external0 (void) interrupt 1
	{
		int C;
	 	C = 1;
	}


//functions


//main program
int main ()
{
   	EA = 1;
	ET0 = 1;
	TMOD = 0x02;
	TH0 = 0xff;
	TR0 = 1;

	
while (1);
//return 0;
}