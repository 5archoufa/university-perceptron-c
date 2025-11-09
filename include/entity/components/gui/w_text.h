#ifndef W_TEXT_H
#define W_TEXT_H

// GUI
#include "entity/components/gui/ec_gui.h"
// Text
#include "ui/text_font_manager.h"
// Rendering
#include "rendering/mesh/mesh.h"
// Math
#include "utilities/math/v2.h"
#include "utilities/math/v3.h"
#include <cglm/cglm.h>

// -------------------------
// Types
// -------------------------

typedef struct
{
    Component *component;
    EC_GUI_Widget *widget;
    // Text properties
    size_t allocatedCharCapacity;
    size_t lastCharCount;
    char *text;
    uint32_t color;
    // Font
    TextFont *font;
    int font_size;
    // Rendering
    Mesh *textMesh;
    bool requiresRebuild;
    vec4 textColor;
} W_Text;

// -------------------------
// Creation & Freeing
// -------------------------

W_Text *W_Text_Create(EC_GUI *ec_gui, Entity *entity, TextFont *font, const char *text, int fontSize, uint32_t color);

// -------------------------
// Mesh Generation
// -------------------------

void W_Text_BuildMesh(W_Text *w_text);
void W_Text_Render(W_Text *w_text, EC_GUI *ec_gui, mat4 model);

// -------------------------
// Setters
// -------------------------

void W_Text_SetText(W_Text *w_text, const char *text);
void W_Text_SetFont(W_Text *w_text, TextFont *font);
void W_Text_SetFontSize(W_Text *w_text, int fontSize);
void W_Text_SetPosition(W_Text *w_text, V3 position);
void W_Text_SetColor(W_Text *w_text, uint32_t color);

// -------------------------
// Prefabs
// -------------------------

W_Text *Prefab_W_Text(EC_GUI *ec_gui, Entity *parent, TextFont *font, const char *text, int fontSize, V3 position, Quaternion rotation, uint32_t color);

#endif