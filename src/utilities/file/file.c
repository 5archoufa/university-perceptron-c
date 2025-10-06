#include "utilities/file/file.h"
#include "logging/logger.h"

static LogConfig _logConfig = {"File", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

char *File_LoadStr(const char *filename)
{
    FILE *file = fopen(filename, "rb"); // open in binary mode to avoid newline conversions
    if (!file)
    {
        LogError(&_logConfig, "Error: Could not open file %s\n", filename);
        return NULL;
    }

    // Seek to end to determine file size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    if (length <= 0)
    {
        LogError(&_logConfig, "Error: File %s is empty or unreadable\n", filename);
        fclose(file);
        return NULL;
    }

    // Allocate memory (+1 for null terminator)
    char *buffer = (char *)malloc(length + 1);
    if (!buffer)
    {
        LogError(&_logConfig, "Error: Out of memory reading %s\n", filename);
        fclose(file);
        return NULL;
    }

    // Read file contents
    size_t bytesRead = fread(buffer, 1, length, file);
    buffer[bytesRead] = '\0'; // Null terminate the string

    fclose(file);
    return buffer;
}