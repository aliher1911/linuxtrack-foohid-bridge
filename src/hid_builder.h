#ifndef HID_BUILDER_H_INCLUDED
#define HID_BUILDER_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <IOKit/IOKitLib.h>

typedef enum control_type_t {
    JOYSTICK, VR
} control_type;

typedef enum axis_type_t {
    X = 0x30, Y = 0x31, Z = 0x32, RX = 0x33, RY = 0x34, RZ = 0x35
} axis_type;

typedef struct device_report_t {
    int16_t x;
    int16_t y;
    int16_t z;
 
    uint16_t rx;
    uint16_t ry;
    uint16_t rz;
} device_report;

// append beginning of hid structure to buffer
size_t create_preamble(control_type type, char buffer[], size_t pos, size_t len);
// append axis to hid descriptor
size_t add_axis(axis_type type, int16_t minimum, int16_t maximum, char buffer[], size_t pos, size_t len);
// add closing block to hid structure
size_t create_closure(char buffer[], size_t pos, size_t len);

// finds kernel driver and (re)creates hid device using provided description
int init_device(char hid_descriptor[], size_t len, io_connect_t *connect);
// send location data to kernel driver
int send_data(io_connect_t connect, device_report *location);
// destroys hid device
int destroy_device(io_connect_t connect);

// message of last error if kernel operations fail
const char* hid_device_err_str();

#endif
