#ifndef TEXT_HPP
#define TEXT_HPP
#ifdef __cplusplus
#include <string>
#include <cstring>

extern "C" {
#endif // __cplusplus
#include <font.h>
#include <stdint.h>

extern uint8_t currentLine;

void InitText();
void ClearText(uint8_t screen);
const char* HexString(uint16_t value);
const char* HexString32(uint32_t value);
void RenderLine(uint8_t screen, const char* text, uint8_t line);
void RenderText(uint8_t screen, const char* text, uint8_t x, uint8_t y);
void RenderLineWithValue(uint8_t screen, const char* text, uint32_t value, uint8_t line);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TEXT_HPP