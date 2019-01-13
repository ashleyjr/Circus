//-----------------------------------------------------------------------------
// Project: Circus/broadcast
// File: 	main.c
// Brief:	Single main file containing all code
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "SI_C8051F850_Register_Enums.h"
#include "SI_C8051F850_Defs.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

#define SYSCLK      	24500000   		   // SYSCLK frequency in Hz
#define BAUDRATE     9600			      // Baud rate of UART in bps

SBIT(LEDR, SFR_P1, 2);                 		// P1.2 LED

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void setup(void);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

volatile U8 carrier_on;

//-----------------------------------------------------------------------------
// Main Routine
//-----------------------------------------------------------------------------

void main (void){
   // Init 
   U32 i;
   carrier_on  = 1;
   LEDR        = 0;
   // !Init

   // Setup
   
   // Disabled watchdogk
   WDTCN    = 0xDE;
   WDTCN    = 0xAD;
	
   // CPU clock
   CLKSEL   = CLKSEL_CLKSL__HFOSC | 
              CLKSEL_CLKDIV__SYSCLK_DIV_1;	 
  
   // Setup XBAR  
   P0MDOUT  = P0MDOUT_B4__PUSH_PULL;            
   P1MDOUT  = P1MDOUT_B1__PUSH_PULL|
              P1MDOUT_B2__PUSH_PULL; 
   P0SKIP   = 0xEF;                             // Skip all but UART pin
   P1SKIP   = 0xFD;	                           // Skip all but PWM pin
   XBR0     = XBR0_URT0E__ENABLED;              // Route out UART
   XBR1     = XBR1_PCA0ME__CEX0;                // Route out PCA0.CEX0 to P1.1
   XBR2     = XBR2_WEAKPUD__PULL_UPS_ENABLED | 
              XBR2_XBARE__ENABLED;					 
     
   // Start 38KHz clock
   PCA0CN   = PCA0CN_CR__STOP;
   PCA0CPH0 = 27;
   PCA0CPL0 = 0;
   PCA0MD   = PCA0MD_CPS__SYSCLK_DIV_12;		
   PCA0CPM0 = PCA0CPM0_ECOM__ENABLED  | 
			     PCA0CPM0_TOG__ENABLED   | 
              PCA0CPM0_MAT__ENABLED   | 
              PCA0CPM0_PWM__ENABLED   |
              PCA0CPM0_PWM16__16_BIT;
   PCA0CN   = PCA0CN_CR__RUN;  
            
   // Setup 1200 Baud UART and BAUD gen on timer 1
	CKCON    = CKCON_SCA__SYSCLK_DIV_48;
	TMOD     = TMOD_T1M__MODE2;
	TCON     = TCON_TR1__RUN; 
   TH1      = 0x2B;                             // Magic values from datasheet
	TL1      = 0x2B;

   // Setup pin interrupt
   TCON    |= TCON_IT0__EDGE;
   IT01CF   = IT01CF_IN1PL__ACTIVE_LOW |
              IT01CF_IN1SL__P0_3       |
              IT01CF_IN0PL__ACTIVE_HIGH |
              IT01CF_IN0SL__P0_3; 
   
   // Interrupts 
   IE       = IE_EA__ENABLED  |
              IE_EX0__ENABLED |
              IE_EX1__ENABLED; 
   // !Setup
 
   while(1){
      for(i=0;i<400000;i++);
      SBUF0 = 'A';
   }; 
} 

//-----------------------------------------------------------------------------
// Interrupt Routines
//-----------------------------------------------------------------------------

INTERRUPT (INT0_ISR, INT0_IRQn){				      // Attached to UART output	  
   IT01CF   |= IT01CF_IN0PL__ACTIVE_HIGH;
   PCA0CN   = PCA0CN_CR__STOP;
}

INTERRUPT (INT1_ISR, INT1_IRQn){				      // Attached to UART output	 
   IT01CF   |= IT01CF_IN0PL__ACTIVE_LOW;
   PCA0CN    = PCA0CN_CR__RUN;
}
//-----------------------------------------------------------------------------
// END
//-----------------------------------------------------------------------------
