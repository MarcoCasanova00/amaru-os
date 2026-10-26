Read-only compatibility fixture from the earlier Bunkai Wolf A3 / AMB platform. Original GPL-3.0 notices retained. Does not enter the Bunker firmware.
Run from project root after building:
cc -O1 -DOWN_PKG=1 -o /tmp/bunker-compat-ota tests/compat-wolf/tests/ota_test.c
/tmp/bunker-compat-ota firmware/build/felucca.fwsc
