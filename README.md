## GBA Flasher for Nintendo DS

This program can load a GBA ROM from a Slot-1 flashcart like the R4, and flash it to one of those cheap bootleg GBA carts you can find everywhere on [AliExpress](https://www.aliexpress.us/w/wholesale-GBA%20cartridge.html). Right now, the 8MB (Link to the Past) and 16MB (Minish Cap/Pokemon) cartridges are supported, and probably other games of the same size. 32MB carts (like Kingdom Hearts) will probably come after I get my hands on one.

To build the software, just run this from the project root:  

```
mkdir build
cd build
arm-none-eabi-cmake ..
make
```

Now you've got a `GBAFlasher.nds` file that you can load from your flashcart, and use to write to your bootleg GBA cartridge.

The simplest workflow for flashing a game to a bootleg cartridge is like so:  
  
  * Load `GBAFlasher.nds` from your R4 or similar flashcart.
  * Navigate the menu on the bottom screen with the D-pad.
  * Select the ROM you want to flash with the A button.
  * Confirm you want to flash the ROM with the Start button.
  
Now, just sit back and wait. This method isn't quite as fast as flashing from a PC. A 4MB ROM flashed in approximately 1 minute, 18 seconds. For a 16MB flash, it took around 5 minutes, 20 seconds. However, if you compare this to [my GBA link flashing software](https://github.com/HonkeyKong/GBAFlashTools), which takes approximately 10 minutes to flash a 4MB ROM file, it's a massive improvement. All in all, I just want to give homebrew developers as many ways to produce their own game cartridges as possible.
