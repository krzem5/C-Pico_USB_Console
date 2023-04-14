#ifndef _UI_H_
#define _UI_H_ 1



#define UI_MAX_LINES 1024
#define UI_MAX_HISTORY_SIZE 256

#define UI_KEY_CHANGE_FOCUS 256
#define UI_KEY_BACKSPACE 257
#define UI_KEY_UP 258
#define UI_KEY_DOWN 259
#define UI_KEY_RIGHT 260
#define UI_KEY_LEFT 261
#define UI_KEY_END 262
#define UI_KEY_HOME 263
#define UI_KEY_DELETE 264



void ui_init(void);



void ui_deinit(void);



void ui_type_key(int key);



const char* ui_get_input(unsigned int* length);



void ui_clear_input(void);



void ui_clear_output(void);



void ui_add_output(const char* data,unsigned int length,_Bool error);



void ui_redraw(void);



#endif
