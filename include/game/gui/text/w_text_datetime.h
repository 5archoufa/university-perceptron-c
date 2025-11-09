#ifndef GAME_GUI_TEXT_W_TEXT_TIME_H
#define GAME_GUI_TEXT_W_TEXT_TIME_H

// Text
#include "entity/components/gui/w_text.h"
// Entity
#include "entity/entity.h"

// ------------------------- 
// Types 
// -------------------------

typedef struct
{
    // Component
    Component *component;
    // Text
    W_Text *w_text;
    // Cache
    size_t stringBufferSize;
    char *stringBuffer;
} W_Text_DateTime;

// ------------------------- 
// Prefabs 
// -------------------------

W_Text_DateTime *Prefab_W_Text_DateTime(EC_GUI *gui, Entity *parent, TextFont *font, int fontSize, V3 position, Quaternion rotation, uint32_t color);

#endif