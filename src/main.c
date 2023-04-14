#include <device.h>
#include <keyboard.h>
#include <poll.h>
#include <signal.h>
#include <terminal.h>
#include <ui.h>
#include <unistd.h>



static _Bool _end_program=0;



static void _end_program_callback(int sig){
	_end_program=1;
}



int main(void){
	device_init();
	terminal_init();
	ui_init();
	signal(SIGINT,_end_program_callback);
	struct pollfd poll_config[2]={
		{
			STDIN_FILENO,
			POLLIN
		},
		{
			-1,
			POLLIN
		}
	};
	while (!_end_program){
		device_find_if_not_connected();
		ui_redraw();
		poll_config[1].fd=device_get_fd();
		poll_config[1].revents=0;
		poll(poll_config,(poll_config[1].fd==-1?1:2),500);
		if (poll_config[0].revents&POLLIN){
			keyboard_read_inputs();
		}
		if (poll_config[1].revents&(POLLERR|POLLHUP)){
			device_close();
		}
		if (poll_config[1].revents&POLLIN){
			device_read_data();
		}
	}
	ui_deinit();
	terminal_deinit();
	device_deinit();
	return 0;
}
