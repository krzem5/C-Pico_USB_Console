#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_ 1



#define PROTOCOL_INTERNAL_BUFFER_SIZE 256

#define PROTOCOL_MAX_INPUT_LENGTH 252

#define PROTOCOL_LOG_FLAG_DEBUG 1
#define PROTOCOL_LOG_FLAG_ERROR 2

#define PROTOCOL_PACKET_TYPE_LOG 0x00
#define PROTOCOL_PACKET_TYPE_INPUT 0x01
#define PROTOCOL_PACKET_TYPE_RESET_SERVER 0xff



void protocol_process_data(const unsigned char* data,unsigned int length);



void protocol_send_input(const char* input,unsigned int length);



void protcol_reset_server(void);



#endif
