#include <input.h>
#include <nds/interrupts.h>
#include <nds/arm9/input.h>

void WaitForA() {
    while (1) {
        scanKeys();
        if (!(keysHeld() & KEY_A)) break;  // Wait for A to be released
        swiWaitForVBlank();
    }
}

void WaitForNoInput() {
    while (keysHeld()) { // Wait until all buttons are released
        scanKeys();
        swiWaitForVBlank();
    }
}
