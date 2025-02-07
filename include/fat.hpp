#ifndef FAT_HPP
#define FAT_HPP

#define MAX_FILES 256          // Increase to handle larger lists
#define MAX_VISIBLE_FILES 18   // Max files shown on screen at once
#define MAX_DISPLAY_LENGTH 22
#define MAX_FILENAME_LENGTH 64

extern uint8_t scrollPos;
extern char currentPath[256];
extern bool isDirectory[MAX_FILES];
extern uint16_t fileCount, selectedIndex;
extern char fileNames[MAX_FILES][MAX_FILENAME_LENGTH];

bool initFAT();
void ListFiles();
void RenderFileList();
void ChangeDirectory(const char* folder);

#endif // FAT_HPP