// Rayan Hussain
// 400142835
// hussar9@mcmaster.ca

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "SCI.h" // necessary for serial communication



unsigned short value;
unsigned int x;
unsigned int y;
unsigned int xyCounter;
signed int xy_Toggle;
signed int startStop_Toggle;

// function prototypes
void outputAngleBCD(unsigned int angle);
void delay1ms(unsigned int multiple);
void setClk(void);


void main(void) {

  /*
 * The next six assignment statements configure the Timer Input Capture                                                   
 */           
  TSCR1 = 0x90;
  TSCR2 = 0x04;                   
  TIOS = 0xFC;                   
  PERT = 0x03;     
  TCTL3 = 0x00;    
  TCTL4 = 0x0A;    
  TIE = 0x03;   //Timer Interrupt Enable        
  
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
  
  // Configure PP0-PP7 Pin
  // Access using PTP to output
  DDRP = 0x0F;
  
  
  // Configure PAD0-PAD3 Pin
  // Access using PT1AD to output
  DDR1AD = 0x0F;

  
  // Check if serial communication with pc is established
  SCI_OutString("Connection with PC has been established");
  SCI_OutChar(CR);
  
  xyCounter = 0;
  
  while(1){
    
    
    if(xyCounter % 2 == 0){

      ATDCTL5 = 0x26; // switch to channel 6 (AN6) for x axis
      
      value = ATDDR0; // store accelerometer value
      x = getAngleX(value);
    
      outputAngleBCD(x);
      SCI_OutUDec(x);
      SCI_OutChar(CR);
      //delay1ms(100);   
      
    } else {
    

                            
      ATDCTL5 = 0x25; // switch to channel 5 (AN5) for y axis
      
      value = ATDDR0; // store accelerometer value
      y = getAngleY(value);
    
      outputAngleBCD(y);
      SCI_OutUDec(y);
      SCI_OutChar(CR);
      //delay1ms(100);
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
 return angle;
}


// Function for receiving y angle using linear approximation
int getAngleY(unsigned short y){
 
 float angle;
 angle = 0.2093*y - 420.7;
 
 if(angle < 0){
  angle = angle * -1;
 }
 return angle;
}


//Function for setting the LEDs to the BCD value of the angle
void outputAngleBCD(unsigned int angle){
  
  PT1AD = angle%10; // the lower 4 bits, these represent the ones coloumn
  PTP = angle/10; // the upper 4 bits, these represent the tens coloumn
  
}


// Function for delay
void delay1ms(unsigned int multiple){
  
  unsigned int i; //loop control variable
  
  TSCR1 = 0x90;   //enable timer and fast timer flag clear
  
  TSCR2 = 0x00;   //Disable timer interrupt, set prescaler=1
  
  TIOS |= 0x01;   //Enable OC0 (not necessary)
  
  TC0 = TCNT + 24000;
   
  for(i=0;i<multiple;i++) {
    TFLG2 = 0x80; //clear the TOF flag
    while (!(TFLG1_C0F));
    TC0 += 24000;
  }
  
  TIOS &= -0x01; //Disable OC0 (not necessary) 
  
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

  //xy_Toggle *= -1;

  xyCounter++;
  
  PTJ ^= 0x01; 

  temp = TC0;       //Refer back to TFFCA, we enabled FastFlagClear, thus by reading the Timer Capture input we automatically clear the flag, allowing another TIC interrupt
  }
