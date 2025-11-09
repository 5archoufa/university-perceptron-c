#include "game/gui/text/w_text_datetime.h"
// Text
#include "entity/components/gui/w_text.h"

// ------------------------- 
// Utilities 
// -------------------------

inline static void UpdateString(W_Text_DateTime *w_text_datetime){
    SimulatedDateTime *dt = World_GetDateTime();
    DateTime_ToString(dt, w_text_datetime->stringBuffer, w_text_datetime->stringBufferSize);
    // Update W_Text
    W_Text_SetText(w_text_datetime->w_text, w_text_datetime->stringBuffer);
}

// ------------------------- 
// Entity Events 
// -------------------------

static void Update(Component *component)
{
    UpdateString(component->self);
}

// ------------------------- 
// Creation & Freeing 
// -------------------------

static void Free(Component *component)
{
    W_Text_DateTime *w_text_datetime = component->self;
    free(w_text_datetime->stringBuffer);
    free(w_text_datetime);
}

W_Text_DateTime *W_Text_DateTime_Create(EC_GUI *gui, Entity *entity, W_Text *w_text)
{
    W_Text_DateTime *w_text_datetime = malloc(sizeof(W_Text_DateTime));
    // Text
    w_text_datetime->w_text = w_text;
    // DateTime String Buffer
    size_t stringBufferSize = *DateTime_GetStringBufferSize();
    w_text_datetime->stringBufferSize = stringBufferSize;
    w_text_datetime->stringBuffer = malloc(stringBufferSize);
    // Component
    w_text_datetime->component = Component_Create(w_text_datetime, entity, EC_T_W_TEXT_DATETIME, Free, NULL, NULL, Update, NULL, NULL);
    // Initial string update
    UpdateString(w_text_datetime);
    return w_text_datetime;
}

W_Text_DateTime *Prefab_W_Text_DateTime(EC_GUI *gui, Entity *parent, TextFont *font, int fontSize, V3 position, Quaternion rotation, uint32_t color)
{
    // Create Text
    W_Text *w_text = Prefab_W_Text(gui, parent, font, "0000-00-00 00:00:00", fontSize, position, rotation, color);
    // Create W_Text_DateTime
    W_Text_DateTime *w_text_datetime = W_Text_DateTime_Create(gui, w_text->component->entity, w_text);
    return w_text_datetime;
}