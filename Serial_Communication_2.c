#include <asf.h>
#include "preprocessor.h"
#include "compiler.h"
#include "pm.h"
#include <avr32/io.h>
#include "board.h"
#include "gpio.h"
#include "power_clocks_lib.h"
#include "intc.h"
#include "eic.h"
#include "flashc.h"
#include <asf.h>
#include <stdbool.h>
#include <stdint.h>
#include "usart.h"
#include "et024006dhu.h"
#include "pwm.h"
#include "pdca.h"
#include "string.h"
#include "stdio.h"

volatile int banderaBoton = 0;
volatile int fin = 0;
volatile char buffer_rx[50];
volatile char buffer_temp;
volatile int result = 0;
volatile char boton[50] = "Boton apretado";

#define LED0a AVR32_PIN_PB27
#define LED1a AVR32_PIN_PB28
#define LED2a AVR32_PIN_PA05
#define LED3a AVR32_PIN_PA06

#define ARRIBA AVR32_PIN_PB22
#define ABAJO AVR32_PIN_PB23
#define DERECHA AVR32_PIN_PB24
#define IZQUIERDA AVR32_PIN_PB25
#define CENTRO AVR32_PIN_PB26

#define PDCA_CHANNEL_USART	0
#define PID_USART_RX 2

#define FOSC0 12000000
#define STARTUP 3

////////////////////////////////////DISPLAY/////////////////////////////////////////
avr32_pwm_channel_t pwm_channel6 = {
	.cdty = 0,
	.cprd = 100
};

static void tft_bl_init(void)
{
	pwm_opt_t opt = {
		.diva = 0,
		.divb = 0,
		.prea = 0,
		.preb = 0
	};
	
	pwm_init(&opt);
	pwm_channel6.CMR.calg = PWM_MODE_LEFT_ALIGNED;
	pwm_channel6.CMR.cpol = PWM_POLARITY_HIGH; //PWM_POLARITY_LOW;//PWM_POLARITY_HIGH;
	pwm_channel6.CMR.cpd = PWM_UPDATE_DUTY;
	pwm_channel6.CMR.cpre = AVR32_PWM_CMR_CPRE_MCK_DIV_2;

	pwm_channel_init(6, &pwm_channel6);
	pwm_start_channels(AVR32_PWM_ENA_CHID6_MASK);
}

/* USART */
#define USART               (&AVR32_USART0)				// AVR32_USART0, AVR32_USART1, AVR32_USART2, AVR32_USART3
#define USART_RX_PIN        0
#define USART_RX_FUNCTION   0
#define USART_TX_PIN        1
#define USART_TX_FUNCTION   0
#define PBA_FREQ_HZ			FOSC0

void DMA_USART();
void EIC();
void usart_init();
void init_Display();


////////////////////////////////////EIC////////////////////////////////////////
__attribute__ ((interrupt))
void botones (void){
	if(gpio_get_pin_value(ARRIBA)==1){
		usart_write_line(USART, "Prender Led");
		gpio_tgl_gpio_pin(LED3);
		eic_clear_interrupt_line(&AVR32_EIC,1);
	}
	if(gpio_get_pin_value(ABAJO)==1){
		banderaBoton = 1;
		eic_clear_interrupt_line(&AVR32_EIC,1);
	}
}

__attribute__ ((interrupt))
void USART_DMA(void){
	pdca_disable_interrupt_transfer_complete(PDCA_CHANNEL_USART);
	pdca_disable(PDCA_CHANNEL_USART);
	fin = 1;
}


int main(void)
{
	pm_switch_to_osc0(&AVR32_PM, FOSC0, STARTUP);
	init_Display();
	EIC();
	usart_init();
	Enable_global_interrupt();
	while(1){
		if(banderaBoton == 1){
			//buffer_temp = 'x';
			for(int i = 0; i < 50; i++){
				DMA_USART();
				while(fin == 0){
					banderaBoton = banderaBoton;
				}
				if(buffer_temp == 'q'){
					banderaBoton = 0;
					break;
				}
				buffer_rx[i] = buffer_temp;
			}
			et024006_PrintString(buffer_rx, (const unsigned char *)&FONT8x8, 0, 40, RED, -1);
			result = strcmp(buffer_rx, "Boton apretado");
			if(result == 0){
				gpio_clr_gpio_pin(LED0a);
				}else{
				gpio_clr_gpio_pin(LED3a);
			}
			for(int i = 0; i<50; i++){
				buffer_rx[i] = NULL;
			}
		}
	}
	
	return 1;
}

void EIC(void) {
	Disable_global_interrupt();
	INTC_init_interrupts();
	INTC_register_interrupt(&botones, 33, 3); //Handler (Nombre de la funcion de interrupcion), IRQ, Nivel de prioridad (0,1,2,3 /mayor prioridad 3)
	gpio_enable_module_pin(22, 1); //Pin y su funcion especial
	eic_options_t eic_options;
	eic_options.eic_mode  = EIC_MODE_EDGE_TRIGGERED; //Interrupcion por nivel o por frente
	eic_options.eic_edge  = EIC_EDGE_RISING_EDGE; //Rising o Falling de acuerdo con el modo anterior
	eic_options.eic_async = EIC_SYNCH_MODE; //Modo sincrono o asincrono
	eic_options.eic_line  = 1; //Numero de linea
	eic_init(&AVR32_EIC, &eic_options, 1); //Inicializar el EIC
	eic_enable_line(&AVR32_EIC, 1); //Habilitar la linea
	eic_enable_interrupt_line(&AVR32_EIC, 1); //Habilitar la interrupcion para la linea
	Enable_global_interrupt();
}

void usart_init(void) {
	static const gpio_map_t USART_GPIO_MAP =
	{
		{USART_RX_PIN, USART_RX_FUNCTION},
		{USART_TX_PIN, USART_TX_FUNCTION}
	};

	// Opciones USART
	static const usart_options_t USART_OPTIONS =
	{
		.baudrate     = 57600,				// Baudrate (directo). Tabla 26.2
		.charlength   = 8,					// Número de bits (5-9 bits)
		.paritytype   = USART_NO_PARITY,	// Paridad (USART_ODD_PARITY, USART_EVEN_PARITY, USART_MARK_PARITY, USART_SPACE_PARITY, USART_NO_PARITY)
		.stopbits     = USART_1_STOPBIT,	// Bits de parada (USART_1_STOPBIT, USART_2_STOPBITS, USART_1_5_STOPBITS)
		.channelmode  = USART_NORMAL_CHMODE	// No mover
	};

	// Mapa de GPIO de USART.
	gpio_enable_module(USART_GPIO_MAP, sizeof(USART_GPIO_MAP) / sizeof(USART_GPIO_MAP[0]));

	// Inicializar USART en modo RS232.
	usart_init_rs232(USART, &USART_OPTIONS, PBA_FREQ_HZ);

}

void DMA_USART(void){
	// this PDCA channel is used for data reception from the SPI
	pdca_channel_options_t pdca_options_USART ={ // pdca channel options

		.addr = &buffer_temp,
		// memory address. We take here the address of the string dummy_data. This string is located in the file dummy.h
		.size = sizeof(buffer_temp),                              // transfer counter: here the size of the string
		.r_addr = NULL,                           // next memory address after 1st transfer complete
		.r_size = 0,                              // next transfer counter not used here
		.pid = PID_USART_RX,        // select peripheral ID - data are on reception from SPI1 RX line
		.transfer_size = 8  // select size of the transfer: 8,16,32 bits
	};

	// Init PDCA transmission channel
	pdca_init_channel(PDCA_CHANNEL_USART, &pdca_options_USART);


	//! \brief Enable pdca transfer interrupt when completed
	INTC_register_interrupt(&USART_DMA, 96, 0);  // pdca_channel_spi1_RX = 0
	pdca_enable_interrupt_transfer_complete(PDCA_CHANNEL_USART);
	pdca_enable(PDCA_CHANNEL_USART);
	fin = 0;
}

void init_Display (void){
	et024006_Init( FOSC0, FOSC0 );
	tft_bl_init();
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
	while(pwm_channel6.cdty < pwm_channel6.cprd)
	{
		pwm_channel6.cdty++;
		pwm_channel6.cupd = pwm_channel6.cdty;
		//pwm_channel6.cdty--;
		pwm_async_update_channel(AVR32_PWM_ENA_CHID6, &pwm_channel6);
		delay_ms(10);
	}
}
