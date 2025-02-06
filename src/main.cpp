#include <nds.h>
#include <text.h>
#include <fat.hpp>
#include <flash.h>
#include <gfx.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

extern uint32_t cartSize;

int main(void) {
    // Enable interrupts
    irqInit();
    irqEnable(IRQ_VBLANK);

    // Set ARM9 as owner of both cart slots.
    sysSetCardOwner(true);
    sysSetCartOwner(true);

    // Self-explanatory functions here
    setupVideo();
    LoadGraphics();
    
    // Clear text layers on both screens
    ClearText(0);
    ClearText(1);

    // Write some text on the top screen
    RenderLine(0, "GBA Flasher by HonkeyKong", 3);
    RenderLine(0, "Initializing...", 4);
    
    // Set up the SD card (If you're using a slot-2 card, you're dumb.)
    if (!fatInitDefault()) {
        RenderText(0, "FAT init failed!", 3, 5);
    } else {
        RenderLine(0, "SD Card Initialized", 5);
        ListFiles();
        RenderFileList();
    }

    // What kind of cart are we working with?
    uint16_t chipType = DetectChipType();
    switch (chipType) {
        case 0:
            RenderLine(0, "Unknown cart type.", 6);
        case 8:
            RenderLine(0, "Detected S29GL064N (8MB)", 6);
            cartSize = (8 * 1024 * 1024);
            break;
        case 16:
            RenderLine(0, "Detected S29GL128N (16MB)", 6);
            cartSize = (16 * 1024 * 1024);
            break;
        case 0xFFFF:
            RenderLine(0, "CFI Query Failed!", 6);
        default:
            RenderLine(0, "Unknown cart type.", 6);
            break;
    }

    // Explain how the UI works
    RenderLine(0, "Select ROM with UP/DOWN", 8);
    RenderLine(0, "Press A to start flashing", 9);

    // ROM Selection Loop
    while (1) {
        scanKeys();
        uint32_t keys = keysDown();

        if (keys & KEY_DOWN) {
            if (selectedIndex < fileCount - 1) selectedIndex++;
            RenderFileList();
        }
        if (keys & KEY_UP) {
            if (selectedIndex > 0) selectedIndex--;
            RenderFileList();
        }
        if (keys & KEY_A) {
            break; // Confirm selection
        }
        swiWaitForVBlank();
    }

    // Load and Flash ROM
    RenderLine(0, "Loading ROM...", 11);
    if(!LoadAndFlashROM(fileNames[selectedIndex])) {
        RenderLine(0, "Flash failed!", 12);
    } else {
        RenderLine(0, "Flash succcess!", 12);
    }

    // Self-explanatory function.
    bgUpdate();

    // Infinite loop
    while (1) {
        swiWaitForVBlank();
    }

    // Return something, because it has to be there.
    return 0;
}
