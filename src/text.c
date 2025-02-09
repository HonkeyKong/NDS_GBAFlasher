#include <text.h>
#include <stdio.h>
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
