//-----------------------------------------------------------------------------
// Project: Switcher
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

//#define SYSCLK      		24500000   				// SYSCLK frequency in Hz
//#define BAUDRATE   		115200					// Baud rate of UART in bps
//
//#define UART_SIZE_IN		16
//#define UART_SIZE_OUT   16
//
//#define ADC1				0x08
//#define ADC2				0x09
//#define ADC3				0x0A
//
#define SCALE_MUL			6068					// When combined with >> 10 will scale by 5.926 to compensate for potential divider
//
//#define DEFAULT_OUT_MV		5000
//#define DEFAULT_HIGH_MV		6000
//#define DEFAULT_LOW_MV		4000
//
#define WAIT 100000
#define ADC3            0x0A

SBIT(LED1, SFR_P1, 0);                 			// DS5 P1.0 LED
SBIT(LED0, SFR_P1, 1);                 			// DS5 P1.0 LED

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

void  setup(void);
//void  uartLoadOut(U8 tx);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

volatile U16   motor_pwm_counter;
volatile U16   motor_duty_left_fwd;
volatile U16   motor_duty_left_rvr;
volatile U16   motor_duty_right_fwd;
volatile U16   motor_duty_right_rvr;

//volatile U8 	uart_in[UART_SIZE_IN];
//volatile U8 	uart_out[UART_SIZE_OUT];
//volatile U8 	head;
//volatile U8 	tail;

//-----------------------------------------------------------------------------
// Main Routine
//-----------------------------------------------------------------------------
U16 readAdc(){                      // Read the available ADCs
   U8 i;
   ADC0MX = ADC3;
   for(i=0;i<2;i++){
      ADC0CN0 |= ADC0CN0_ADBUSY__SET;
      while(ADC0CN0 & ADC0CN0_ADBUSY__SET);     // Wait for sample to complete
   }
   return (U16)ADC0;        // Scale to mV
}

void main (void){
   //U32 i//;
   U16 adc;
   
   motor_pwm_counter    = 0;
   motor_duty_left_fwd  = 0;
   motor_duty_left_rvr  = 0;
   motor_duty_right_fwd = 0;
   motor_duty_right_rvr = 0;
   
   setup();

	//SCON0_TI 	= 1; 
	//SCON0_RI 	= 0;
  
   //tail = 0;
   //head = 0;

   //SBUF0 = 'A';

   LED0 = 1;
   LED1 = 1;
   //uartLoadOut('A');	
   //EA = 1;
	while (1){
      adc =  readAdc() >> 2;
      motor_duty_left_fwd = adc;
      motor_duty_right_fwd = 0x00FF - adc;
      
      //if(wait > 512){
      //   LED0 = 1;
      //}else{
      //   LED0 = 0;
      //}
      
      //for(i=0;i<WAIT;i++);
      //LED0 = 0;
      //LED1 = 1;
      //for(i=0;i<WAIT;i++);
      //LED0 = 1;
      //LED1 = 0;

      //uartLoadOut('A');	
      //
      //if(SCON0_RI){
		//	SCON0_RI = 0;
		//	uart_in[0] = SBUF0;
		//	uartLoadOut(uart_in[0]);	
      //} 
	}
} 

//-----------------------------------------------------------------------------
// Interrupt Routines
//-----------------------------------------------------------------------------

INTERRUPT (TIMER1_ISR, TIMER1_IRQn){}				// Needed for UART timing	


INTERRUPT (TIMER2_ISR, TIMER2_IRQn){				 
   if(0 == motor_pwm_counter){
      LED0 = 1;
      LED1 = 1;
   }
   if(motor_duty_left_fwd == motor_pwm_counter){ 
      LED0 = 0;
   }
   if(motor_duty_right_fwd == motor_pwm_counter){ 
      LED1 = 0;
   }
   motor_pwm_counter = (motor_pwm_counter + 1) % 255;
}


//-----------------------------------------------------------------------------
// Routines
//-----------------------------------------------------------------------------

//void uartLoadOut(U8 tx){			      // Handle buffering out Tx UART
//	uart_out[head] = tx;	               // Buffer outgoing
//	head++;						
//	head %= UART_SIZE_OUT;				   // Wrap around
//}

void setup(void){
   // Start of peripheral setup
	//U8 TCON_save;
	// Watchdog 
		WDTCN = 0xDE; 								// First key
		WDTCN = 0xAD; 								// Second key - Watchdog now disabled
	// Clock
		//CLKSEL = 
		//	CLKSEL_CLKSL__HFOSC 				| 	// Use 24.5MHz interal clock
		//	CLKSEL_CLKDIV__SYSCLK_DIV_1;			// Do not divide
	// Port 0
		P0MDOUT = 
			P0MDOUT_B0__OPEN_DRAIN 				| 	// PWM1 output
			P0MDOUT_B1__OPEN_DRAIN 				| 	// PWM2 output
			P0MDOUT_B2__OPEN_DRAIN				| 
			P0MDOUT_B3__OPEN_DRAIN 				| 
			P0MDOUT_B4__OPEN_DRAIN 				| 	// UART TX
			P0MDOUT_B5__OPEN_DRAIN				| 	// UART RX
			P0MDOUT_B6__OPEN_DRAIN 				| 
			P0MDOUT_B7__OPEN_DRAIN;
	// Port 1
		P1MDOUT = 
			P1MDOUT_B0__PUSH_PULL 				| 
			P1MDOUT_B1__OPEN_DRAIN 				| 
			P1MDOUT_B2__OPEN_DRAIN				| 
			P1MDOUT_B3__OPEN_DRAIN 				| 	// TEST2
			P1MDOUT_B4__OPEN_DRAIN 				| 	// TEST1
			P1MDOUT_B5__OPEN_DRAIN				| 
			P1MDOUT_B6__OPEN_DRAIN 				| 
			P1MDOUT_B7__OPEN_DRAIN;
		P1MDIN = 
			P1MDIN_B0__DIGITAL 					|	// ADC1
			P1MDIN_B1__DIGITAL 					| 	// ADC2
			P1MDIN_B2__ANALOG					| 	// ADC3
			P1MDIN_B3__DIGITAL 					| 
			P1MDIN_B4__DIGITAL 					| 
			P1MDIN_B5__DIGITAL					| 
			P1MDIN_B6__DIGITAL 					| 
			P1MDIN_B7__DIGITAL;
	
      //P1SKIP =
      //   P1SKIP_B0__SKIPPED;
      // Port crossbar
//		XBR0 = 
//			XBR0_URT0E__DISABLED 				| 	// Route out UART
//			XBR0_SPI0E__DISABLED 				| 
//			XBR0_SMB0E__DISABLED				| 
//			XBR0_CP0E__DISABLED 				| 
//			XBR0_CP0AE__DISABLED 				| 
//			XBR0_CP1E__DISABLED					| 
//			XBR0_CP1AE__DISABLED 				|
//			XBR0_SYSCKE__DISABLED;
//		XBR1 = 
//			XBR1_PCA0ME__CEX0_CEX1 				| 	// Route out PCA0 and PCA1
//			XBR1_ECIE__DISABLED 				| 
//			XBR1_T0E__DISABLED					| 
//			XBR1_T1E__DISABLED 					|
//			XBR1_T2E__DISABLED;
		XBR2 = 
			XBR2_WEAKPUD__PULL_UPS_ENABLED 		| 	// Weak pull ups
			XBR2_XBARE__ENABLED;					// Enable cross bar
	// ADC
		//ADC0MX = 									// Mux set in application
		//	ADC0MX_ADC0MX__ADC0P10;
		ADC0CF = 
			(1 << ADC0CF_ADSC__SHIFT) 			| 
			ADC0CF_AD8BE__NORMAL 				| 	// ADC set to 10 bit
			ADC0CF_ADGN__GAIN_1 				|	// ADC gain set to 1
			ADC0CF_ADTM__TRACK_NORMAL;				// Immediate covert
		ADC0CN0 &=
			~ADC0CN0_ADCM__FMASK;
		ADC0CN0 |= 
			ADC0CN0_ADEN__ENABLED				|
			ADC0CN0_ADCM__ADBUSY;
//	// Timer 0
//		TCON_save = TCON;
//		TCON &= 
//			TCON_TR0__BMASK 					&	 
//			TCON_TR1__BMASK;
//		TH1 = (150 << TH1_TH1__SHIFT);
//		TL1 = (150 << TL1_TL1__SHIFT);
//		TCON = 
//			TCON_save;
	// Timer setup
		CKCON = 
			CKCON_SCA__SYSCLK_DIV_12 			| 
			CKCON_T0M__PRESCALE 				| 
			CKCON_T3MH__EXTERNAL_CLOCK 			| 
			CKCON_T3ML__EXTERNAL_CLOCK			| 
			CKCON_T1M__SYSCLK;
		TMOD = 
			TMOD_T0M__MODE0 					| 
			TMOD_CT0__TIMER 					| 
			TMOD_GATE0__DISABLED				| 
			TMOD_T1M__MODE2 					| 
			TMOD_CT1__TIMER 					| 
			TMOD_GATE1__DISABLED;
		TCON |= 
			TCON_TR1__RUN ;
	// UART
		SCON0 |= 
			SCON0_REN__RECEIVE_ENABLED;
	// VREF
		REF0CN = 
			REF0CN_REFSL__VDD_PIN 				| 
			REF0CN_IREFLVL__1P65 				| 
			REF0CN_GNDSL__GND_PIN				| 
			REF0CN_TEMPE__TEMP_DISABLED;
//	// RSTSRC
//		RSTSRC = 
//			RSTSRC_C0RSEF__NOT_SET 				| 
//			RSTSRC_MCDRSF__SET 					| 
//			RSTSRC_PORSF__SET					| 
//			RSTSRC_SWRSF__NOT_SET;
	// Interrupt
		IE = 
			IE_EA__ENABLED 						| 
			IE_EX0__DISABLED 					| 
			IE_EX1__DISABLED 					| 
			IE_ESPI0__DISABLED					| 
			IE_ET0__DISABLED 					| 
			IE_ET1__DISABLED						| 	// Timer 1 enabled
			IE_ET2__ENABLED 					| 	// Timer 2 enabled
			IE_ES0__DISABLED;
		IP = 
			IP_PX0__LOW 						| 
			IP_PX1__LOW 						| 
			IP_PSPI0__LOW 						| 
			IP_PT0__LOW 						| 
			IP_PT1__LOW							|
			IP_PT2__HIGH 						| 	// Timer 2 highest priority
			IP_PS0__LOW;
//	// PCA
//		PCA0CN_CR =
//			PCA0CN_CR__STOP;
//		PCA0CPM0 = 
//			PCA0CPM0_CAPN__DISABLED 			| 
//			PCA0CPM0_ECCF__ENABLED 				| 
//			PCA0CPM0_MAT__ENABLED				| 
//			PCA0CPM0_CAPP__DISABLED 			| 
//			PCA0CPM0_ECOM__ENABLED				| 
//			PCA0CPM0_PWM__ENABLED 				| 
//			PCA0CPM0_TOG__DISABLED;
//		PCA0CPM1 = 
//			PCA0CPM1_CAPN__DISABLED 			| 
//			PCA0CPM1_ECCF__ENABLED 				|		 
//			PCA0CPM1_MAT__ENABLED				| 
//			PCA0CPM1_CAPP__DISABLED 			| 
//			PCA0CPM1_ECOM__ENABLED				| 
//			PCA0CPM1_PWM__ENABLED 				| 
//			PCA0CPM1_TOG__DISABLED;
//		PCA0MD = 
//			PCA0MD_CIDL__NORMAL 				| 
//			PCA0MD_ECF__OVF_INT_DISABLED 		| 
//			PCA0MD_CPS__SYSCLK;		
//		PCA0CN |= 
//			PCA0CN_CR__RUN;
	// Timer 2
		TMR2CN |= TMR2CN_TR2__RUN;
	// End of peripheral setup
}

//-----------------------------------------------------------------------------
// END
//-----------------------------------------------------------------------------
