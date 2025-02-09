# GBA Flasher for Nintendo DS
# Author: Ryan Souders (HonkeyKong)
cmake_minimum_required(VERSION 3.13)

set(DEVKITPRO /opt/devkitpro)
set(CMAKE_TOOLCHAIN_FILE ${DEVKITPRO}/cmake/NDS.cmake)

project(GBAFlasher C CXX ASM)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/res/src)

set(NDS_TITLE "GBA Flasher")
set(NDS_LINE1 "By HonkeyKong")
set(NDS_LINE2 "http://www.honkeykong.org/")
set(NDS_ICON "${CMAKE_SOURCE_DIR}/res/gfx/gbaflash.bmp")

# Directories where resource files are located
set(WAV_DIR "${CMAKE_SOURCE_DIR}/res/sfx")
set(GFX_DIR "${CMAKE_SOURCE_DIR}/res/gfx")
set(BG_PNG "${GFX_DIR}/bg/guibg.png")
set(FONT_PNG "${GFX_DIR}/font/font.png")
set(BG_SRC "${CMAKE_SOURCE_DIR}/res/src/gfx/guibg.c")
set(FONT_SRC "${CMAKE_SOURCE_DIR}/res/src/gfx/font.c")

add_custom_command(
    OUTPUT ${FONT_SRC}
    COMMAND grit ${FONT_PNG} -gt -gB4 -pn16 -ftc -o${FONT_SRC}
    DEPENDS ${FONT_PNG}
    COMMENT "Converting font..."
    VERBATIM
)

add_custom_command(
    OUTPUT ${BG_SRC}
    COMMAND grit ${BG_PNG} -gt -gB4 -mRtf -pn16 -ftc -o${BG_SRC}
    DEPENDS ${BG_PNG}
    COMMENT "Converting background..."
    VERBATIM
)

add_custom_target(generate_gfx DEPENDS ${BG_SRC} ${FONT_SRC})

# Find all .wav files in the directory
file(GLOB WAV_FILES "${WAV_DIR}/*.wav")

set(SFX_HEADER "${CMAKE_SOURCE_DIR}/res/src/sfx/soundbank.h")
set(SFX_BIN "${CMAKE_SOURCE_DIR}/res/src/sfx/soundbank.bin")
set(SB_HEADER "${CMAKE_SOURCE_DIR}/res/src/sfx/sb.h")
set(SB_ASM "${CMAKE_SOURCE_DIR}/res/src/sfx/sb.s")

add_custom_command(
    OUTPUT ${SFX_BIN} ${SFX_HEADER}
    COMMAND mmutil ${WAV_FILES} -d -o${SFX_BIN} -h${SFX_HEADER}

    DEPENDS ${WAV_FILES}
    COMMENT "Generating MaxMod soundbank..."
    VERBATIM
)

add_custom_target(generate_soundbank DEPENDS ${SFX_BIN} ${SFX_HEADER})

add_custom_command(
    OUTPUT ${SB_HEADER} ${SB_ASM}
    COMMAND bin2s -H ${SB_HEADER} ${SFX_BIN} > ${SB_ASM}
    DEPENDS ${SFX_BIN}
    COMMENT "Generating Soundbank header..."
    VERBATIM
)

add_custom_target(generate_sb_header DEPENDS ${SB_HEADER} ${SB_ASM})
                
set( SOURCE_FILES
${BG_SRC}
${SB_ASM}
${FONT_SRC}
src/fat.cpp
src/gfx.cpp
src/flash.c
src/input.c
src/text.cpp
src/main.cpp
src/audio.cpp )
            
add_executable( GBAFlasher ${SOURCE_FILES} )
add_dependencies( GBAFlasher generate_gfx )
add_dependencies(GBAFlasher generate_soundbank)
add_dependencies( GBAFlasher generate_sb_header )
target_link_libraries( GBAFlasher nds9 fat mm9 )
nds_create_rom(GBAFlasher NAME ${NDS_TITLE} SUBTITLE1 ${NDS_LINE1} SUBTITLE2 ${NDS_LINE2} ICON ${NDS_ICON})
