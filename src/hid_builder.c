#include "hid_builder.h"
#include <string.h>
#include <stdlib.h>

#define GENERIC_DESKTOP_PAGE (0x01)
#define VR_CONTROLS_PAGE     (0x03)

char joystick_header[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE (Game Pad)
    0xa1, 0x01,                    // COLLECTION (Application)
    0xa1, 0x00,                    //   COLLECTION (Physical)
};

char vr_header[] = {
    0x05, 0x03,                    // USAGE_PAGE (VR Controls)
    0x09, 0x05,                    // USAGE (Head Tracker)
    0xa1, 0x00,                    // COLLECTION (Physical)
};

size_t create_preamble(control_type type, char buffer[], size_t pos, size_t len) {
	if (pos == -1) {
		return -1;
	}
	char *header = NULL;
	size_t header_len = 0;
	switch(type) {
		case JOYSTICK:
			header = joystick_header;
			header_len = sizeof(joystick_header);
			break;
		case VR:
			header = vr_header;
			header_len = sizeof(vr_header);
			break;
	}
	if (pos + header_len > len) {
		return -1;
	}
	memcpy(buffer + pos, header, header_len);
    return pos + header_len;
}

char joystick_closure[] = {
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION	
};

char vr_closure[] = {
    0xc0                           // END_COLLECTION	
};

size_t create_closure(char buffer[], size_t pos, size_t len) {
	if (pos == -1) {
		return -1;
	}
	char *closure = NULL;
	size_t closure_len = 0;
	switch(buffer[1]) {
		case GENERIC_DESKTOP_PAGE:
			closure = joystick_closure;
			closure_len = sizeof(joystick_closure);
			break;
		case VR_CONTROLS_PAGE:
			closure = vr_closure;
			closure_len = sizeof(vr_closure);
			break;
		default:
			return -1;
	}
	if (pos + closure_len > len) {
		return -1;
	}
	memcpy(buffer + pos, closure, closure_len);
    return pos + closure_len;
}

char axis_template1[] = {
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x33,                    //   USAGE (Rx)
    0x16, 0x4c, 0xff,              //   LOGICAL_MINIMUM (-180)
    0x26, 0xb4, 0x00,              //   LOGICAL_MAXIMUM (180)
    0x75, 0x10,                    //   REPORT_SIZE (16)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
};

char axis_template2[] = {
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
};

char axis_header[] = {
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
};

char axis_footer[] = {
    0x75, 0x10,                    //   REPORT_SIZE (16)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
};

#define LOG_MIN_8   (0x15)
#define LOG_MIN_16  (0x16)
#define LOG_MAX_8   (0x25)
#define LOG_MAX_16  (0x26)
#define AXIS_OFFSET (3)

#define fit_in_8(x) (x >= -128 && x < 128)
#define range_descriptor_size(x) ((x >= -128 && x < 128) ? 2 : 3)

size_t add_limit(int16_t value, char code_8, char code_16, char buffer[], size_t pos) {
	if (fit_in_8(value)) {
		buffer[pos] = code_8;
		memcpy(buffer + pos + 1, &value, 1);
		return 2;
	} else {
		buffer[pos] = code_16;
		memcpy(buffer + pos + 1, &value, 2);
		return 3;
	}
}

size_t add_axis(axis_type type, int16_t minimum, int16_t maximum, char buffer[], size_t pos, size_t len) {
	if (pos == -1) {
		return -1;
	}
	size_t total_size = sizeof(axis_header) + sizeof(axis_footer) 
						+ range_descriptor_size(minimum) + range_descriptor_size(maximum);
	if (pos + total_size > len) {
		return -1;
	}
	// add header
	memcpy(buffer + pos, axis_header, sizeof(axis_header));
	buffer[pos + AXIS_OFFSET] = type;
	pos += sizeof(axis_header);
	// add min
	pos += add_limit(minimum, LOG_MIN_8, LOG_MIN_16, buffer, pos);
	// add max
	pos += add_limit(maximum, LOG_MAX_8, LOG_MAX_16, buffer, pos);
	// add footer
	memcpy(buffer + pos, axis_footer, sizeof(axis_footer));
    return pos + sizeof(axis_footer);
}

const char* error_no_kernel_driver = "Unable to access IOService.\n";
const char* error_cant_create_hid_device = "Unable to create HID device.\n";
const char* error_cant_destroy_hid_device = "Unable to destroy HID device.\n";
const char* error_cant_send_hid_data = "Unable to send data to HID device.\n";
const char* all_ok = "No error.\n";

const char* hid_error;

const char* hid_device_err_str() {
	return hid_error;
}

#define SERVICE_NAME "it_unbit_foohid"

#define FOOHID_CREATE  (0) // create selector
#define FOOHID_DESTROY (1) // remove selector
#define FOOHID_SEND    (2) // send selector

#define DEVICE_NAME "LinuxTrack Virtual Control"
#define DEVICE_SN "SN 1911"

int init_device(char buffer[], size_t len, io_connect_t *connect) {
	hid_error = all_ok;

    io_iterator_t iterator;
    io_service_t service;

    // Get a reference to the IOService
    kern_return_t ret = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(SERVICE_NAME), &iterator);

    if (ret != KERN_SUCCESS) {
    	hid_error = error_no_kernel_driver;
        return -1;
    }

    // Iterate till success
    int found = 0;
    while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
        ret = IOServiceOpen(service, mach_task_self(), 0, connect);

        if (ret == KERN_SUCCESS) {
            found = 1;
            break;
        }

        IOObjectRelease(service);
    }
    IOObjectRelease(iterator);

    if (!found) {
    	hid_error = error_no_kernel_driver;
        return -1;
    }

    // Fill up the input arguments.
    uint32_t input_count = 8;
    uint64_t input[input_count];
    input[0] = (uint64_t) strdup(DEVICE_NAME);  // device name
    input[1] = strlen((char *)input[0]);  // name length
    input[2] = (uint64_t) buffer;  // report descriptor
    input[3] = len;  // report descriptor len
    input[4] = (uint64_t) strdup(DEVICE_SN);  // serial number
    input[5] = strlen((char *)input[4]);  // serial number len
    input[6] = (uint64_t) 2;  // vendor ID
    input[7] = (uint64_t) 3;  // device ID

    // attempt to create new device
    ret = IOConnectCallScalarMethod(*connect, FOOHID_CREATE, input, input_count, NULL, 0);
    if (ret != KERN_SUCCESS) {
    	// if we can't there's a chance previous device still exists
    	if (destroy_device(*connect)!=0) {
    		// we can't destory, so it wasn't there or there's a system problem
    		hid_error = error_cant_create_hid_device;
			free((void *)input[0]);
			free((void *)input[4]);
    		return -1;
    	}
    	// try to create new device
	    ret = IOConnectCallScalarMethod(*connect, FOOHID_CREATE, input, input_count, NULL, 0);
	    if (ret != KERN_SUCCESS) {
	    	// we destroyed previous, but can't create new. give up.
    		hid_error = error_cant_create_hid_device;
			free((void *)input[0]);
			free((void *)input[4]);
    		return -1;
	    }
    }
	free((void *)input[0]);
	free((void *)input[4]);
    return 0;
}

int send_data(io_connect_t connect, device_report *location) {
    uint32_t send_count = 4;
    uint64_t send[send_count];
    send[0] = (uint64_t) strdup(DEVICE_NAME);  // device name
    send[1] = strlen((char *)send[0]);  // name length
    send[2] = (uint64_t) location;  // hid struct
    send[3] = sizeof(device_report);  // hid struct len
    kern_return_t ret = IOConnectCallScalarMethod(connect, FOOHID_SEND, send, send_count, NULL, 0);
	free((void *)send[0]);
    if (ret != KERN_SUCCESS) {
    	hid_error = error_cant_send_hid_data;
    	return -1;
    }
    return 0;
}

int destroy_device(io_connect_t connect) {
    uint32_t input_count = 2;
    uint64_t input[input_count];
    input[0] = (uint64_t) strdup(DEVICE_NAME);  // device name
    input[1] = strlen((char *)input[0]);  // name length
    kern_return_t ret = IOConnectCallScalarMethod(connect, FOOHID_DESTROY, input, input_count, NULL, 0);
    free((void *)input[0]);
    if (ret != KERN_SUCCESS) {
    	hid_error = error_cant_destroy_hid_device;
        return -1;
    }
    return 0;
}
