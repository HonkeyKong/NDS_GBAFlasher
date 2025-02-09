#include <text.h>
#include <audio.h>
#include <flash.h>
#include <input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sfx/soundbank.h>

#define FLASH_BASE 0x08000000 // Start of cartridge ROM space
#define _FLASH_WRITE(pa, pd) { *((uint16_t*)FLASH_BASE + ((pa) / 2)) = (pd); __asm("nop"); }

bool bitSwapped = false;

uint8_t readByte(int addr) {
    uint8_t data = *((uint16_t*)FLASH_BASE + (addr / 2));
    if (bitSwapped) {
        data = (data & 0xFC) | ((data & 1) << 1) | ((data & 2) >> 1);
    }
    return data;
}

uint32_t cartSize = 0;
uint32_t region0Size = 0;
uint32_t region1Size = 0;
uint16_t region0Sectors = 0;
uint16_t region1Sectors = 0;

// Query the ROM chip using CFI
bool QueryCFI() {
    _FLASH_WRITE(0x0000, 0xF0); // Reset the chip to normal mode
    _FLASH_WRITE(0xAA, 0x98);  // Enter CFI mode

    uint16_t Q = *(uint16_t*)(FLASH_BASE + 0x20);
    uint16_t R = *(uint16_t*)(FLASH_BASE + 0x22);
    uint16_t Y = *(uint16_t*)(FLASH_BASE + 0x24);

    if (Q == 'Q' && R == 'R' && Y == 'Y') {
        bitSwapped = false;
    } else if (Q == 'R' && R == 'Q' && Y == 'Z') {
        bitSwapped = true;
    } else {
        return false;
    }

    // Detect sector layout
    uint8_t regionCount = readByte(0x58); // Adjusts for D0/D1 swap automatically

    for (uint8_t region = 0; region < regionCount; ++region) {
        uint32_t baseOffset = 0x5A + region * 8;
        uint16_t sectorCount = readByte(baseOffset) | (readByte(baseOffset + 2) << 8);
        uint16_t sectorSize = readByte(baseOffset + 4) | (readByte(baseOffset + 6) << 8);

        if (region == 0) {
            region0Sectors = sectorCount + 1;
            region0Size = sectorSize * 256; // 8KB sectors for region 0
        } else {
            region1Sectors = sectorCount + 1;
            region1Size = sectorSize * 256; // Larger sectors for region 1
        }
    }

    _FLASH_WRITE(0x0000, 0xF0); // Exit CFI mode
    return true;
}

uint16_t DetectChipType() {
    if (!QueryCFI()) {
        return 0xFFFF; // Failed to query the chip
    }

    // Use the queried sector layout info
    if (region0Sectors == 8 && region0Size == 8192 && region1Sectors == 127 && region1Size == 65536) {
        cartSize = (8 * 1024 * 1024);
        return 8;  // S29GL064N (8MB)
    } 
    else if (region0Sectors > 8 && region0Size >= 128 * 1024) {
        cartSize = (16 * 1024 * 1024);
        return 16; // S29GL128N (16MB)
    } 
    else {
        return 0;  // Unknown chip
    }
}

void EraseSector(uint32_t address) {
    _FLASH_WRITE(0xAAA, 0xA9);
    _FLASH_WRITE(0x555, 0x56);
    _FLASH_WRITE(0xAAA, 0x80);
    _FLASH_WRITE(0xAAA, 0xA9);
    _FLASH_WRITE(0x555, 0x56);
    _FLASH_WRITE(address, 0x30); // Sector erase command

    // Wait for completion
    while (*(volatile uint16_t*)(FLASH_BASE + address) != 0xFFFF) {
        __asm("nop");
    }
}

bool VerifyData(uint32_t address, const uint8_t* data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        uint8_t readValue = *(uint8_t*)(FLASH_BASE + address + i);
        uint8_t expectedValue = data[i];

        // Log each mismatch for debugging
        if (readValue != expectedValue) {
            return false;
        }
    }
    return true;
}

void WriteData(uint32_t address, const uint8_t* data, uint32_t size) {
    for (uint32_t i = 0; i < size; i += 2) {
        uint16_t word = data[i] | (data[i + 1] << 8); // Combine two bytes into a 16-bit word
        _FLASH_WRITE(0xAAA, 0xA9);
        _FLASH_WRITE(0x555, 0x56);
        _FLASH_WRITE(0xAAA, 0xA0);
        _FLASH_WRITE(address + i, word);

        // Wait for the write to complete
        while (*(uint16_t*)(FLASH_BASE + address + i) != word) {
            __asm("nop");
        }
    }
}

uint32_t romSize = 0;  // Global ROM size variable

bool LoadAndFlashROM(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        RenderLine(0, "ERROR OPENING FILE!", 12);
        return false;
    }

    fseek(file, 0, SEEK_END);
    uint32_t romSize = ftell(file);
    rewind(file);

    // Show ROM Info and confirm flashing
    if (!DisplayROMInfo(filename, romSize)) {
        fclose(file);
        PlaySound(SFX_CANCEL);
        return false; // User canceled
    }

    if (romSize > cartSize) {
        fclose(file);
        RenderLine(0, "ROM TOO BIG FOR CART!", 17);
        PlaySound(SFX_FAIL);
        return false;
    }

    // Start flashing
    RenderLine(0, "FLASHING ROM...", 19);

    uint8_t* buffer = (uint8_t*)malloc(CHUNK_SIZE);
    if (!buffer) {
        fclose(file);
        RenderLine(0, "MEMORY ALLOCATION FAILED!", 20);
        PlaySound(SFX_FAIL);
        return false;
    }

    uint32_t offset = 0;

    // Write ROM data in chunks
    for (uint32_t i = 0; i < region0Sectors && offset < romSize; i++) {
        uint32_t sectorAddress = i * region0Size;
        uint32_t bytesToRead = (romSize - offset > region0Size) ? region0Size : (romSize - offset);

        fread(buffer, 1, bytesToRead, file);
        EraseSector(sectorAddress);
        WriteData(sectorAddress, buffer, bytesToRead);

        offset += bytesToRead;

        // Display progress
        char progressMsg[32];
        snprintf(progressMsg, sizeof(progressMsg), "Progress: %d%%", (offset * 100) / romSize);
        RenderLine(0, progressMsg, 20);
    }

    for (uint32_t i = 0; i < region1Sectors && offset < romSize; i++) {
        uint32_t sectorAddress = region0Sectors * region0Size + i * region1Size;
        uint32_t bytesToRead = (romSize - offset > region1Size) ? region1Size : (romSize - offset);

        fread(buffer, 1, bytesToRead, file);
        EraseSector(sectorAddress);
        WriteData(sectorAddress, buffer, bytesToRead);

        offset += bytesToRead;

        // Display progress
        char progressMsg[32];
        snprintf(progressMsg, sizeof(progressMsg), "Progress: %d%%", (offset * 100) / romSize);
        RenderLine(0, progressMsg, 20);
    }

    RenderLine(0, "FLASH COMPLETE!", 21);
    PlaySound(SFX_SUCCESS);

    free(buffer);
    fclose(file);
    return true;
}
