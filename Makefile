all: hidtrack test_builder

hidtrack : src/hidtrack.c src/hid_builder.c src/hid_builder.h
	gcc -o hidtrack -g -Isrc/. src/hidtrack.c src/linuxtrack.c src/hid_builder.c -framework IOKit

test_builder : src/test_builder.c src/hid_builder.c src/hid_builder.h
	gcc -o test_builder -g -I. src/test_builder.c src/hid_builder.c -framework IOKit

clean:
	rm -rf hidtrack test_builder
	rm -fr *.dSYM
