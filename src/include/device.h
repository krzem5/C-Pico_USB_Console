#ifndef _DEVICE_H_
#define _DEVICE_H_ 1



#define DEVICE_REQUIRED_VID "fff0"
#define DEVICE_REQUIRED_PID "0001"



void device_init(void);



void device_deinit(void);



int device_get_fd(void);



void device_find_if_not_connected(void);



void device_close(void);



void device_read_data(void);



void device_write_data(const unsigned char* data,unsigned int length);



#endif
