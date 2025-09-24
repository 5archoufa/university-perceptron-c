typedef enum LogLevel
{
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO
} LogLevel;

typedef enum LogColor
{
    LOG_COLOR_DEFAULT,
    LOG_COLOR_RED,
    LOG_COLOR_GREEN,
    LOG_COLOR_ORANGE,
    LOG_COLOR_BLUE,
    LOG_COLOR_GRAY
} LogColor;

typedef struct LogConfig
{
    char *name;
    LogLevel logLevel;
    int color;
} LogConfig;

void Log(LogConfig *config, const char *format, ...);
void LogSuccess(LogConfig *config, const char *format, ...);
void LogWarning(LogConfig *config, const char *format, ...);
void LogError(LogConfig *config, const char *format, ...);
void LogFree(LogConfig *config, const char *format, ...);
void LogInit(LogConfig *config, const char *format, ...);
void LogCreate(LogConfig *config, const char *format, ...);