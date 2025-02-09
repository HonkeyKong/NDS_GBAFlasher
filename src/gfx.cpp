#include <gfx.hpp>
#include <gfx/font.h>
#include <gfx/guibg.h>
#include <nds/arm9/video.h>
#include <nds/arm9/background.h>

// VRAM base pointer for BG0 and BG1
u16* bg0MapTop;
u8* bg0GfxTop;
u16* bg1MapTop;
u8* bg1GfxTop;

u16* bg0MapBottom;
u8* bg0GfxBottom;
u16* bg1MapBottom;
u8* bg1GfxBottom;

void setupVideo() {
    videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE);
    videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE);

    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankB(VRAM_B_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);

    int bg0Top = bgInit(0, BgType_Text4bpp, BgSize_T_256x256, 31, 0);
    int bg1Top = bgInit(1, BgType_Text4bpp, BgSize_T_256x256, 29, 2);
    int bg0Bottom = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 15, 4);
    int bg1Bottom = bgInitSub(1, BgType_Text4bpp, BgSize_T_256x256, 14, 6);

    bg0MapTop = (u16*)bgGetMapPtr(bg0Top);
    bg0GfxTop = (u8*)bgGetGfxPtr(bg0Top);
    bg1MapTop = (u16*)bgGetMapPtr(bg1Top);
    bg1GfxTop = (u8*)bgGetGfxPtr(bg1Top);

    bg0MapBottom = (u16*)bgGetMapPtr(bg0Bottom);
    bg0GfxBottom = (u8*)bgGetGfxPtr(bg0Bottom);
    bg1MapBottom = (u16*)bgGetMapPtr(bg1Bottom);
    bg1GfxBottom = (u8*)bgGetGfxPtr(bg1Bottom);
}

void LoadGraphics() {
    dmaCopy(fontPal, (uint16_t*)BG_PALETTE + 16, fontPalLen);
    dmaCopy(fontPal, (uint16_t*)BG_PALETTE_SUB + 16, fontPalLen);
    dmaCopy(fontTiles, bg0GfxTop, fontTilesLen);
    dmaCopy(fontTiles, bg0GfxBottom, fontTilesLen);

    dmaCopy(guibgPal, (uint16_t*)BG_PALETTE, guibgPalLen);
    dmaCopy(guibgPal, (uint16_t*)BG_PALETTE_SUB, guibgPalLen);
    dmaCopy(guibgTiles, bg1GfxTop, guibgTilesLen);
    dmaCopy(guibgTiles, bg1GfxBottom, guibgTilesLen);

    dmaCopy(guibgMap, bg1MapTop, guibgMapLen);
    dmaCopy(guibgMap, bg1MapBottom, guibgMapLen);
}
