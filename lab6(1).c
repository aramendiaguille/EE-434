/***************************************************/
/*                                                 */
/*                Eric Klukovich                   */
/*          Lab #6 - Function Generator            */
/*                                                 */
/***************************************************/

// Header Files
#include <reg51.h>
#include <stdio.h>


// Global Variables
sbit generator = P3^7;
int frequency, timerHigh, timerLow;


// Function Implementations
void refresh(int number, int led)
   {
    // Output the value for the led digit
		   switch(number)
				{
				 // check for 0
						case 0:
							P1 = 0xC0;
							break;
						
				 // check for 1
						case 1:
							P1 = 0xF9;
							break;
				
				 // check for 2
						case 2:
							P1 = 0xA4;
							break;		
				
				 // check for 3
						case 3:
							P1 = 0xB0;
							break;		
				
				 // check for 4
						case 4:
							P1 = 0x99;
							break;		
				
				 // check for 5
						case 5:
							P1 = 0x92;
							break;		
				
				 // check for 6
						case 6:
							P1 = 0x82;
							break;		
				
				 // check for 7
						case 7:
							P1 = 0xF8;
							break;			
				
				 // check for 8
						case 8:
							P1 = 0x80;
							break;		
				
				 // check for 9
						case 9:
							P1 = 0x90;
							break;							
				}

    // Output the value for the led digit
		   switch(led)
				{
				 // check for digit 0
						case 0:
							P1 &= 0x7F;
						  T1 = 1;
						  T0 = 1;
						  INT1 = 1;
							break;
						
				 // check for digit 1
						case 1:
							P1 |= 0x80;
						  T1 = 0;
						  T0 = 1;
						  INT1 = 1;							
							break;
				
				 // check for digit 2
						case 2:
							P1 |= 0x80;
						  T1 = 1;
						  T0 = 0;
						  INT1 = 1;										
							break;		
				
				 // check for digit 3
						case 3:
							P1 |= 0x80;
						  T1 = 1;
						  T0 = 1;
						  INT1 = 0;			
							break;									
				}				
	 }
	 
void pString(char *string)
   {
		// loop until null character
       while(*string != '\0')
				 {
					// send one character to the screen  
             putchar(*string);
             string++;
         }
   }
	 
void generate()
	 {
		// initialize variables
		   int time;
       unsigned long machineCycles;
		 
	  // turn off the timer
       TR0 = 0;
		 
		// initialize the generator pin 
		   generator = 1;
		 
    // calculate the number of machine cycles from the frequency
       machineCycles = (1000000000 / frequency) / 2;
		   machineCycles = machineCycles / 1085;

		// calculate the value to load in the timer
		   time = 0xFFFF - machineCycles;
		   time += 12;
		 
		// split the timer value into two bytes 
       timerLow = time & 0xFF;
       timerHigh = (time >> 8) & 0xFF;	

    // load the values into the timer
       TH0 = timerHigh;
       TL0 = timerLow;
		 
    // turn on timer 0 interrupts
		   EA = 1;
		   ET0 = 1;

		// turn on the timer
       TR0 = 1;	 		 
   }

	 
	 
void getInput()
	 {
		// initialize variables
       char value;
       int i = 0;		
       frequency = 0;		 
		 
    // get input from the user
		 
  		 // loop and get all the inputs
					value = _getkey();
					while(value != 0x0D && i < 5)
						{  
						 // echo the letter to the screen
								putchar(value);

  					 // put value in the char array
								frequency = (frequency * 10) + (value - 0x30);
								i ++;		
	
						 // get the key  
							  value = _getkey();							 
						}	 	 
	 }

 
void timer0 (void) interrupt 1 
   {
		// turn off the timer
       TR0 = 0;
		 
		// Reload Timer values (10 ms delay)		 
       TH0 = timerHigh;		 
	     TL0 = timerLow;

		// turn on the timer
       TR0 = 1;
   
    // take off 10 ms from total delay
       generator = ~generator;
   }	 
	 
	 
// Main Program
void main (void) 
   {	 	 
		// initialize variables
		   int i;
		 
	  // initialize UART  
		 
       // clear SMOD bit (double the baud rate)
          PCON &= 0x07F;
   
       // Set timer 0 to 16 bit, set timer 1 to 8 bit auto-reload
          TMOD = 0x21;
   
       // set timer 1 to -3 for 9600 baud rate	
          TH1 = -3;
         
       // Start the timer
          TR1 = 1;

       // Set UART to 8 bit mode and enable the receiver
          SCON = 0x52;		 
		 
    // Infinite loop
       while(1)
				 { 
					// prompt for the time				 
					   pString("\nEnter the frequency in hertz (100-1000): ");
             getInput();
						 
          // wait for the button to be pushed
             while( INT0 == 1 );
					
          // start the count down
             generate();
					 
					// delay for debounce	 
						 for(i = 0; i<20000; i++){}

					// wait to turn off the off the generator
						 while( INT0 == 1 )
							 {
								for( i=0; i < 10; i++) 
									refresh( frequency % 10, 0 );
								for( i=0; i < 10; i++)  
									refresh( (frequency / 10) % 10, 1 );	
								for( i=0; i < 10; i++) 							 
									refresh( (frequency / 100)%10, 2 );
								for( i=0; i < 10; i++)  
									refresh( (frequency / 1000), 3 );								 
							 }	
							 
					// turn off displays
						 P1 |= 0x80;
						 T1 = 1;
						 T0 = 1;
						 INT1 = 1;

					// turn off the generator							 
						 TR0 = 0;
						 EA = 0;						 
			   }
   }