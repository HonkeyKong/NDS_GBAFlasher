#include <audio.h>
#include <sfx/sb.h>
#include <sfx/soundbank.h>

void initSound() {
    mmInitDefaultMem((mm_addr)soundbank_bin);
    mmLoadEffect(SFX_CANCEL);
    mmLoadEffect(SFX_CLICK);
    mmLoadEffect(SFX_FAIL);
    mmLoadEffect(SFX_PROMPT);
    mmLoadEffect(SFX_SELECT);
    mmLoadEffect(SFX_SUCCESS);
    mmSelectMode(MM_MODE_A);
}

void PlaySound(uint8_t effect) {
    if (effect > 5) return; // Ignore invalid effects

    mm_sound_effect sfx = {
        { effect },    // Sound effect ID
        (int)(1.0f * (1 << 10)),  // Playback rate (normal speed)
        0,  // Handle (not needed for one-shot effects)
        255,  // Volume (max)
        127   // Panning (center)
    };

    mmEffectEx(&sfx);
}
