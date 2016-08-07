all: joytrack hidtrack debug test_builder

joytrack : joytrack.c ThreeAxes.h
	gcc -o joytrack -g -I. joytrack.c linuxtrack.c -framework IOKit

hidtrack : hidtrack.c hid_builder.c hid_builder.h
	gcc -o hidtrack -g -I. hidtrack.c linuxtrack.c hid_builder.c -framework IOKit

test_builder : test_builder.c hid_builder.c hid_builder.h
	gcc -o test_builder -g -I. test_builder.c hid_builder.c -framework IOKit

debug : linuxtrack_hello_world.c
	gcc -o debug -g -I. linuxtrack_hello_world.c linuxtrack.c

clean:
	rm joytrack debug hidtrack
