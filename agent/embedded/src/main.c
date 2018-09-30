//-----------------------------------------------------------------------------
// Project: Circus/embedded
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

#define SYSCLK      		24500000   				// SYSCLK frequency in Hz
#define BAUDRATE   		115200					// Baud rate of UART in bps
//
#define UART_SIZE_IN		8
#define UART_SIZE_OUT   20
#define ADC1				0x08
#define ADC2				0x09
#define ADC3				0x0A

SBIT(LED1, SFR_P1, 2);                       // P1.2 LED
SBIT(LED0, SFR_P1, 4);                 		// P1.4 LED

SBIT(RM, SFR_P1, 5);
SBIT(LM, SFR_P1, 6);

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

void u32_to_dec_str(U32 data, U8 * ptr);
void u32_to_hex_str(U32 data, U8 * ptr);
void  setup(void);
U8    uartRx();
void  uartLoadIn(U8 rx);
void  uartLoadOut(U8 tx);
U8    uartSizeIn(void);
void  uartTransmit(void);
U16   readAdc(void);
void  motorControl(void);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

volatile U16   motor_pwm_counter;
volatile    U16   motor_duty_left;
volatile    U16   motor_duty_right;


volatile U8 	uart_in[UART_SIZE_IN];
volatile U8 	in_head;
volatile U8 	in_tail;
volatile U8 	uart_out[UART_SIZE_OUT];
volatile U8 	out_head;
volatile U8 	out_tail;

//-----------------------------------------------------------------------------
// Main Routine
//-----------------------------------------------------------------------------

#define PERIOD 10000
void main (void){ 
   volatile U8 data;
   U32 i;
   U8 str[32];
  
   SCON0_TI 	= 1; 
	SCON0_RI 	= 0;
  

   in_tail  = 0;
   in_head  = 0;
   out_tail = 0;
   out_head = 0;
   
   setup();

  
   motor_pwm_counter    = 0;
   motor_duty_left      = 0; 
   motor_duty_right     = 0; 
   
   
   setup();

	SCON0_TI 	= 1; 
	SCON0_RI 	= 0;
  
   LM = 0;
   RM = 0;
   
   LED0     = 1;
   LED1     = 1;
  
   while (1){ 
      if(SCON0_RI){   
         SCON0_RI = 0;
         data = SBUF0;
         
         uartLoadOut('\n');	
         uartLoadOut('\r');	
         uartLoadOut(data); 
         if('A' == data){
            if(motor_duty_right < 255){	
               motor_duty_right+=8;
            }
            u32_to_dec_str((U32)motor_duty_right, str);
         }
         if('B' == data){
            if(motor_duty_right != 0){
               motor_duty_right-=8;
            }
            u32_to_dec_str((U32)motor_duty_right, str);
         }
         if('C' == data){
            if(motor_duty_left < 255){
               motor_duty_left+=8;
            }
            u32_to_dec_str((U32)motor_duty_left, str);
         }
         if('D' == data){;
            if(motor_duty_left != 0){
               motor_duty_left-=8;
            }
            u32_to_dec_str((U32)motor_duty_left, str);
         }
         for(i=0;i<10;i++){
            uartLoadOut(str[i]);
         } 
      }
	}
} 

//-----------------------------------------------------------------------------
// Interrupt Routines
//-----------------------------------------------------------------------------

INTERRUPT (TIMER1_ISR, TIMER1_IRQn){}				// Needed for UART timing	

INTERRUPT (TIMER2_ISR, TIMER2_IRQn){			   // Timer running at 4KHz 
   uartTransmit();  
   motor_pwm_counter++;
   if(motor_pwm_counter == 255){
      motor_pwm_counter = 0;
   } 
   if(0 == motor_pwm_counter){
      LM = 1;
      RM = 1;
   }
   if(motor_duty_left < motor_pwm_counter){ 
      LM = 0;
   }
   if(motor_duty_right < motor_pwm_counter){ 
      RM = 0;
   }
   TMR2H = 255;                                 // Reset timer
   TMR2CN_TF2H = 0; 
}

//-----------------------------------------------------------------------------
// Routines
//-----------------------------------------------------------------------------



// Turn 32 bit data in to a string of 10 chars
void u32_to_dec_str(U32 data, U8 * ptr){
   U8 * p;
   S8 i,j;
   U32 c, d, m;
   d = data;
   p = ptr;
   for(i=9;i>-1;i--){ 
      m = 1;
      for(j=0;j<i;j++){
         m *= 10;
      }
      c = d / m; 
      *p = (U8)c + 48;
      *p++;
      d -= c * m;
   }
}

// Turn 32 bit data In to a string of 8 chars
void u32_to_hex_str(U32 data, U8 * ptr){
   U8 * p;
   S8 i;
   U32 c, d;
   d = data;
   p = ptr;
   for(i=28;i>-1;i-=4){
      c = d >> i; 
      if(c > 9){
         *p = c + 55;
      }else{
         *p = c + 48;
      }
      *p++;
      d -= c << i;
   }
}

U8 uartRx(){
   U8 data = 0;
   if(in_head != in_tail){
      data = uart_in[in_tail];                   // Timer tuned so no need to check
      in_tail++;                                   // Transmit UART
      in_tail %= UART_SIZE_IN;                    // Wrap around
   }
   return data;
}

void uartLoadIn(U8 rx){
   uart_in[in_head] = rx;
   in_head++;
   in_head %= UART_SIZE_IN;
}

U8 uartSizeIn(void){
   if(in_head < in_tail){
      return UART_SIZE_IN - in_tail + in_head;
   }else{
      return in_head - in_tail;
   }
}

void uartLoadOut(U8 tx){			               // Handle buffering out Tx UART
	uart_out[out_head] = tx;	                  // Buffer outgoing						
	out_head++;
   out_head %= UART_SIZE_OUT;			            // Wrap around
}

void uartTransmit(void){
   if(out_head != out_tail){
      SBUF0 = uart_out[out_tail];                   // Timer tuned so no need to check
      out_tail++;                                   // Transmit UART
      out_tail %= UART_SIZE_OUT;                    // Wrap around
   }
}

U16 readAdc(void){                              // Read the available ADCs
   U8 i;
   ADC0MX = ADC3;
   for(i=0;i<2;i++){
      ADC0CN0 |= ADC0CN0_ADBUSY__SET;
      while(ADC0CN0 & ADC0CN0_ADBUSY__SET);     // Wait for sample to complete
   }
   return (U16)ADC0;        
}


void setup(void){ 
	U8 TCON_save;
	// Watchdog 
		WDTCN = 0xDE; 								      // First key
		WDTCN = 0xAD; 								      // Second key - Watchdog now disabled
	// Clock
		CLKSEL = 
			CLKSEL_CLKSL__HFOSC 				| 	      // Use 24.5MHz interal clock
			CLKSEL_CLKDIV__SYSCLK_DIV_1;			   // Do not divide
	// Port 0
		P0MDOUT = 
			P0MDOUT_B0__PUSH_PULL 				| 	// PWM1 output
			P0MDOUT_B1__PUSH_PULL 				| 	// PWM2 output
			P0MDOUT_B2__OPEN_DRAIN				| 
			P0MDOUT_B3__OPEN_DRAIN 				| 
			P0MDOUT_B4__PUSH_PULL 				| 	   // UART TX
			P0MDOUT_B5__OPEN_DRAIN				| 	   // UART RX
			P0MDOUT_B6__OPEN_DRAIN 				| 
			P0MDOUT_B7__OPEN_DRAIN;
	// Port 1
		P1MDOUT = 
			P1MDOUT_B0__PUSH_PULL 				| 
			P1MDOUT_B1__OPEN_DRAIN 				| 
			P1MDOUT_B2__PUSH_PULL				| 
			P1MDOUT_B3__PUSH_PULL 				| 	// TEST2
			P1MDOUT_B4__PUSH_PULL 				| 	// TEST1
			P1MDOUT_B5__PUSH_PULL				|  // RM_FWD
			P1MDOUT_B6__PUSH_PULL 				|  // LM_RVR
			P1MDOUT_B7__PUSH_PULL;              // LM_FWD
		P1MDIN = 
			P1MDIN_B0__DIGITAL 					|	// ADC1
			P1MDIN_B1__DIGITAL 					| 	// ADC2
			P1MDIN_B2__DIGITAL					   | 	// ADC3
			P1MDIN_B3__DIGITAL 					| 
			P1MDIN_B4__DIGITAL 					| 
			P1MDIN_B5__DIGITAL					| 
			P1MDIN_B6__DIGITAL 					| 
			P1MDIN_B7__DIGITAL;
	// Port 2
		P2MDOUT = 
			P2MDOUT_B0__PUSH_PULL 				| 
			P2MDOUT_B1__PUSH_PULL;             // RM_RVR  	
   // Port crossbar
		XBR0 = 
			XBR0_URT0E__ENABLED 				   | 	// Route out UART
			XBR0_SPI0E__DISABLED 				| 
			XBR0_SMB0E__DISABLED				   | 
			XBR0_CP0E__DISABLED 				   | 
			XBR0_CP0AE__DISABLED 				| 
			XBR0_CP1E__DISABLED					| 
			XBR0_CP1AE__DISABLED 				|
			XBR0_SYSCKE__DISABLED;
		XBR1 = 
			XBR1_PCA0ME__CEX0_CEX1 				| 	// Route out PCA0 and PCA1
			XBR1_ECIE__DISABLED 				   | 
			XBR1_T0E__DISABLED					| 
			XBR1_T1E__DISABLED 					|
			XBR1_T2E__DISABLED;
		XBR2 = 
			XBR2_WEAKPUD__PULL_UPS_ENABLED 	| 	   // Weak pull ups
			XBR2_XBARE__ENABLED;					      // Enable cross bar
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
	// Timer 0
		TCON_save = TCON;
		TCON &= 
			TCON_TR0__BMASK 					&	 
			TCON_TR1__BMASK;
		TH1 = (150 << TH1_TH1__SHIFT);
		TL1 = (150 << TL1_TL1__SHIFT);
		TCON = 
			TCON_save;
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
	// RSTSRC
		RSTSRC = 
			RSTSRC_C0RSEF__NOT_SET 				| 
			RSTSRC_MCDRSF__SET 					| 
			RSTSRC_PORSF__SET					| 
			RSTSRC_SWRSF__NOT_SET;
	// Interrupt
		IE = 
			IE_EA__ENABLED 						| 
			IE_EX0__DISABLED 					| 
			IE_EX1__DISABLED 					| 
			IE_ESPI0__DISABLED					| 
			IE_ET0__DISABLED 					| 
			IE_ET1__ENABLED						| 	// Timer 1 enabled
			IE_ET2__ENABLED 					| 	// Timer 2 enabled
			IE_ES0__ENABLED;
		IP = 
			IP_PX0__LOW 						| 
			IP_PX1__LOW 						| 
			IP_PSPI0__LOW 						| 
			IP_PT0__LOW 						| 
			IP_PT1__LOW							|
			IP_PT2__HIGH 						| 	// Timer 2 highest priority
			IP_PS0__LOW;
	// PCA
		PCA0CN_CR =
			PCA0CN_CR__STOP;
		PCA0CPM0 = 
			PCA0CPM0_CAPN__DISABLED 			| 
			PCA0CPM0_ECCF__ENABLED 				| 
			PCA0CPM0_MAT__ENABLED				| 
			PCA0CPM0_CAPP__DISABLED 			| 
			PCA0CPM0_ECOM__ENABLED				| 
			PCA0CPM0_PWM__ENABLED 				| 
			PCA0CPM0_TOG__DISABLED;
		PCA0CPM1 = 
			PCA0CPM1_CAPN__DISABLED 			| 
			PCA0CPM1_ECCF__ENABLED 				|		 
			PCA0CPM1_MAT__ENABLED				| 
			PCA0CPM1_CAPP__DISABLED 			| 
			PCA0CPM1_ECOM__ENABLED				| 
			PCA0CPM1_PWM__ENABLED 				| 
			PCA0CPM1_TOG__DISABLED;
		PCA0MD = 
			PCA0MD_CIDL__NORMAL 				| 
			PCA0MD_ECF__OVF_INT_DISABLED 		| 
			PCA0MD_CPS__SYSCLK;		
		PCA0CN |= 
			PCA0CN_CR__RUN;
	// Timer 2
		TMR2CN |= TMR2CN_TR2__RUN;
	// End of peripheral setup
}

//-----------------------------------------------------------------------------
// END
//-----------------------------------------------------------------------------
