#include <stdarg.h>
#include <stdio.h>
#include "logging/logger.h"

const char *COLOR_RESET = "]:: \x1b[0m";
const char* LINE_BREAK = "\n";
const char* MSG_FREE_START = "[🗑️] ";
const char* MSG_FREE_MID = "Free(";
const char* MSG_FREE_END = ")\n";
const char* MSG_INIT_START = "[🏁] ";
const char* MSG_INIT_MID = "Init(";
const char* MSG_INIT_END = ")\n";
const char* MSG_CREATE_START = "[👶] ";
const char* MSG_CREATE_MID = "Create(";
const char* MSG_CREATE_END = ")\n";

static const char *GetColorChar(LogColor c)
{
    switch (c)
    {
    case LOG_COLOR_RED:
        return "\x1b[31m[";
    case LOG_COLOR_GREEN:
        return "\x1b[32m[";
    case LOG_COLOR_BLUE:
        return "\x1b[34m[";
    case LOG_COLOR_ORANGE:
        return "\x1b[33m[";
    case LOG_COLOR_GRAY:
        return "\x1b[90m[";
    case LOG_COLOR_DEFAULT:
    default:
        return "\x1b[37m[";
    }
}


static void PrintName(LogColor color, char* name){
    const char *colorStr = GetColorChar(color);
    fputs(colorStr, stdout);
    fputs(name, stdout);
    fputs(COLOR_RESET, stdout);
}

void Log(LogConfig *config, const char *format, ...)
{
    if(config->logLevel < LOG_LEVEL_INFO) return;
    PrintName(config->color, config->name);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fputs(LINE_BREAK, stdout);
    fflush(stdout);
}

void LogSuccess(LogConfig *config, const char *format, ...)
{
    PrintName(LOG_COLOR_GREEN, config->name);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fputs(LINE_BREAK, stdout);
    fflush(stdout);
}

void LogWarning(LogConfig *config, const char *format, ...)
{
    if(config->logLevel < LOG_LEVEL_WARN) return;
    PrintName(LOG_COLOR_ORANGE, config->name);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fputs(LINE_BREAK, stdout);
    fflush(stdout);
}

void LogError(LogConfig *config, const char *format, ...)
{
    if(config->logLevel < LOG_LEVEL_ERROR) return;
    PrintName(LOG_COLOR_RED, config->name);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fputs(LINE_BREAK, stdout);
    fflush(stdout);
}

void LogFree(LogConfig *config, const char *format, ...)
{
    if(config->logLevel < LOG_LEVEL_INFO) return;
    fputs(MSG_FREE_START, stdout);
    PrintName(LOG_COLOR_GRAY, config->name);
    fputs(MSG_FREE_MID, stdout);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fputs(MSG_FREE_END, stdout);
    fflush(stdout);
}

void LogCreate(LogConfig *config, const char *format, ...)
{
    if(config->logLevel < LOG_LEVEL_INFO) return;
    fputs(MSG_CREATE_START, stdout);
    PrintName(LOG_COLOR_GRAY, config->name);
    fputs(MSG_CREATE_MID, stdout);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fputs(MSG_CREATE_END, stdout);
    fflush(stdout);
}

void LogInit(LogConfig *config, const char *format, ...)
{
    if(config->logLevel < LOG_LEVEL_INFO) return;
    fputs(MSG_INIT_START, stdout);
    PrintName(LOG_COLOR_BLUE, config->name);
    fputs(MSG_INIT_MID, stdout);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fputs(MSG_INIT_END, stdout);
    fflush(stdout);
}