// Rayan Hussain
// 400142835
// hussar9@mcmaster.ca

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "SCI.h" // necessary for serial communication


unsigned short value;
unsigned int x;
unsigned int y;

//counters for mode and start/stop, used in interrupts
unsigned int xyCounter;
unsigned int startStopCounter;

// function prototypes
void outputAngleBCD(unsigned int angle);
void delay1ms(unsigned int multiple);
void setClk(void);


void main(void) {

  
  //The next six assignment statements configure the Timer Input Capture                                                  
  // Edited to allow use of IOC0 and IOC2 (Dig Pins 3 and 7)          
  TSCR1 = 0x90;
  TSCR2 = 0x04;                   
  TIOS = 0xFA;                   
  PERT = 0x05;     
  TCTL3 = 0x00;    
  TCTL4 = 0x22;    
  TIE = 0x05;   //Timer Interrupt Enable        
  
	EnableInterrupts; //CodeWarrior's method of enabling interrupts
 
  // ADC Channel configuration
  ATDCTL1 = 0x4F;		// set for 12-bit resolution (1001111) 
	ATDCTL3 = 0x88;		// right justified, one sample per sequence
	ATDCTL4 = 0x01;		// prescaler = 1; 4MHz / (2 * (1 + 1)) == 1MHz
	ATDCTL5 = 0x25;		// continuous conversion on channel 6 (AN6)

  // Set baud rate based on student specifc bus speed(E-clock)
  // Our specific E-clock speed was 4 MHz
  // baud divisor = 4000000/(16 * 19200) = 13.02.. <- less than 5% error off nearest whole number
  SCI_Init(19200);
  
  // Set clock speed
  //setClk();
  
  
  //Configure on board LED as output, we will use this to show user if measuring x or y values
  DDRJ = 0xFF; 
  PTJ = 0x00; //start powered off to show that we are currently measuring along the x axis
  
  // Configure PP0-PP3 Pin
  // Access using PTP to output
  DDRP = 0x0F;
  
  
  // Configure PAD0-PAD3 Pin
  // Access using PT1AD to output
  DDR1AD = 0x0F;

  
  // Check if serial communication with pc is established
  SCI_OutString("Connection with PC has been established");
  SCI_OutChar(CR);
  
  startStopCounter = 0;
  xyCounter = 0;
  
  while(1){
    
    if(startStopCounter % 2 == 0){  // even number means start, odd number means stop 
    
      if(xyCounter % 2 == 0){  // even number runs MODE 0, odd number runs MODE 1

        ATDCTL5 = 0x26; // switch to channel 6 (AN6) for x axis, MODE 0
        
        value = ATDDR0; // store accelerometer value
        x = getAngleX(value);
      
        outputAngleBCD(x);
        SCI_OutString("x angle = ");
        SCI_OutUDec(x);
        SCI_OutChar(CR);
        delay1ms(110);   
        
      } else {
                              
        ATDCTL5 = 0x25; // switch to channel 5 (AN5) for y axis, MODE 1
        
        value = ATDDR0; // store accelerometer value
        y = getAngleY(value);
      
        outputAngleBCD(y);
        SCI_OutString("y angle = ");
        SCI_OutUDec(y);
        SCI_OutChar(CR);
        delay1ms(110);
      }
      
    }
    
  }
  

  for(;;) {
    _FEED_COP(); /* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}


// Function for receiving x angle using linear approximation
int getAngleX(unsigned short x){
 
 float angle;
 angle = 0.2369*x - 504.6;
 
 if(angle < 0){  
  angle = angle * -1;
 } 
 
 if (angle > 90){
  angle = 90;
 }
 
 return angle;
}


// Function for receiving y angle using linear approximation
int getAngleY(unsigned short y){
 
 float angle;
 angle = 0.2093*y - 420.7;
 
 if(angle < 0){
  angle = angle * -1;
 }
 
 if (angle > 90){
  angle = 90;
 }
 
 return angle;
}


//Function for setting the LEDs to the BCD value of the angle
void outputAngleBCD(unsigned int angle){
  
  PT1AD = angle%10; // the lower 4 bits, these represent the ones coloumn
  PTP = angle/10; // the upper 4 bits, these represent the tens coloumn
  
}


// Function for delay
// Lazy delay function to waste time for 1ms. Avoids the use of the timer, which is being used elsewhere
void delay1ms(unsigned int numTimes){
  unsigned int i;
  unsigned int j;
  
  for(j = 0; j<numTimes; j++){
    for(i = 0; i<68; i++){
      // Delay
      PTJ = PTJ;
      PTJ = PTJ;
      PTJ = PTJ;
      PTJ = PTJ;
      PTJ = PTJ;
    }
  }   
}


// Everything necessary for setting the clock speed

#define VCOFRQ 0x00        //VCOFRQ[1:0]  32MHz <= VCOCLK <= 48MHz
#define SYNDIV 0x05        //SYNDIV[5:0]  Syn divide is 11
#define REFFRQ 0x40        //REFFRQ[1:0]  2MHz < fREF <= 6MHz
#define REFDIV 0x01        //REFDIV[3:0]  Ref Divide is 1

void setClk(void){
  CPMUPROT = 0x26;               //Protection of clock configuration is disabled
								 // NOTE: On some Esduinos you may need to use CPMUPROT=0. Try both and see which value allows you to change the registers below in your debugger!
  
  CPMUCLKS = 0x80;               //PLLSEL=1. Select Bus Clock Source:  PLL clock or Oscillator.
  CPMUOSC = 0x80;                //OSCE=1. Select Clock Reference for PLLclk as:fOSC (8 MHz).
  
  CPMUREFDIV = REFFRQ+REFDIV;    //Set fREF divider and selects reference clock frequency Range. fREF= 4 MHz.  
  
  CPMUSYNR=VCOFRQ + SYNDIV;      //Set Syn divide and selects VCO frequency range. fVCO = 48 MHz.
  
  CPMUPOSTDIV=0x00;              //Set Post Divider (0x00= 0000 0000). fPLL= 48 MHz.
                                 //Overall clock speed is fBUS=fPLL/2=24 MHz  
  
  while (CPMUFLG_LOCK == 0) {}  //Wait for PLL to achieve desired tolerance of target frequency. NOTE: For use when the source clock is PLL. comment out when using external oscillator as source clock 
 
 CPMUPROT = 1;                  //Protection for clock configuration is reenabled 
  //Note: If you change your clock speed you will need to update delay1ms function to give a 1 ms delay
}



/*
 * This is the Interrupt Service Routine for TIC channel 0 (Code Warrior has predefined the name for you as "Vtimch0"                                                    
 */           
interrupt  VectorNumber_Vtimch0 void ISR_Vtimch0(void)
{

  unsigned int temp; //DON'T EDIT THIS

  xyCounter++;
  
  PTJ ^= 0x01; 

  temp = TC0;       //Refer back to TFFCA, we enabled FastFlagClear, thus by reading the Timer Capture input we automatically clear the flag, allowing another TIC interrupt
}
  
   /*
 * This is the Interrupt Service Routine for TIC channel 2 (Code Warrior has predefined the name for you as "Vtimch2"                                                    
 */           
interrupt  VectorNumber_Vtimch2 void ISR_Vtimch2(void)
{
  
  unsigned int temp; //DON'T EDIT THIS
  
  startStopCounter++;
  
  temp = TC2;       //Refer back to TFFCA, we enabled FastFlagClear, thus by reading the Timer Capture input we automatically clear the flag, allowing another TIC interrupt
}
  
