#include <cstdio>
#include <text.h>
#include <gfx/font.h>
#include <nds/interrupts.h>
#include <nds/arm9/input.h>
#include <nds/arm9/video.h>

#define TILE_WIDTH 32
#define TILE_HEIGHT 21

// VRAM base pointers for BG0
extern u16* bg0MapTop;
extern u16* bg0MapBottom;
uint8_t currentLine = 3;

void ClearText(uint8_t screen) {
    memset(screen? bg0MapBottom : bg0MapTop, 0, 32 * 24 * 2); // Clear the 32x24 tile map (Visible area)
    currentLine = 3;
}

void RenderText(uint8_t screen, const char *text, uint8_t x, uint8_t y) {
    u16 *bg = (screen? bg0MapBottom : bg0MapTop);
    uint16_t index = y * TILE_WIDTH + x;
    while (*text && x < TILE_WIDTH) {
        char ch = *text++;
        if (ch >= 'a' && ch <= 'z') {
            ch -= 32; // Convert lowercase to uppercase
        }
        if (ch >= 32 && ch <= 126) {  // Printable ASCII range
            bg[index++] = (ch) | (1 << 12); // Set palette 1
        } else {
            bg[index++] = 0; // Fallback to empty tile
        }
        x++;
    }
}

void RenderLine(uint8_t screen, const char *text, uint8_t line) {
    if (line < TILE_HEIGHT) {
        RenderText(screen, text, 3, line);
    } else {
        ClearText(screen);
        currentLine = 3;
        RenderText(screen, text, 3, currentLine++);
    }
}

void RenderLineWithValue(uint8_t screen, const char* text, uint32_t value, uint8_t line) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s %d", text, value);
    RenderLine(screen, buffer, line);
}

const char* HexString(uint16_t value) {
    static char hexStr[5]; // 4 digits + null terminator
    const char hexDigits[] = "0123456789ABCDEF";
    hexStr[0] = hexDigits[(value >> 12) & 0xF];
    hexStr[1] = hexDigits[(value >> 8) & 0xF];
    hexStr[2] = hexDigits[(value >> 4) & 0xF];
    hexStr[3] = hexDigits[value & 0xF];
    hexStr[4] = '\0';
    return hexStr;
}

const char* HexString32(uint32_t value) {
    static char hexBuf[9]; // 8 hex digits + null terminator

    for (int i = 7; i >= 0; i--) {
        int nib = value & 0xF;
        hexBuf[i] = (nib < 10) ? (char)('0' + nib) : (char)('A' + (nib - 10));
        value >>= 4;
    }

    hexBuf[8] = '\0'; // Null-terminate
    return hexBuf;
}

extern uint32_t romSize;

bool DisplayROMInfo(const char* filename, uint32_t romSize) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        RenderLine(0, "Error opening file!", 10);
        return false;
    }

    // Read first 192 bytes for ROM metadata
    uint8_t header[192];
    fread(header, 1, 192, file);
    fclose(file);

    // Extract ROM details
    char romTitle[13] = {0};
    char gameCode[5] = {0};
    char makerCode[3] = {0};

    memcpy(romTitle, &header[0xA0], 12);
    memcpy(gameCode, &header[0xAC], 4);
    memcpy(makerCode, &header[0xB0], 2);

    // Ensure null-terminated strings
    romTitle[12] = '\0';
    gameCode[4] = '\0';
    makerCode[2] = '\0';

    // Clear top screen and display ROM info
    ClearText(0);
    RenderLine(0, "ROM INFORMATION", 3);
    RenderLine(0, "===============", 4);

    RenderLine(0, "TITLE:", 6);
    RenderText(0, romTitle, 10, 6);
    RenderLine(0, "GAME CODE:", 7);
    RenderText(0, gameCode, 14, 7);
    RenderLine(0, "MAKER CODE:", 8);
    RenderText(0, makerCode, 15, 8);

    char sizeMsg[32];
    snprintf(sizeMsg, sizeof(sizeMsg), "SIZE: %d KB", romSize / 1024);
    RenderLine(0, sizeMsg, 9);

    // Warning prompt
    RenderLine(0, "WARNING:", 11);
    RenderLine(0, "THIS WILL ERASE AND", 12);
    RenderLine(0, "OVERWRITE THE CART!", 13);

    RenderLine(0, "PRESS START TO CONTINUE.", 15);
    RenderLine(0, "PRESS B TO CANCEL.", 16);

    // Wait for user input
    while (1) {
        scanKeys();
        uint32_t keys = keysDown();

        if (keys & KEY_START) return true;  // Proceed with flashing
        if (keys & KEY_B) return false;     // Cancel and return to file list

        swiWaitForVBlank();
    }
}
