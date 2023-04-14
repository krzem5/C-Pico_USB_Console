#include <device.h>
#include <protocol.h>
#include <ui.h>
#include <stdio.h>



static unsigned char _protocol_buffer[PROTOCOL_INTERNAL_BUFFER_SIZE];
static unsigned int _protocol_buffer_start=0;
static unsigned int _protocol_buffer_end=0;
static unsigned int _protocol_buffer_length=0;

static const unsigned int _protocol_packet_type_to_length[256]={
	[PROTOCOL_PACKET_TYPE_LOG]=-1
};



void protocol_process_data(const unsigned char* data,unsigned int length){
	if (!length){
		return;
	}
	do{
		while (length&&_protocol_buffer_length<PROTOCOL_INTERNAL_BUFFER_SIZE){
			_protocol_buffer[_protocol_buffer_end]=*data;
			length--;
			data++;
			_protocol_buffer_end=(_protocol_buffer_end+1)&(PROTOCOL_INTERNAL_BUFFER_SIZE-1);
			_protocol_buffer_length++;
		}
		if (_protocol_buffer[_protocol_buffer_start]!=0xff){
_skip_first_byte:
			_protocol_buffer_start=(_protocol_buffer_start+1)&(PROTOCOL_INTERNAL_BUFFER_SIZE-1);
			_protocol_buffer_length--;
			continue;
		}
		if (_protocol_buffer_length<1){
_not_enough_data:
			if (length){
				goto _next_data_chunk;
			}
			return;
		}
		unsigned char type=_protocol_buffer[(_protocol_buffer_start+1)&(PROTOCOL_INTERNAL_BUFFER_SIZE-1)];
		if (type==0xff){
			goto _skip_first_byte;
		}
		unsigned int required_length=_protocol_packet_type_to_length[type];
		if (required_length==-1){
			required_length=_protocol_buffer[(_protocol_buffer_start+2)&(PROTOCOL_INTERNAL_BUFFER_SIZE-1)];
		}
		if (_protocol_buffer_length<required_length){
			goto _not_enough_data;
		}
		switch (type){
			case PROTOCOL_PACKET_TYPE_LOG:
				{
					char buffer[256];
					for (unsigned int i=4;i<required_length;i++){
						buffer[i-4]=_protocol_buffer[(_protocol_buffer_start+i)&(PROTOCOL_INTERNAL_BUFFER_SIZE-1)];
					}
					ui_add_output(buffer,required_length-4,!!_protocol_buffer[(_protocol_buffer_start+3)&(PROTOCOL_INTERNAL_BUFFER_SIZE-1)]);
					break;
				}
		}
		_protocol_buffer_length-=required_length;
		_protocol_buffer_start=(_protocol_buffer_start+required_length)&(PROTOCOL_INTERNAL_BUFFER_SIZE-1);
_next_data_chunk:
	} while (_protocol_buffer_length);
}



void protocol_send_input(const char* input,unsigned int length){
	unsigned char buffer[PROTOCOL_MAX_INPUT_LENGTH+3]={0xff,PROTOCOL_PACKET_TYPE_INPUT,length+3};
	for (unsigned int i=0;i<length;i++){
		buffer[i+3]=input[i];
	}
	device_write_data(buffer,length+3);
}



void protcol_reset_server(void){
	unsigned char buffer[]={0xff,PROTOCOL_PACKET_TYPE_RESET_SERVER};
	device_write_data(buffer,2);
}
