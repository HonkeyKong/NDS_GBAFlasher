#include <nds.h> 
#include <text.h>
#include <fat.hpp>
#include <flash.h>
#include <input.h>
#include <gfx.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

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
                RenderFileList();
            }
        }

        if (keys & KEY_A) {
            if (isDirectory[selectedIndex]) {
                ChangeDirectory(fileNames[selectedIndex]);  
            } else {
                char fullPath[256];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", currentPath, fileNames[selectedIndex]);

                if (LoadAndFlashROM(fullPath)) {
                    ListFiles();
                    RenderFileList();
                }

                // Instead of locking up, just delay input briefly after a selection.
                scanKeys();
                while (keysHeld() & KEY_A) {
                    swiWaitForVBlank();
                    scanKeys();  // This ensures we actually update the key state
                }
            }
        }

        if (keys & KEY_B) {
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
