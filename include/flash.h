#ifndef FLASH_H
#define FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define CHUNK_SIZE (128 * 1024)

void FlashROM();
bool QueryCFI();
uint16_t DetectChipType();
uint8_t readByte(int addr);
void EraseSector(uint32_t address);
bool LoadAndFlashROM(const char* filename);
void WriteData(uint32_t address, const uint8_t* data, uint32_t size);
bool VerifyData(uint32_t address, const uint8_t* data, uint32_t size);
void WriteROMToCartridge(const uint8_t* romData, uint32_t romSize);

#ifdef __cplusplus
}
#endif

#endif // FLASH_H