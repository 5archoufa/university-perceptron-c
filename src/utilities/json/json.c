#include "utilities/json/json.h"
// C
#include <stdlib.h>
#include <string.h>

// ----------------------------------------
// API 
// ----------------------------------------

JSON *JSON_Parse(char *jsonString)
{
    if(jsonString == NULL) return NULL;
    int len = strlen(jsonString);
    if(len == 0) return NULL;


    
    return NULL;
}

JSONEntry *JSON_GetValue(JSON *json, char *key)
{
    for (size_t i = 0; i < json->entries_size; i++)
    {
        JSONEntry *entry = json->entries[i];
        if (strcmp(entry->key, key) == 0)
        {
            return entry;
        }
    }
    return NULL;
}