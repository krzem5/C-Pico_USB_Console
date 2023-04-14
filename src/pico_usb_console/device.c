#include <device.h>
#include <errno.h>
#include <fcntl.h>
#include <libudev.h>
#include <protocol.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>



static struct udev* _udev_ctx;
static int _device_usb_device_handle=-1;



void device_init(void){
	_udev_ctx=udev_new();
}



void device_deinit(void){
	if (_device_usb_device_handle!=-1){
		close(_device_usb_device_handle);
		_device_usb_device_handle=-1;
	}
	udev_unref(_udev_ctx);
}



int device_get_fd(void){
	return _device_usb_device_handle;
}



void device_find_if_not_connected(void){
	if (_device_usb_device_handle!=-1){
		return;
	}
	struct udev_enumerate* dev_list=udev_enumerate_new(_udev_ctx);
	udev_enumerate_add_match_subsystem(dev_list,"tty");
	udev_enumerate_scan_devices(dev_list);
	struct udev_list_entry* entry=udev_enumerate_get_list_entry(dev_list);
	while (entry){
		struct udev_device* dev=udev_device_new_from_syspath(_udev_ctx,udev_list_entry_get_name(entry));
		const char* vid=udev_device_get_property_value(dev,"ID_VENDOR_ID");
		const char* pid=udev_device_get_property_value(dev,"ID_MODEL_ID");
		if (vid&&pid&&!strcmp(vid,DEVICE_REQUIRED_VID)&&!strcmp(pid,DEVICE_REQUIRED_PID)){
			_device_usb_device_handle=open(udev_device_get_devnode(dev),O_RDWR|O_NOCTTY|O_SYNC|O_NONBLOCK);
			udev_device_unref(dev);
			udev_enumerate_unref(dev_list);
			if (_device_usb_device_handle==-1){
				return;
			}
			struct termios config;
			if (tcgetattr(_device_usb_device_handle,&config)){
				close(_device_usb_device_handle);
				_device_usb_device_handle=-1;
				return;
			}
			config.c_iflag&=~(IGNBRK|BRKINT|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF|IXANY|INPCK);
			config.c_iflag|=PARMRK|IGNPAR;
			config.c_cflag&=~(CBAUD|CSIZE|PARENB|PARODD|CSTOPB|CRTSCTS);
			config.c_cflag|=CLOCAL|CREAD|B115200|CS8;
			config.c_lflag=0;
			config.c_oflag=0;
			config.c_cc[VMIN]=0;
			config.c_cc[VTIME]=5;
			if (tcsetattr(_device_usb_device_handle,TCSANOW,&config)){
				close(_device_usb_device_handle);
				_device_usb_device_handle=-1;
			}
			return;
		}
		udev_device_unref(dev);
		entry=udev_list_entry_get_next(entry);
	}
	udev_enumerate_unref(dev_list);
}



void device_close(void){
	if (_device_usb_device_handle!=-1){
		close(_device_usb_device_handle);
		_device_usb_device_handle=-1;
	}
}



void device_read_data(void){
	if (_device_usb_device_handle==-1){
		return;
	}
	int count;
	unsigned char buffer[32];
	do{
		count=read(_device_usb_device_handle,buffer,32);
		if (count==-1){
			if (errno==ENODEV){
				close(_device_usb_device_handle);
				_device_usb_device_handle=-1;
			}
			return;
		}
		protocol_process_data(buffer,count);
	} while (count);
}



void device_write_data(const unsigned char* data,unsigned int length){
	if (_device_usb_device_handle==-1){
		return;
	}
	if (write(_device_usb_device_handle,data,length)<length){
		close(_device_usb_device_handle);
		_device_usb_device_handle=-1;
	}
}
