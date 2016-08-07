# linuxtrack-foohid-bridge
Utility to translate Linuxtrack coordinates into Foohid commands.

Uses example code from:
  https://github.com/uglyDwarf/linuxtrack
  https://github.com/unbit/foohid

## To use:
1. install Linuxtrack (follow instructions above)
2. install foohid driver (joystick emulator)
3. run hidtrack
4. in the game, set up view controls to follow virtual joystick

## Notes
All 6DOF are mapped.

X, Y, Z - head movements

RX, RY, RZ - head rotations - roll (acha side to side), pitch (nod) and heading (turn left) respectively
