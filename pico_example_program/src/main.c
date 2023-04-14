#include <hardware/gpio.h>
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <pico_usb_console/protocol.h>
#include <pico_usb_console/serial.h>



static void _input_callback(unsigned char length,const char* data){
	if (!length){
		PICO_USB_CONSOLE_PROTOCOL_ERROR("Empty input");
	}
	else{
		pico_usb_console_protocol_send_log(0,"[%u]: %s",length,data);
	}
}



int main(void){
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN,GPIO_OUT);
	gpio_put(PICO_DEFAULT_LED_PIN,1);
	pico_usb_console_init();
	pico_usb_console_protocol_set_input_callback(_input_callback);
	watchdog_enable(500,0);
	while (1){
		pico_usb_console_update();
		pico_usb_console_protocol_update();
		watchdog_update();
	}
	reset_usb_boot(0,0);
}
