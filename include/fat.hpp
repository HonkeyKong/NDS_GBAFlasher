#ifndef FAT_HPP
#define FAT_HPP

#include <fat.h>
#include <text.h>
#include <cstring>
#include <dirent.h>

#define MAX_FILES 50
#define MAX_FILENAME_LENGTH 64

extern int fileCount, selectedIndex;
extern char fileNames[MAX_FILES][MAX_FILENAME_LENGTH];

void ListFiles();
void RenderFileList();
bool isGBA(const char* filename);

#endif // FAT_HPP