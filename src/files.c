#include <fat.h>
#include <text.h>
#include <files.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <nds/arm9/input.h>
#include <nds/interrupts.h>

uint8_t scrollPos = 0; 
uint16_t fileCount = 0;
uint16_t scrollOffset = 0;
uint16_t selectedIndex = 0;
char currentPath[256] = "/";  // Start in root directory
bool isDirectory[MAX_FILES];  // Track if it's a folder
char fileNames[MAX_FILES][MAX_FILENAME_LENGTH];

bool initFAT() {
    return fatInitDefault();
}

void ListFiles() {
    DIR* dir;
    struct dirent* entry;
    fileCount = 0;

    if (currentPath[0] == '\0') {
        strcpy(currentPath, "/"); // Reset to root if empty
    }

    dir = opendir(currentPath);
    if (!dir) {
        ClearText(1);
        RenderText(1, "Failed to open dir", 3, 4);
        return;
    }

    memset(fileNames, 0, sizeof(fileNames));
    memset(isDirectory, 0, sizeof(isDirectory));

    while ((entry = readdir(dir)) != NULL && fileCount < MAX_FILES) {
        if (entry->d_type == DT_DIR) {  // Folders
            snprintf(fileNames[fileCount], MAX_FILENAME_LENGTH, "(%s)", entry->d_name);
            isDirectory[fileCount] = true;
            fileCount++;
        } else if (strstr(entry->d_name, ".gba")) {  // Show only .gba files
            strncpy(fileNames[fileCount], entry->d_name, MAX_FILENAME_LENGTH);
            isDirectory[fileCount] = false;
            fileCount++;
        }
    }

    closedir(dir);

    // Ensure the screen is cleared before rendering the new file list
    ClearText(1);
    RenderFileList();

    if (fileCount == 0) {
        RenderText(1, "No files found", 3, 6);
    }
}


void RenderFileList() {
    ClearText(1);

    for (int i = 0; i < MAX_VISIBLE_FILES; i++) {
        int fileIndex = scrollOffset + i;
        if (fileIndex >= fileCount) break;

        char displayText[MAX_DISPLAY_LENGTH + 4]; // 22 chars + "..." + null terminator
        const char *filename = fileNames[fileIndex];

        // Selected file scrolls horizontally if too long
        if (fileIndex == selectedIndex && strlen(filename) > MAX_DISPLAY_LENGTH) {
            snprintf(displayText, sizeof(displayText), "%.22s...", filename + scrollPos);
            if (scrollPos > strlen(filename) - MAX_DISPLAY_LENGTH) {
                scrollPos = 0;  // Reset scroll when reaching the end
            } else {
                scrollPos++;  // Advance scroll
            }
        } else {
            // Truncate long names and add "..."
            if (strlen(filename) > MAX_DISPLAY_LENGTH) {
                snprintf(displayText, sizeof(displayText), "%.22s...", filename);
            } else {
                snprintf(displayText, sizeof(displayText), "%s", filename);
            }
        }

        if (fileIndex == selectedIndex) {
            RenderText(1, ">", 3, i + 3);  // Highlight selection
        }
        RenderText(1, displayText, 4, i + 3);
    }
    
    swiWaitForVBlank();
}


void ChangeDirectory(const char* folder) {
    // If going up a directory
    if (strcmp(folder, "..") == 0) {
        char* lastSlash = strrchr(currentPath, '/');
        if (lastSlash && lastSlash != currentPath) {
            *lastSlash = '\0';  // Remove last folder from path
        } else {
            strcpy(currentPath, "/");  // Return to root
        }
    } else {
        // Strip parentheses from directory name
        char cleanFolder[MAX_FILENAME_LENGTH];
        size_t len = strlen(folder);
        
        if (folder[0] == '(' && folder[len - 1] == ')') {
            strncpy(cleanFolder, folder + 1, len - 2);
            cleanFolder[len - 2] = '\0';
        } else {
            strncpy(cleanFolder, folder, MAX_FILENAME_LENGTH);
        }

        // Ensure path doesn't exceed buffer size
        if (strlen(currentPath) + strlen(cleanFolder) + 2 < sizeof(currentPath)) {
            if (strcmp(currentPath, "/") != 0) {
                strcat(currentPath, "/");
            }
            strcat(currentPath, cleanFolder);
        }
    }

    // Refresh file list and UI
    ListFiles();
    selectedIndex = 0;
    scrollOffset = 0;
    RenderFileList();
}

extern uint32_t romSize;

bool DisplayROMInfo(const char* filename, uint32_t romSize) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        RenderLine(0, "Error opening file!", 10);
        return false;
    }

    // Read first 192 bytes for ROM metadata
    uint8_t header[192];
    fread(header, 1, 192, file);
    fclose(file);

    // Extract ROM details
    char romTitle[13] = {0};
    char gameCode[5] = {0};
    char makerCode[3] = {0};

    memcpy(romTitle, &header[0xA0], 12);
    memcpy(gameCode, &header[0xAC], 4);
    memcpy(makerCode, &header[0xB0], 2);

    // Ensure null-terminated strings
    romTitle[12] = '\0';
    gameCode[4] = '\0';
    makerCode[2] = '\0';

    // Clear top screen and display ROM info
    ClearText(0);
    RenderLine(0, "ROM INFORMATION", 3);
    RenderLine(0, "===============", 4);

    RenderLine(0, "TITLE:", 6);
    RenderText(0, romTitle, 10, 6);
    RenderLine(0, "GAME CODE:", 7);
    RenderText(0, gameCode, 14, 7);
    RenderLine(0, "MAKER CODE:", 8);
    RenderText(0, makerCode, 15, 8);

    char sizeMsg[32];
    snprintf(sizeMsg, sizeof(sizeMsg), "SIZE: %d KB", romSize / 1024);
    RenderLine(0, sizeMsg, 9);

    // Warning prompt
    RenderLine(0, "WARNING:", 11);
    RenderLine(0, "THIS WILL ERASE AND", 12);
    RenderLine(0, "OVERWRITE THE CART!", 13);

    RenderLine(0, "PRESS START TO CONTINUE.", 15);
    RenderLine(0, "PRESS B TO CANCEL.", 16);

    // Wait for user input
    while (1) {
        scanKeys();
        uint32_t keys = keysDown();

        if (keys & KEY_START) return true;  // Proceed with flashing
        if (keys & KEY_B) return false;     // Cancel and return to file list

        swiWaitForVBlank();
    }
}
