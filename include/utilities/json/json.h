#ifndef JSON_H
#define JSON_H

// C
#include <stdlib.h>
#include <stdbool.h>

// ----------------------------------------
// Types 
// ----------------------------------------

typedef enum JSONEntryType{
    JSON_T_NULL,
    JSON_T_BOOL,
    JSON_T_NUMBER,
    JSON_T_STRING,
    JSON_T_ARRAY,
    JSON_T_OBJECT
} JSONType;


typedef struct JSONEntry{
    char *key;
    JSONType type;
    union {
        bool boolValue;
        double numberValue;
        char *stringValue;
        struct {
            struct JSON **items;
            size_t size;
        } arrayValue;
        struct {
            struct JSON **pairs;
            size_t size;
        } objectValue;
    } value;
} JSONEntry;

typedef struct JSON{
    size_t entries_size;
    JSONEntry **entries;
} JSON;

// ----------------------------------------
// API
// ----------------------------------------

JSON *JSON_Parse(char *jsonString);
void JSON_Free(JSON *json);

#endif