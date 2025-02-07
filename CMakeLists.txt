# GBA Flasher for Nintendo DS
# Author: Ryan Souders (HonkeyKong)
cmake_minimum_required(VERSION 3.13)

set(DEVKITPRO /opt/devkitpro)
set(CMAKE_TOOLCHAIN_FILE ${DEVKITPRO}/cmake/NDS.cmake)

project(GBAFlasher C CXX ASM)

include_directories(${CMAKE_SOURCE_DIR}/res)
include_directories(${CMAKE_SOURCE_DIR}/include)

set(BG_SRC "${CMAKE_SOURCE_DIR}/res/guibg.c")
set(FONT_SRC "${CMAKE_SOURCE_DIR}/res/font.c")
set(BG_PNG "${CMAKE_SOURCE_DIR}/res/guibg.png")
set(FONT_PNG "${CMAKE_SOURCE_DIR}/res/font.png")

set(NDS_TITLE "GBA Flasher")
set(NDS_LINE1 "By HonkeyKong")
set(NDS_LINE2 "http://www.honkeykong.org/")
set(NDS_ICON "${CMAKE_SOURCE_DIR}/res/gbaflash.bmp")

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

add_custom_target(generate_gfx_src ALL DEPENDS ${FONT_SRC} ${BG_SRC})

set( SOURCE_FILES
    ${BG_SRC}
    ${FONT_SRC}
    src/fat.cpp
    src/gfx.cpp
    src/flash.c
    src/input.c
    src/text.cpp
    src/main.cpp )

add_executable( GBAFlasher ${SOURCE_FILES} )
add_dependencies( GBAFlasher generate_gfx_src )
target_link_libraries( GBAFlasher nds9 fat )
nds_create_rom(GBAFlasher NAME ${NDS_TITLE} SUBTITLE1 ${NDS_LINE1} SUBTITLE2 ${NDS_LINE2} ICON ${NDS_ICON})
