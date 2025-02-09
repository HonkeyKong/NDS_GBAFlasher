#include <gfx.h>
#include <nds.h> 
#include <text.h>
#include <audio.h>
#include <files.h>
#include <flash.h>
#include <input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <maxmod9.h>
#include <sfx/soundbank.h>

extern uint32_t cartSize;
extern uint16_t scrollOffset;

void infiniteLoop() { while(1); }

int main(void) {
    ntrcardOpen();
    gbacartOpen();

    setupVideo();
    LoadGraphics();

    ClearText(0);
    ClearText(1);

    RenderLine(0, "GBA Flasher by HonkeyKong", 3);
    RenderLine(0, "Initializing...", 4);

    initSound();

    if (!initFAT()) {
        RenderText(0, "FAT init failed!", 3, 5);
        infiniteLoop();
    } else {
        RenderLine(0, "SD Card Initialized", 5);
        ListFiles();
        RenderFileList();
    }

    uint16_t chipType = DetectChipType();
    switch (chipType) {
        case 8:
            RenderLine(0, "Detected S29GL064N (8MB)", 6);
            cartSize = (8 * 1024 * 1024);
            break;
        case 16:
            RenderLine(0, "Detected S29GL128N (16MB)", 6);
            cartSize = (16 * 1024 * 1024);
            break;
        default:
            RenderLine(0, "Unknown cart type.", 6);
            infiniteLoop();
            break;
    }

    RenderLine(0, "Select ROM with UP/DOWN", 8);
    RenderLine(0, "Press A to select.", 9);
    RenderLine(0, "Press B to go back.", 10);

    int scrollTimer = 0;

    while (1) {
        scanKeys();
        uint32_t keys = keysDown();

        if (keys & KEY_DOWN) {
            if (selectedIndex < fileCount - 1) {
                selectedIndex++;
                scrollPos = 0;

                if (selectedIndex >= scrollOffset + MAX_VISIBLE_FILES) {
                    scrollOffset++;
                }
                PlaySound(SFX_CLICK);
                RenderFileList();
            }
        }

        if (keys & KEY_UP) {
            if (selectedIndex > 0) {
                selectedIndex--;
                scrollPos = 0;

                if (selectedIndex < scrollOffset) {
                    scrollOffset--;
                }
                PlaySound(SFX_CLICK);
                RenderFileList();
            }
        }

        if (keys & KEY_A) {
            PlaySound(SFX_SELECT);
            if (isDirectory[selectedIndex]) {
                ChangeDirectory(fileNames[selectedIndex]);  
            } else {
                char fullPath[256];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", currentPath, fileNames[selectedIndex]);

                if (LoadAndFlashROM(fullPath)) {
                    ListFiles();
                    RenderFileList();
                }
                scanKeys();
                while (keysHeld() & KEY_A) {
                    swiWaitForVBlank();
                    scanKeys(); 
                }
            }
        }

        if (keys & KEY_B) {
            PlaySound(SFX_CANCEL);
            ChangeDirectory("..");
        }

        if (scrollTimer++ > 10) {
            scrollTimer = 0;
            RenderFileList();
        }

        swiWaitForVBlank();
    }

    return 0;
}
