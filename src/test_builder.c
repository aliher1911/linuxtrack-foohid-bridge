#include "hid_builder.h"
#include <stdio.h>
#include <stddef.h>
#include "ThreeAxes.h"
#include "VRControls2.h"

#define BUF_SIZE (1024)

void testJoystickDescriptor() {
	printf("Test joystick descriptor.\n");
	char buffer[BUF_SIZE];

	size_t ptr = 0;

	ptr = create_preamble(JOYSTICK, buffer, ptr, BUF_SIZE);
	ptr = add_axis(X, -127, 127, buffer, ptr, BUF_SIZE);
	ptr = add_axis(Y, -79, 79, buffer, ptr, BUF_SIZE);
	ptr = add_axis(Z, -45, 45, buffer, ptr, BUF_SIZE);
	ptr = create_closure(buffer, ptr, BUF_SIZE);

    printf("Size = %lu, Expected = %lu\n", ptr, sizeof(ReportDescriptor));
    for(size_t i = 0; i < ptr; i++) {
    	if (buffer[i] != ReportDescriptor[i]) {
    		printf("Mismatch at line %lu. Got = %02x, Expected = %02x\n", i, (int)buffer[i], (int)ReportDescriptor[i]);
    	}
    }
}

void testVrDescriptor() {
	printf("Test vr descriptor.\n");
	char buffer[BUF_SIZE];

	size_t ptr = 0;

	ptr = create_preamble(VR, buffer, ptr, BUF_SIZE);
	ptr = add_axis(RX, -127, 127, buffer, ptr, BUF_SIZE);
	ptr = add_axis(RY, -79, 79, buffer, ptr, BUF_SIZE);
	ptr = add_axis(RZ, -45, 45, buffer, ptr, BUF_SIZE);
	ptr = add_axis(X, 0, 300, buffer, ptr, BUF_SIZE);
	ptr = add_axis(Y, 0, 200, buffer, ptr, BUF_SIZE);
	ptr = add_axis(Z, 0, 100, buffer, ptr, BUF_SIZE);
	ptr = create_closure(buffer, ptr, BUF_SIZE);

    printf("Size = %lu, Expected = %lu\n", ptr, sizeof(ReportDescriptor2));
    for(size_t i = 0; i < ptr && i < sizeof(ReportDescriptor2); i++) {
    	if (buffer[i] != ReportDescriptor2[i]) {
    		printf("Mismatch at line %lu. Got = %02x, Expected = %02x\n", 
    			   i, (unsigned char)buffer[i], (unsigned char)ReportDescriptor[i]);
    	}
    }
}

void testBufferSizeCheck() {
	printf("Test buffer size check\n");
	char buffer[BUF_SIZE];
	size_t ptr;
	ptr = create_preamble(JOYSTICK, buffer, -1, BUF_SIZE);
	printf("Create: Expected %li, got %li\n", -1L, ptr);
	ptr = add_axis(X, -127, 127, buffer, -1, BUF_SIZE);
	printf("Add: Expected %li, got %li\n", -1L, ptr);
	ptr = create_closure(buffer, -1, BUF_SIZE);	
	printf("Close: Expected %li, got %li\n", -1L, ptr);
}

void testCreateDevice() {
	printf("Test create device.\n");
	char buffer[BUF_SIZE];
	size_t ptr = 0;
	ptr = create_preamble(JOYSTICK, buffer, ptr, BUF_SIZE);
	ptr = add_axis(RX, -127, 127, buffer, ptr, BUF_SIZE);
	ptr = add_axis(RY, -79, 79, buffer, ptr, BUF_SIZE);
	ptr = add_axis(RZ, -45, 45, buffer, ptr, BUF_SIZE);
	ptr = create_closure(buffer, ptr, BUF_SIZE);

	// handle to open device
	io_connect_t connect;
	if (init_device(buffer, ptr, &connect)!=0) {
		printf("Failed to create device: %s", hid_device_err_str());
	}
}

void testDestroyDevice() {
	printf("Test destroy device.\n");
	char buffer[BUF_SIZE];
	size_t ptr = 0;
	ptr = create_preamble(JOYSTICK, buffer, ptr, BUF_SIZE);
	ptr = add_axis(X, -127, 127, buffer, ptr, BUF_SIZE);
	ptr = add_axis(Y, -79, 79, buffer, ptr, BUF_SIZE);
	ptr = add_axis(Z, -45, 45, buffer, ptr, BUF_SIZE);
	ptr = create_closure(buffer, ptr, BUF_SIZE);

	// handle to open device
	io_connect_t connect;
	if (init_device(buffer, ptr, &connect)!=0) {
		printf("Failed to create device: %s", hid_device_err_str());
		return;
	}
	if (destroy_device(connect)!=0) {
		printf("Failed to create device: %s", hid_device_err_str());
	}
}

void testSendData() {
	printf("Test send data.\n");
	char buffer[BUF_SIZE];
	size_t ptr = 0;
	ptr = create_preamble(JOYSTICK, buffer, ptr, BUF_SIZE);
	ptr = add_axis(X, -127, 127, buffer, ptr, BUF_SIZE);
	ptr = add_axis(Y, -79, 79, buffer, ptr, BUF_SIZE);
	ptr = add_axis(Z, -45, 45, buffer, ptr, BUF_SIZE);
	ptr = add_axis(RX, -300, 300, buffer, ptr, BUF_SIZE);
	ptr = add_axis(RY, -200, 200, buffer, ptr, BUF_SIZE);
	ptr = add_axis(RZ, 0, 100, buffer, ptr, BUF_SIZE);
	ptr = create_closure(buffer, ptr, BUF_SIZE);

	// handle to open device
	io_connect_t connect;
	if (init_device(buffer, ptr, &connect)!=0) {
		printf("Failed to create device: %s", hid_device_err_str());
	}
	device_report report;
	report.x = 0;
	report.y = 0;
	report.z = 0;
	report.rx = 0;
	report.ry = 0;
	report.rz = 0;
	// this would become maddness since they would drift our of axis ranges
	for(int i=0; i<1000; i++) {
		report.x += 1;
		report.y += 2;
		report.z += 3;
		report.rx -= 1;
		report.ry -= 2;
		report.rz -= 3;
		send_data(connect, &report);
        usleep(1000000);  // sleep for a second
	}
}

int main() {
	// testJoystickDescriptor();
	// testVrDescriptor();
	// testBufferSizeCheck();
	// testCreateDevice();
	// testDestroyDevice();
	// testSendData();
}
