/* Program     : Final Project
   Description : Water Sensor and Data Logger
   For         : AT89C4051 @ 11.0592MHz
   Filename    : finalproject.c
   Compiler    : Keil uVision 4 - C51 Compiler
   Author      : Charles Weaver and Adeline Duong
   Date        : LAST UPDATED 12/8/2012 
****************************************************************************/
#define RTC_CONFIGURATION 0		// enable to use RTC_configure_and_test()
#include <regx51.h>
#include <intrins.h>
#include <stdio.h>
// defines
#define ADC_CLK         P3_4
#define ADC_DATA        P3_5
#define ADC_SELECT      P3_3
#define LINEFEED        0x0A
#define CARRIAGE        0x0D
// i2c
#define MEM_CTRL_RD     0xA1 // Address to read from the memory chip
#define MEM_CTRL_WR     0xA0 // Address to write to the memory chip
#define RTC_CTRL_RD     0xDF // Address to read from the RTC chip
#define RTC_CTRL_WR     0xDE // Address to write to the RTC chip
#define RTC_START_OSC   0x80 // Byte to start oscillator, written to 0x00
#define RTC_SECONDS_REG 0x00
#define RTC_MINUTES_REG 0x01
#define RTC_HOURS_REG   0x02
#define RTC_DATE_REG    0x04
#define RTC_MONTH_REG   0x05
#define RTC_YEAR_REG    0x06
#define SECONDS_MASK    0x7F
#define MINUTES_MASK    0x7F
#define HOURS_MASK      0x3F
#define DATE_MASK       0x3F
#define MONTH_MASK      0x1F 
#define I2C_SCL         P1_1 // I2C Clock
#define I2C_SDA         P1_0 // I2C Data
#define MSB_MASK        0x80
#define SECONDS            0
#define MINUTES            1
#define HOURS              2
#define DAYS               3
#define MONTHS             4
#define YEARS              5
//define data types
typedef unsigned char   U8;
typedef unsigned short U16;
typedef signed   short S16;
typedef unsigned long  U32;

static volatile U16 MEM_address_counter = 0x0000;
static          U8  tt_compare = 0;
code            U8  digit[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

//global variable
bit	TRIGGER;

/****************************************************************************
                        FUNCTION PROTOTYPES
*****************************************************************************/
// I2C: Inter-Integrated Circuit protocol
U8   I2C_rx();
bit  I2C_tx( U8 tx_data );
void I2C_start();  
void I2C_stop();                  
// I2C Memory: AT24C64 EEPROM 8192 x 8, Byte addressable, range: 0x0000-0x1FFF
U8   MEM_read_byte( U16 rd_addr );
U16  MEM_last_address(); 
void MEM_write_byte( U8 write_data, U16 wr_addr );
void MEM_set_address( U16 addr );
void MEM_erase();
void MEM_write_delay();
// I2C Real Time Clock: MCP7940M 
U8   RTC_read_byte( U8 rd_addr );
void RTC_set_address( U8 addr );
void RTC_configure_and_test();
// sampling
void write_sample();
U8   *get_sample();
void print_samples();
void take_sample( U8 timebase );
void print_continuous_sample();
// miscellaneous
U8   bcd2ascii_high( U8 bcd_byte );
U8   bcd2ascii_low( U8 bcd_byte );
U8   ADC_read();
void ADC_clk();
void init();
void reset();

void RTC_configure_and_test();
void RTC_write_byte( U8 write_byte, U8 wr_addr );

/****************************************************************************
                     INTERRUPT SERVICE ROUTINES
*****************************************************************************/
void external0(void) interrupt 0
{
	U8 delay;
	 
	 EX0 = 0;			//turn off interrupt
	 for (delay = 0; delay < 40; delay++) MEM_write_delay();	//takes care of ringing

	 if( !P3_2 ) TRIGGER = !(TRIGGER);	//complement carry

	 EX0 = 1;			//turn interrupt back on 		
}

/****************************************************************************
                                MAIN
*****************************************************************************/
void main(){
  //declaring variables
  U8 choice, action;
  bit display;
  
  //initalize stuff
  init();

  while (1)
{
  display = 1;
  TRIGGER = 1;
  MEM_address_counter = MEM_last_address();

  puts( "SENSOR DATALOGGER" );
  puts ("Select Speed: ");	  		//lets hope they don't want to sample every day, month, or year!
  puts (" 1) seconds");
  puts (" 2) minutes");
  puts (" 3) hours");
  puts (" 4) View continuous sampling data" );
  choice = getchar() - 0x31;
  putchar( LINEFEED );
  putchar( CARRIAGE );
  if( choice == 3 )
    {
	 puts( "Must reset system to exit. ***DATA WILL NOT BE SAVED***" );
	 print_continuous_sample();
	}
  puts ("Logging data....");
  puts ("Press button to stop" );

  //record data  
  while(TRIGGER)
    {
      take_sample( choice );
    } 

  //display menu	  
  while (display)
  	{
		//prompt for input
		puts (" 1. Display data");
		puts (" 2. Continue logging data" );
		puts (" 3. Erase All Memory" );
		action = getchar();
		putchar( LINEFEED );
      	putchar( CARRIAGE );
		
		if (action == '1')
		{
			 print_samples();
			 display = 0;
		} 
		else if (action == '2')
		{
			display = 0;
		}
		else if (action == '3')
		{
			puts ("Please wait....");
			MEM_erase();
			puts ("Erase completed");
			display = 0;
		} 
		else
		{ 
			puts ("Invalid Input!");
		} 
	}
}	   
}

/****************************************************************************
                        FUNCTION IMPLEMENTATION
*****************************************************************************/
//............................................................................//
void init(){
  SCON     = 0x50;
  TMOD     = 0x20;
  TH1      = 0xFD;
  TR1      = 1;
  TI       = 1;
  I2C_SDA  = 1;
  I2C_SCL  = 1;
  ADC_DATA = 1;
  EA = 1;
  EX0 = 1;
}
//............................................................................//
void take_sample( U8 timebase ){ 
  U8 tt;

  switch( timebase )
    {
      case SECONDS: // sample per second
      tt = RTC_read_byte(RTC_SECONDS_REG)&SECONDS_MASK;
      break;
      case MINUTES: // sample per minute
      tt = RTC_read_byte(RTC_MINUTES_REG)&MINUTES_MASK;
      break;
      case HOURS:   // sample per hour
      tt = RTC_read_byte(RTC_HOURS_REG)&HOURS_MASK;
      break;
      case DAYS:    // sample per day
      tt = RTC_read_byte(RTC_DATE_REG)&DATE_MASK;
      break;
      case MONTHS:  // sample per month
      tt = RTC_read_byte(RTC_MONTH_REG)&MONTH_MASK;
      break;
      case YEARS:   // sample per year
      tt = RTC_read_byte(RTC_YEAR_REG);
      break;
      default:      // default to sample per second
      tt = RTC_read_byte(RTC_SECONDS_REG)&SECONDS_MASK;
      break;
    }

  if( tt != tt_compare )
    {
      write_sample();
      tt_compare = tt;
    }
}
//............................................................................//
void print_samples(){
  U8 sample_value, ss, mm, hh, dd, nn, yy;
  U16 j;
  U16 i = MEM_last_address();
  S16 tempVal;

  for( j = 0; j < i; j+=7 )
    {
      sample_value = MEM_read_byte(  j    );
      nn           = MEM_read_byte( (j+1) );
      dd           = MEM_read_byte( (j+2) );
      yy           = MEM_read_byte( (j+3) );
      hh           = MEM_read_byte( (j+4) );
      mm           = MEM_read_byte( (j+5) );
      ss           = MEM_read_byte( (j+6) );

	  tempVal  = -66*sample_value;
	  tempVal += 7918;
//	  sample_value = (U8)(tempVal);
//	  putchar ('\n');
      if( tempVal < 0 ) tempVal = 0;
	  putchar (digit[tempVal/1000]);
	  putchar (digit[(tempVal%1000)/100]);
	  putchar ('.');
	  putchar (digit[(tempVal%100)/10]); 
	  putchar (digit[tempVal%10]);
	  putchar( ' ');
	  putchar( 'c');
	  putchar( 'm');
      putchar( ' ' );
      putchar( bcd2ascii_high(nn) ); // print 10s of month
      putchar( bcd2ascii_low(nn) );  // print 1s of month
      putchar( '/' );
      putchar( bcd2ascii_high(dd) ); // print 10s of date
      putchar( bcd2ascii_low(dd) );  // print 1s of date
      putchar( '/' );
      putchar( bcd2ascii_high(yy) ); // print 10s of year
      putchar( bcd2ascii_low(yy) );  // print 1s of year
      putchar( ' ' );
      putchar( bcd2ascii_high(hh) ); // print 10s of hours
      putchar( bcd2ascii_low(hh) );  // print 1s of hours
      putchar( ':' );
      putchar( bcd2ascii_high(mm) ); // print 10s of minutes
      putchar( bcd2ascii_low(mm) );  // print 1s of minutes
      putchar( ':' );
      putchar( bcd2ascii_high(ss) ); // print 10s of seconds
      putchar( bcd2ascii_low(ss) );  // print 1s of seconds      
      putchar( LINEFEED );
      putchar( CARRIAGE );
    }
}
//............................................................................//
void MEM_erase(){ // Fill AT29C64 memory with 0xFF
  U16 j;
  for( j = 0x0000; j < 0x1FFE; j++ ) MEM_write_byte( 0xFF, j );

  /* LAST TWO BYTES ARE RESERVED FOR ADDRESS COUNTER, ERASED TO 0x00 */
  MEM_write_byte( 0x00, 0x1FFE );
  MEM_write_byte( 0x00, 0x1FFF );
}
//............................................................................//
U8 bcd2ascii_high( U8 bcd_byte ){ return(((bcd_byte&0xF0)>>4)+0x30); }
//............................................................................//
U8 bcd2ascii_low( U8 bcd_byte ){ return((bcd_byte&0x0F)+0x30); }
//............................................................................//
void I2C_start(){
  I2C_SDA = 1;
  I2C_SCL = 1;
  _nop_();
  _nop_();
  _nop_();
  _nop_();
  _nop_();
  I2C_SDA = 0;
  _nop_();
  _nop_();
  _nop_();
  I2C_SCL = 0;
}
//............................................................................//
void I2C_stop(){
  I2C_SDA = 0;
  _nop_();
  _nop_();
  _nop_();
  _nop_();
  _nop_();
  I2C_SCL = 1;
  _nop_();
  _nop_();
  _nop_();
  I2C_SDA = 1;  
}
//............................................................................//
bit I2C_tx( U8 tx_data ){
  U8 k;
  
  for( k = 0; k < 8; k++ )
    {
	  I2C_SDA = (( tx_data & MSB_MASK )?1:0);
	  tx_data <<= 1;                         
	  I2C_SCL = 1;
    _nop_();
    _nop_();
	  I2C_SCL = 0;
	}
  I2C_SDA = 1;
  _nop_();
  _nop_();
  _nop_();
  I2C_SCL = 1;
  _nop_();
  _nop_();
  _nop_();

  k = I2C_SDA;
  I2C_SCL = 0;
  return k; // ACK bit
}
//............................................................................//
U8 I2C_rx(){
  U8 k;
  U8 rx_data;
  
  for( k = 0; k < 8; k++ )
    {
	  rx_data <<= 1;
	  I2C_SCL = 1;
	  _nop_();
      _nop_();
	  rx_data |= I2C_SDA;
	  I2C_SCL = 0;
	}
  return rx_data;
}	
//............................................................................//
void ADC_clk(){
  _nop_();
  _nop_();
  ADC_CLK = 1;
  _nop_();
  _nop_();
  ADC_CLK = 0;
  _nop_();
}
//............................................................................//
void RTC_set_address( U8 addr ){
  bit ACK;
  
  ACK = I2C_tx( RTC_CTRL_WR ); 
  while(ACK);  
  ACK = I2C_tx( addr ); 
  while(ACK); 
}
//............................................................................//
void MEM_write_delay(){
  U16 j;
  for( j = 530; j > 0; j-- ) _nop_(); // ~5ms for the EEPROM to write the data in 
}
//............................................................................//
void MEM_set_address( U16 addr ){
  bit ACK;

  ACK = I2C_tx( MEM_CTRL_WR ); // send control byte for AT24C64 EEPROM write operation 
  while(ACK); 
  ACK = I2C_tx( addr/256 );    // send high byte of 13-bit address 
  while(ACK); 
  ACK = I2C_tx( addr%256 );    // send low byte of 13-bit address
  while(ACK); 
}
//............................................................................//
U8 RTC_read_byte( U8 rd_addr ){
  U8 stored_data;
  bit ACK;

  I2C_start();
  RTC_set_address( rd_addr );
  I2C_start();
  ACK = I2C_tx( RTC_CTRL_RD );
  while( ACK );      
  stored_data = I2C_rx();
  I2C_SDA = 1;
  _nop_();
  _nop_();
  _nop_();
  I2C_SCL = 1;
  _nop_();
  _nop_();
  _nop_();
  I2C_SCL = 0;
  I2C_stop();
  return stored_data;
}
//............................................................................//
void MEM_write_byte( U8 write_data, U16 wr_addr ){

  bit ACK;
  I2C_start();
  MEM_set_address( wr_addr );
  ACK = I2C_tx( write_data );
  while(ACK);
  I2C_stop();
  MEM_write_delay();        
}
//............................................................................//
U8 MEM_read_byte( U16 rd_addr ){
  U8 stored_data;
  bit ACK;

  I2C_start();
  MEM_set_address( rd_addr );
  I2C_start();
  ACK = I2C_tx( MEM_CTRL_RD );
  while(ACK);      
  stored_data = I2C_rx();
  I2C_SDA = 1;
  _nop_();
  _nop_();
  _nop_();
  I2C_SCL = 1;
  _nop_();
  _nop_();
  _nop_();
  I2C_SCL = 0;
  I2C_stop();
  return stored_data;
}
//............................................................................//
U8 ADC_read(){
  U8 k;
  U8 bdata j;

  ADC_SELECT = 0;
  ADC_clk();
  for( k = 0; k < 9; k++ ) { ADC_clk(); j <<= 1; if( ADC_DATA ) j++; }
  ADC_SELECT = 1;
  ADC_CLK = 1;
  _nop_();
  _nop_();
  return j;
}
//............................................................................//
U8 *get_sample(){
  U8 sample[7];

  sample[0] = ADC_read();
  sample[1] = RTC_read_byte( RTC_MONTH_REG   ) & MONTH_MASK  ;
  sample[2] = RTC_read_byte( RTC_DATE_REG    ) & DATE_MASK   ;
  sample[3] = RTC_read_byte( RTC_YEAR_REG    )               ;
  sample[4] = RTC_read_byte( RTC_HOURS_REG   ) & HOURS_MASK  ;
  sample[5] = RTC_read_byte( RTC_MINUTES_REG ) & MINUTES_MASK;
  sample[6] = RTC_read_byte( RTC_SECONDS_REG ) & SECONDS_MASK;
  return sample;
}
//............................................................................//
void write_sample(){
  U8 i;
  U16 k;

  U8 *sample = get_sample();

  for( i = 0; i < 7; i++ ) MEM_write_byte( sample[i], MEM_address_counter++ );
  // After every sample the last sampled address is backed up to memory
  k = MEM_address_counter; 
  MEM_write_byte( (k/256), 0x1FFE );
  MEM_write_byte( (k%256), 0x1FFF ); 
}
//............................................................................//
U16 MEM_last_address(){ // returns the last sample address
  U16 k;
  k   = MEM_read_byte( 0x1FFE );
  k <<= 8;
  k  |= MEM_read_byte( 0x1FFF ); 
  return k;
}

void print_continuous_sample(){
  U8 sample_value, k, ss, mm, hh, dd, nn, yy;
  U8 ss_compare = 0;

  while(1) // this prints out date/time every second
    {
	  sample_value = ADC_read();
	  ss = RTC_read_byte( RTC_SECONDS_REG ) & SECONDS_MASK; 
      mm = RTC_read_byte( RTC_MINUTES_REG ) & MINUTES_MASK;
      hh = RTC_read_byte( RTC_HOURS_REG   ) & HOURS_MASK  ;
      dd = RTC_read_byte( RTC_DATE_REG    ) & DATE_MASK   ;
      nn = RTC_read_byte( RTC_MONTH_REG   ) & MONTH_MASK  ;
      yy = RTC_read_byte( RTC_YEAR_REG    )               ;
  
      if( ss != ss_compare )
        {
	      putchar( '0' );                //..................
	      putchar( 'x' );                // Sample voltage  :             
	      k = ((sample_value&0xF0)>>4);  // from ADC in hex :
	      putchar( digit[k] );           // Change to       :
	      k = sample_value & 0x0F;       //  inches or cm   :
	      putchar( digit[k] );           //..................
	      putchar( ' ' );
		  putchar( bcd2ascii_high(nn) ); // print 10s of month
		  putchar( bcd2ascii_low(nn) );  // print 1s of month
	      putchar( '/' );
		  putchar( bcd2ascii_high(dd) ); // print 10s of date
		  putchar( bcd2ascii_low(dd) );  // print 1s of date
	      putchar( '/' );
		  putchar( bcd2ascii_high(yy) ); // print 10s of year
		  putchar( bcd2ascii_low(yy) );  // print 1s of year
	      putchar( ' ' );
		  putchar( bcd2ascii_high(hh) ); // print 10s of hours
		  putchar( bcd2ascii_low(hh) );  // print 1s of hours
	      putchar( ':' );
		  putchar( bcd2ascii_high(mm) ); // print 10s of minutes
		  putchar( bcd2ascii_low(mm) );  // print 1s of minutes
	      putchar( ':' );
		  putchar( bcd2ascii_high(ss) ); // print 10s of seconds
		  putchar( bcd2ascii_low(ss) );  // print 1s of seconds      
	      putchar( LINEFEED );
	      putchar( CARRIAGE );
		  ss_compare = ss;
        }
	  }
}

#if RTC_CONFIGURATION
void RTC_configure_and_test(){
  U8 ss;
  U8 ss_compare = 0;
  U8 mm;
  U8 hh;
  U8 dd;
  U8 nn;
  U8 yy;
  
  // DATE/TIME VALUES
  U8 seconds   = 0x00;       // xx:xx:00
  U8 minutes   = 0x10;       // xx:00:xx
  U8 hours     = 0x18;       // 16:xx:xx
  U8 date      = 0x12;       // 6th
  U8 month     = 0x12;       // December
  U8 year      = 0x12;       // 2012                  
                                                
  seconds |= RTC_START_OSC;  // Start the clock   

  RTC_write_byte( seconds, RTC_SECONDS_REG );
  RTC_write_byte( minutes, RTC_MINUTES_REG );
  RTC_write_byte( hours,   RTC_HOURS_REG   );
  RTC_write_byte( date,    RTC_DATE_REG    );
  RTC_write_byte( month,   RTC_MONTH_REG   );
  RTC_write_byte( year,    RTC_YEAR_REG    );  

  while(1) // this prints out date/time every second
    {
	    ss = RTC_read_byte( RTC_SECONDS_REG ) & SECONDS_MASK; 
    	mm = RTC_read_byte( RTC_MINUTES_REG ) & MINUTES_MASK;
      hh = RTC_read_byte( RTC_HOURS_REG   ) & HOURS_MASK  ;
      dd = RTC_read_byte( RTC_DATE_REG    ) & DATE_MASK   ;
      nn = RTC_read_byte( RTC_MONTH_REG   ) & MONTH_MASK  ;
      yy = RTC_read_byte( RTC_YEAR_REG    )               ;
  
      if( ss != ss_compare )
        {
	        putchar( bcd2ascii_high(nn) ); // print 10s of month
	        putchar( bcd2ascii_low(nn) );  // print 1s of month
          putchar( '/' );
	        putchar( bcd2ascii_high(dd) ); // print 10s of date
	        putchar( bcd2ascii_low(dd) );  // print 1s of date
          putchar( '/' );
	        putchar( bcd2ascii_high(yy) ); // print 10s of year
	        putchar( bcd2ascii_low(yy) );  // print 1s of year
          putchar( ' ' );
	        putchar( bcd2ascii_high(hh) ); // print 10s of hours
	        putchar( bcd2ascii_low(hh) );  // print 1s of hours
          putchar( ':' );
	        putchar( bcd2ascii_high(mm) ); // print 10s of minutes
	        putchar( bcd2ascii_low(mm) );  // print 1s of minutes
          putchar( ':' );
	        putchar( bcd2ascii_high(ss) ); // print 10s of seconds
	        putchar( bcd2ascii_low(ss) );  // print 1s of seconds      
          putchar( LINEFEED );
          putchar( CARRIAGE );
	        ss_compare = ss;
        }
	  }
}

void RTC_write_byte( U8 write_byte, U8 wr_addr ){
	bit ACK;

	I2C_start();					         
	ACK = I2C_tx( RTC_CTRL_WR );	 // RTC control
  while(ACK);                    // hold for slave acknowledgement
	ACK = I2C_tx( wr_addr );       //address
  while(ACK);                    // hold for slave acknowledgement
	ACK = I2C_tx( write_byte );    // write data
  while(ACK);                    // hold for slave acknowledgement
	I2C_stop();					           
}
#endif