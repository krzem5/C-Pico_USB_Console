#include <device.h>
#include <protocol.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <terminal.h>
#include <ui.h>



#define UI_LINE_FLAG_INPUT 1
#define UI_LINE_FLAG_HEAD 2
#define UI_LINE_FLAG_ERROR 4



typedef struct _UI_LINE{
	unsigned int flags;
	char* data;
} ui_line_t;



typedef struct _UI_STYLES{
	const char* console_background;
	const char* console_input_color;
	const char* console_output_color;
	const char* console_error_color;
	const char* console_arrow_color;
	const char* console_scroll_bar;
	const char* input_background;
	const char* input_color;
	const char* input_placeholder_color;
	const char* input_cursor_background;
} ui_styles_t;



typedef struct _UI_INPUT{
	unsigned int length;
	char data[PROTOCOL_MAX_INPUT_LENGTH];
} ui_input_t;



static const ui_styles_t _ui_styles[4]={
	{ // no input focus, connected
		"\x1b[48;2;30;31;25m",
		"\x1b[3m\x1b[38;2;155;155;155m",
		"\x1b[38;2;235;235;235m",
		"\x1b[1m\x1b[38;2;237;51;59m",
		"\x1b[38;2;155;155;155m",
		"\x1b[38;2;203;209;215m",
		"\x1b[48;2;36;37;33m",
		"\x1b[38;2;205;205;205m",
		"\x1b[38;2;100;100;100m",
		"\x1b[48;2;135;135;135m"
	},
	{ // input focus, connected
		"\x1b[48;2;30;31;25m",
		"\x1b[3m\x1b[38;2;155;155;155m",
		"\x1b[38;2;235;235;235m",
		"\x1b[1m\x1b[38;2;237;51;59m",
		"\x1b[38;2;155;155;155m",
		"\x1b[38;2;203;209;215m",
		"\x1b[48;2;66;67;63m",
		"\x1b[38;2;235;235;235m",
		"\x1b[38;2;130;130;130m",
		"\x1b[48;2;165;165;165m"
	},
	{ // no input focus, disconnected
		"\x1b[48;2;30;31;25m",
		"\x1b[3m\x1b[38;2;155;155;155m",
		"\x1b[38;2;235;235;235m",
		"\x1b[1m\x1b[38;2;237;51;59m",
		"\x1b[38;2;155;155;155m",
		"\x1b[38;2;203;209;215m",
		"\x1b[48;2;105;0;15m",
		"\x1b[38;2;205;205;205m",
		"\x1b[38;2;100;100;100m",
		"\x1b[48;2;135;135;135m"
	},
	{ // input focus, disconnected
		"\x1b[48;2;30;31;25m",
		"\x1b[3m\x1b[38;2;155;155;155m",
		"\x1b[38;2;235;235;235m",
		"\x1b[1m\x1b[38;2;237;51;59m",
		"\x1b[38;2;155;155;155m",
		"\x1b[38;2;203;209;215m",
		"\x1b[48;2;165;29;45m",
		"\x1b[38;2;235;235;235m",
		"\x1b[38;2;130;130;130m",
		"\x1b[48;2;165;165;165m"
	}
};
static const char* _ui_scroll_bar_parts[]={
	"\xe2\x96\x94",
	"\xf0\x9f\xae\x82",
	"\xf0\x9f\xae\x83",
	"\xe2\x96\x80",
	"\xf0\x9f\xae\x84",
	"\xf0\x9f\xae\x85",
	"\xf0\x9f\xae\x86",
	"\xe2\x96\x88",
	"\xe2\x96\x87",
	"\xe2\x96\x86",
	"\xe2\x96\x85",
	"\xe2\x96\x84",
	"\xe2\x96\x83",
	"\xe2\x96\x82",
	"\xe2\x96\x81"
};

static _Bool _ui_changes=0;
static _Bool _ui_was_disconnected=0;
static _Bool _ui_focus_on_input=1;
static ui_line_t _ui_lines[UI_MAX_LINES];
static unsigned int _ui_line_count=0;
static unsigned int _ui_line_scroll=0;
static ui_input_t _ui_history[UI_MAX_HISTORY_SIZE];
static unsigned int _ui_history_length=1;
static unsigned int _ui_history_index=0;
static unsigned int _ui_input_scroll=0;
static unsigned int _ui_input_cursor=0;
static unsigned int _ui_width;
static unsigned int _ui_height;



static void _ui_add_line(unsigned int flags,const char* data,unsigned int length){
	flags|=UI_LINE_FLAG_HEAD;
	do{
		if (_ui_line_count==UI_MAX_LINES){
			free(_ui_lines->data);
			for (unsigned int i=1;i<UI_MAX_LINES;i++){
				_ui_lines[i-1]=_ui_lines[i];
			}
			if (_ui_line_scroll&&_ui_line_scroll<UI_MAX_LINES-_ui_height){
				_ui_line_scroll--;
			}
		}
		else{
			_ui_line_count++;
			if (_ui_line_scroll==_ui_line_count-_ui_height){
				_ui_line_scroll++;
			}
		}
		ui_line_t* line=_ui_lines+_ui_line_count-1;
		line->flags=flags;
		unsigned int line_length=0;
		unsigned int space=_ui_width-3;
		unsigned int tab_extra_space=0;
		while (space&&line_length<length){
			unsigned char c=data[line_length];
			unsigned int char_length=1;
			if ((c>>5)==0b110){
				char_length=2;
			}
			else if ((c>>4)==0b1110){
				char_length=3;
			}
			else if ((c>>3)==0b11110){
				char_length=4;
			}
			else if (c=='\t'){
				tab_extra_space++;
			}
			line_length+=char_length;
			space--;
		}
		line->data=malloc(line_length+tab_extra_space+space+1);
		unsigned int j=0;
		for (unsigned int i=0;i<line_length;i++){
			if (data[i]=='\t'){
				line->data[j]=0xc2;
				j++;
				line->data[j]=0xb7;
			}
			else{
				line->data[j]=data[i];
			}
			j++;
		}
		data+=line_length;
		length-=line_length;
		line_length+=tab_extra_space;
		for (unsigned int i=0;i<space;i++){
			line->data[line_length]=' ';
			line_length++;
		}
		line->data[line_length]=0;
		flags&=~UI_LINE_FLAG_HEAD;
	} while (length);
}



static void _ensure_not_in_history_preview(void){
	if (_ui_history_index!=_ui_history_length-1){
		_ui_history[_ui_history_length-1]=_ui_history[_ui_history_index];
		_ui_history_index=_ui_history_length-1;
	}
}



void ui_init(void){
	_ui_changes=1;
	_ui_width=terminal_get_size(&_ui_height);
	printf("\x1b[?25l");
	for (unsigned int i=0;i<_ui_height-1;i++){
		putchar('\n');
	}
}



void ui_deinit(void){
	ui_clear_output();
	printf("\x1b[0m\x1b[?25h\x1b[%uA\x1b[0G\x1b[0J",_ui_height);
}



void ui_type_key(int key){
	switch (key){
		case UI_KEY_CHANGE_FOCUS:
			_ui_focus_on_input^=1;
			_ui_changes=1;
			break;
		case UI_KEY_BACKSPACE:
			if (_ui_focus_on_input&&_ui_input_cursor){
				_ensure_not_in_history_preview();
				if (_ui_input_cursor==_ui_input_scroll){
					_ui_input_scroll--;
				}
				_ui_input_cursor--;
				_ui_history[_ui_history_index].length--;
				for (unsigned int i=_ui_input_cursor;i<_ui_history[_ui_history_index].length;i++){
					_ui_history[_ui_history_index].data[i]=_ui_history[_ui_history_index].data[i+1];
				}
				_ui_changes=1;
			}
			break;
		case UI_KEY_UP:
			if (_ui_focus_on_input&&_ui_history_index){
				_ui_history_index--;
				_ui_input_cursor=_ui_history[_ui_history_index].length;
				_ui_changes=1;
			}
			else if (!_ui_focus_on_input&&_ui_line_scroll){
				_ui_line_scroll--;
				_ui_changes=1;
			}
			break;
		case UI_KEY_DOWN:
			if (_ui_focus_on_input&&_ui_history_index<_ui_history_length-1){
				_ui_history_index++;
				_ui_input_cursor=_ui_history[_ui_history_index].length;
				_ui_changes=1;
			}
			else if (!_ui_focus_on_input&&_ui_line_count>_ui_height&&_ui_line_scroll<=_ui_line_count-_ui_height){
				_ui_line_scroll++;
				_ui_changes=1;
			}
			break;
		case UI_KEY_RIGHT:
			if (_ui_focus_on_input&&_ui_input_cursor<_ui_history[_ui_history_index].length){
				_ui_input_cursor++;
				if (_ui_input_cursor==_ui_input_scroll+_ui_width){
					_ui_input_scroll++;
				}
				_ui_changes=1;
			}
			break;
		case UI_KEY_LEFT:
			if (_ui_focus_on_input&&_ui_input_cursor){
				if (_ui_input_cursor==_ui_input_scroll){
					_ui_input_scroll--;
				}
				_ui_input_cursor--;
				_ui_changes=1;
			}
			break;
		case UI_KEY_END:
			if (_ui_focus_on_input&&_ui_input_cursor<_ui_history[_ui_history_index].length){
				_ui_input_cursor=_ui_history[_ui_history_index].length;
				_ui_input_scroll=(_ui_input_cursor>=_ui_width?_ui_input_cursor-_ui_width+1:0);
				_ui_changes=1;
			}
			else if (!_ui_focus_on_input&&_ui_line_scroll<_ui_line_count-_ui_height){
				_ui_line_scroll=_ui_line_count-_ui_height+1;
				_ui_changes=1;
			}
			break;
		case UI_KEY_HOME:
			if (_ui_focus_on_input&&_ui_input_cursor){
				_ui_input_cursor=0;
				_ui_input_scroll=0;
				_ui_changes=1;
			}
			else if (!_ui_focus_on_input&&_ui_line_scroll){
				_ui_line_scroll=0;
				_ui_changes=1;
			}
			break;
		case UI_KEY_DELETE:
			if (_ui_focus_on_input&&_ui_input_cursor<_ui_history[_ui_history_index].length){
				_ensure_not_in_history_preview();
				_ui_history[_ui_history_index].length--;
				for (unsigned int i=_ui_input_cursor;i<_ui_history[_ui_history_index].length;i++){
					_ui_history[_ui_history_index].data[i]=_ui_history[_ui_history_index].data[i+1];
				}
				_ui_changes=1;
			}
			break;
		default:
			if (_ui_focus_on_input&&_ui_history[_ui_history_index].length<PROTOCOL_MAX_INPUT_LENGTH){
				_ensure_not_in_history_preview();
				_ui_history[_ui_history_index].length++;
				for (unsigned int i=_ui_history[_ui_history_index].length;i>_ui_input_cursor;i--){
					_ui_history[_ui_history_index].data[i]=_ui_history[_ui_history_index].data[i-1];
				}
				_ui_history[_ui_history_index].data[_ui_input_cursor]=key;
				_ui_input_cursor++;
				if (_ui_input_cursor==_ui_input_scroll+_ui_width){
					_ui_input_scroll++;
				}
				_ui_changes=1;
			}
			break;
	}
}



const char* ui_get_input(unsigned int* length){
	*length=_ui_history[_ui_history_index].length;
	return _ui_history[_ui_history_index].data;
}



void ui_clear_input(void){
	_ui_add_line(UI_LINE_FLAG_INPUT,_ui_history[_ui_history_index].data,_ui_history[_ui_history_index].length);
	_ui_input_cursor=0;
	_ui_input_scroll=0;
	_ensure_not_in_history_preview();
	if (!_ui_history_index||_ui_history[_ui_history_index-1].length!=_ui_history[_ui_history_index].length||strcmp(_ui_history[_ui_history_index-1].data,_ui_history[_ui_history_index].data)){
		if (_ui_history_length<UI_MAX_HISTORY_SIZE-1){
			_ui_history_index++;
			_ui_history_length++;
		}
		else{
			for (unsigned int i=0;i<UI_MAX_HISTORY_SIZE-1;i++){
				_ui_history[i]=_ui_history[i+1];
			}
		}
	}
	_ui_history[_ui_history_index].length=0;
	_ui_changes=1;
}



void ui_clear_output(void){
	while (_ui_line_count){
		_ui_line_count--;
		free((_ui_lines+_ui_line_count)->data);
	}
	_ui_line_scroll=0;
	_ui_changes=1;
}



void ui_add_output(const char* data,unsigned int length,_Bool error){
	_ui_add_line((error?UI_LINE_FLAG_ERROR:0),data,length);
	_ui_changes=1;
}



void ui_redraw(void){
	_ui_width=terminal_get_size(&_ui_height);
	_Bool disconnected=(device_get_fd()==-1);
	if (_ui_was_disconnected!=disconnected){
		_ui_changes=1;
		_ui_was_disconnected=disconnected;
	}
	if (!_ui_changes){
		return;
	}
	_ui_changes=0;
	const ui_styles_t* styles=_ui_styles+_ui_focus_on_input+(disconnected<<1);
	printf("\x1b[%uA\x1b[0G%s",_ui_height,styles->console_background);
	unsigned int vertical_scroll_pixel_index=(_ui_line_count<=_ui_height-1?0:_ui_line_scroll*(_ui_height-2)*8/(_ui_line_count-_ui_height+1));
	_Bool print_head=1;
	for (unsigned int i=0;i<_ui_height-1;i++){
		unsigned int line_index=i+_ui_line_scroll;
		if (line_index>=_ui_line_count){
			for (unsigned int j=0;j<_ui_width-1;j++){
				putchar(' ');
			}
		}
		else{
			printf("%s",styles->console_arrow_color);
			if (print_head||(_ui_lines[line_index].flags&UI_LINE_FLAG_HEAD)){
				printf("%s",((_ui_lines[line_index].flags&UI_LINE_FLAG_INPUT)?"«":"»"));
				print_head=0;
			}
			else{
				putchar(' ');
			}
			printf(" %s%s\x1b[22m\x1b[23m",((_ui_lines[line_index].flags&UI_LINE_FLAG_INPUT)?styles->console_input_color:((_ui_lines[line_index].flags&UI_LINE_FLAG_ERROR)?styles->console_error_color:styles->console_output_color)),_ui_lines[line_index].data);
		}
		printf("%s",styles->console_scroll_bar);
		signed int diff=vertical_scroll_pixel_index-i*8;
		if (diff<=-8||diff>=8){
			putchar(' ');
		}
		else{
			printf("%s",_ui_scroll_bar_parts[diff+7]);
		}
		putchar('\n');
	}
	printf("%s",styles->input_background);
	if (!_ui_history[_ui_history_index].length){
		printf("%s<input>",styles->input_placeholder_color);
		for (unsigned int i=7;i<_ui_width;i++){
			putchar(' ');
		}
	}
	else{
		printf("%s",styles->input_color);
		for (unsigned int i=0;i<_ui_width;i++){
			if (i+_ui_input_scroll==_ui_input_cursor){
				printf("%s",styles->input_cursor_background);
			}
			if (i+_ui_input_scroll<_ui_history[_ui_history_index].length){
				if (_ui_history[_ui_history_index].data[i+_ui_input_scroll]=='\t'){
					printf("·");
				}
				else{
					putchar(_ui_history[_ui_history_index].data[i+_ui_input_scroll]);
				}
			}
			else{
				putchar(' ');
			}
			if (i+_ui_input_scroll==_ui_input_cursor){
				printf("%s",styles->input_background);
			}
		}
	}
	fflush(stdout);
}
