// Rayan Hussain
// 400142835
// hussar9@mcmaster.ca

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "SCI.h" // necessary for serial communication



void main(void) {

  // Set baud rate based on student specifc bus speed(E-clock)
  // Our specific E-clock speed was 4 MHz
  // baud divisor = 4000000/(16 * 19200) = 13.02.. <- less than 5% error off nearest whole number
  SCI_Init(19200);
  
  
  DDRJ = 0xFF;
  
  // Configure PP0-PP7 Pin
  // Access using PTP to output
  DDRP = 0x0F;
  
  
  // Configure PAD0-PAD7 Pin
  // Access using PT1AD to output
  DDR1AD = 0x0F;
  
  // Check if serial communication with pc is established
  SCI_OutString("Connection with PC has been established");
  SCI_OutChar(CR);
  
  // Lower 4 bits (ones)
  PT1AD = 0b00000001;
  PT1AD = 0b00000010;
  PT1AD = 0b00000100;
  PT1AD = 0b00001000;
  
  PT1AD = 0b00000000;
  
  // Upper 4 bits (tens)
  PTP = 0b00000001;
  PTP = 0b00000010;
  PTP = 0b00000100;
  PTP = 0b00001000;
  
  PTP = 0b00000000;
  

	EnableInterrupts;


  for(;;) {
    _FEED_COP(); /* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}
