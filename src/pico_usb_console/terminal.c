#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>



static struct termios _terminal_old_config;



void terminal_init(void){
	tcgetattr(STDOUT_FILENO,&_terminal_old_config);
	struct termios terminal_config=_terminal_old_config;
	terminal_config.c_iflag=(terminal_config.c_iflag&(~(INLCR|IGNBRK)))|ICRNL;
	terminal_config.c_lflag=(terminal_config.c_lflag&(~(ICANON|ECHO)))|ISIG|IEXTEN;
	tcsetattr(STDIN_FILENO,TCSANOW,&terminal_config);
}



void terminal_deinit(void){
	tcsetattr(STDOUT_FILENO,TCSANOW,&_terminal_old_config);
}



unsigned int terminal_get_size(unsigned int* height){
	struct winsize window_size;
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&window_size);
	if (height){
		*height=window_size.ws_row;
	}
	return window_size.ws_col;
}
