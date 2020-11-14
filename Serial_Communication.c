#include "board.h"
#include "pm.h"
#include "flashc.h"
#include "gpio.h"
#include "eic.h"
#include "usart.h"

char rx;
char tx[2] = "de";

#define LED2 AVR32_PIN_PA05
#define ARRIBA AVR32_PIN_PB22
#define LED3 AVR32_PIN_PA06
//// OSCO
#define FOSC0 12000000
// Startup
#define STARTUP 3

/* USART */
#define USART               (&AVR32_USART0)				// AVR32_USART0, AVR32_USART1, AVR32_USART2, AVR32_USART3

#define USART_RX_PIN        0
#define USART_RX_FUNCTION   0
#define USART_TX_PIN        1
#define USART_TX_FUNCTION   0
#define PBA_FREQ_HZ			FOSC0

////////////////////////////////////EIC////////////////////////////////////////
__attribute__ ((interrupt))
void botones (void){
	if(gpio_get_pin_value(ARRIBA)==1){
		usart_write_line(USART, "Prender Led");
		//usart_write_char(USART, tx);
		gpio_tgl_gpio_pin(LED3);
		eic_clear_interrupt_line(&AVR32_EIC,1);
	}
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

int main(void)
{
	pm_switch_to_osc0(&AVR32_PM, FOSC0, STARTUP);
	usart_init();
	EIC();
	while(1){
		
		usart_read_char(USART, &rx);
		if (rx == '1'){
			gpio_clr_gpio_pin(LED2);
		}
	}
	return 1;
}