#include <fat.hpp>

int fileCount = 0;
int selectedIndex = 0;
char fileNames[MAX_FILES][MAX_FILENAME_LENGTH];

bool isGBA(const char* filename) {
    const char *ext = strrchr(filename, '.');  // Find the last '.'
    return (ext && strcasecmp(ext, ".gba") == 0);  // Case-insensitive match
}

void ListFiles() {
    DIR* dir;
    struct dirent* entry;
    fileCount = 0;

    dir = opendir("/");
    if (!dir) {
        RenderText(1, "SD Card not found", 5, 1);
        return;
    }

    while ((entry = readdir(dir)) != NULL && fileCount < MAX_FILES) {
        if (entry->d_type != DT_DIR && isGBA(entry->d_name)) {  
            strncpy(fileNames[fileCount], entry->d_name, MAX_FILENAME_LENGTH);
            fileNames[fileCount][MAX_FILENAME_LENGTH - 1] = '\0';  // Ensure null termination
            fileCount++;
        }
    }

    closedir(dir);

    if (fileCount == 0) {
        RenderText(1, "No .gba files found", 5, 3);
    }
}

void RenderFileList() {
    ClearText(1);
    for (int i = 0; i < fileCount; i++) {
        if (i == selectedIndex) {
            RenderText(1, ">", 3, i + 3);  // Highlight current selection
        }
        RenderText(1, fileNames[i], 5, i + 3);
    }
}
