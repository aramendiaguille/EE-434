#include "reg51.h"
char uart_data;
void main(){

SCON = 0x50; /* uart in mode 1 (8 bit), REN=1 */
TMOD = TMOD | 0x20 ; /* Timer 1 in mode 2 */
TH1 = 0xFD; /* 9600 Bds at 11.059MHz */
TL1 = 0xFD; /* 9600 Bds at 11.059MHz */
ES = 1; /* Enable serial interrupt*/
EA = 1; /* Enable global interrupt */
TR1 = 1; /* Timer 1 run */
 while(1);
}

void serial_IT(void) interrupt 4
{

	if (RI == 1)
	{ /* if reception occur */
		RI = 0; /* clear reception flag for next reception */
		
		uart_data = SBUF; /* Read receive data */
		if(uart_data>=0x41 && uart_data<=0x5A)
		{
			uart_data=uart_data+0x20;
		}
		if(uart_data>=0x61 && uart_data<=0x7A)
		{
			uart_data=uart_data-0x20;
		}
		 SBUF=uart_data; /* Send back data on uart*/
	}

	else TI = 0; /* if emission occur */
} 

