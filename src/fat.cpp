#include <fat.h>
#include <text.h>
#include <cstring>
#include <fat.hpp>
#include <dirent.h>
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
