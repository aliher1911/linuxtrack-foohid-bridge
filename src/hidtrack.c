/**
 * Create a virtual joystick controlled by Linuxtrack
 * Compile me with: gcc -o hidtrack -g -I. hidtrack.c linuxtrack.c hid_builder.c -framework IOKit
 */

#include <IOKit/IOKitLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <linuxtrack.h>
#include <hid_builder.h>

bool intialize_tracking(void)
{
  linuxtrack_state_type state;
  //Initialize the tracking using Default profile
  state = linuxtrack_init(NULL);
  if(state < LINUXTRACK_OK){
    printf("%s\n", linuxtrack_explain(state));
    return false;
  }
  int timeout = 0;
  //wait up to 20 seconds for the tracker initialization
  while(timeout < 200){
    state = linuxtrack_get_tracking_state();
    printf("Status: %s\n", linuxtrack_explain(state));
    if((state == RUNNING) || (state == PAUSED)){
      return true;
    }
    usleep(100000);
    ++timeout;
  }
  return false;
}

static uint8_t running;

void stop_loop(int signal) {
  running = 0;
  printf("Interrupted by user.\n");
}

// coordinate limits should match linuxtrack profile
#define MIN_X  (-300)
#define MAX_X   (300)
#define MIN_Y  (-300)
#define MAX_Y   (300)
#define MIN_Z     (0)
#define MAX_Z   (300)
#define MIN_RX  (-45)
#define MAX_RX   (45)
#define MIN_RY  (-80)
#define MAX_RY   (80)
#define MIN_RZ (-130)
#define MAX_RZ  (130)

#define BUF_SIZE (1024)

#define CLIP(v, min, max) ((v < min) ? min : ((v > max) ? max : v))

int main(int argc, char **argv) {
    printf("LinuxTrack to Foo HID translator.\n");
    control_type control = JOYSTICK;
    if (argc>1) {
        if (strcmp("-j", argv[1])==0) {
            printf("Using Joystick Device\n");
            control = JOYSTICK;
        }
        if (strcmp("-v", argv[1])==0) {
            printf("Using VR Device\n");
            control = VR;
        }
    }

    io_connect_t connect;
    char buffer[BUF_SIZE];
    size_t ptr = 0;
    ptr = create_preamble(control, buffer, ptr, BUF_SIZE);
    ptr = add_axis(X, MIN_X, MAX_X, buffer, ptr, BUF_SIZE);
    ptr = add_axis(Y, MIN_Y, MAX_Y, buffer, ptr, BUF_SIZE);
    ptr = add_axis(Z, MIN_Z, MAX_Z, buffer, ptr, BUF_SIZE);
    ptr = add_axis(RX, MIN_RX, MAX_RX, buffer, ptr, BUF_SIZE);
    ptr = add_axis(RY, MIN_RY, MAX_RY, buffer, ptr, BUF_SIZE);
    ptr = add_axis(RZ, MIN_RZ, MAX_RZ, buffer, ptr, BUF_SIZE);
    ptr = create_closure(buffer, ptr, BUF_SIZE);

    if (init_device(buffer, ptr, &connect)!=0) {
        printf("Failed to create device: %s", hid_device_err_str());
        exit(-1);
    }

    if (!intialize_tracking()) {
        printf("Linuxtrack doesn't work right!\n");
        printf("Make sure it is installed and configured correctly.\n");
        exit(2);
    }

    running = 1;  
    signal(SIGINT, &stop_loop);

    device_report device;

    device.x = 0;
    device.y = 0;
    device.z = 0;
    device.rx = 0;
    device.ry = 0;
    device.rz = 0;

    float heading, pitch, roll, x, y, z;
    unsigned int counter;

    while(running){
        if (linuxtrack_get_pose(&heading, &pitch, &roll, &x, &y, &z, &counter) > 0) {
            device.rx = CLIP(roll,    MIN_RZ, MAX_RX);
            device.ry = CLIP(pitch,   MIN_RY, MAX_RY);
            device.rz = CLIP(heading, MIN_RZ, MAX_RZ);
            device.x  = CLIP(x,       MIN_X,  MAX_X );
            device.y  = CLIP(y,       MIN_Y,  MAX_Y );
            device.z  = CLIP(z,       MIN_Z,  MAX_Z );
            if (send_data(connect, &device)!=0) {
                printf("Failed to send data: %s", hid_device_err_str());
            } else {
                printf("h: %7.2f  p: %7.2f  y: %7.2f\n", heading, pitch, roll);
            }
            usleep(100000);  // sleep for 1/10 a second
        }
    }

    linuxtrack_shutdown();
    destroy_device(connect);
    printf("Finished.");
    exit(0);
}
