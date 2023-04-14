#include <device.h>
#include <protocol.h>
#include <ui.h>
#include <unistd.h>



#define BUILD_COMMAND(a,b) (((a)<<8)|(b))



void keyboard_read_inputs(void){
	char command[4];
	int count=read(STDIN_FILENO,command,4);
	command[1]=(count>2?command[2]:0);
	switch (BUILD_COMMAND(command[0],command[1])){
		case BUILD_COMMAND(4,0): // ctrl+D
			protcol_reset_server();
			device_close();
			break;
		case BUILD_COMMAND(9,0): // tab | ctrl+I
			ui_type_key(UI_KEY_CHANGE_FOCUS);
			break;
		case BUILD_COMMAND(10,0): // enter
			{
				unsigned int length;
				const char* data=ui_get_input(&length);
				protocol_send_input(data,length);
				ui_clear_input();
				break;
			}
		case BUILD_COMMAND(15,0): // ctrl+O
			ui_clear_output();
			break;
		case BUILD_COMMAND(127,0): // backspace
			ui_type_key(UI_KEY_BACKSPACE);
			break;
		case BUILD_COMMAND(27,51): // delete
			ui_type_key(UI_KEY_DELETE);
			break;
		case BUILD_COMMAND(27,65): // up arrow
			ui_type_key(UI_KEY_UP);
			break;
		case BUILD_COMMAND(27,66): // down arrow
			ui_type_key(UI_KEY_DOWN);
			break;
		case BUILD_COMMAND(27,67): // right arrow
			ui_type_key(UI_KEY_RIGHT);
			break;
		case BUILD_COMMAND(27,68): // left arrow
			ui_type_key(UI_KEY_LEFT);
			break;
		case BUILD_COMMAND(27,70): // end
			ui_type_key(UI_KEY_END);
			break;
		case BUILD_COMMAND(27,72): // home
			ui_type_key(UI_KEY_HOME);
			break;
		case BUILD_COMMAND(27,90): // shift+tab
			ui_type_key('\t');
			break;
		default:
			if (!command[1]&&(command[0]>31&&command[0]<127)){
				ui_type_key(command[0]);
			}
	}
}
