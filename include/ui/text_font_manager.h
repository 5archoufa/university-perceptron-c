#ifndef UI_TEXT_H
#define UI_TEXT_H

// File
#include "utilities/file/file.h"
// stb_truetype
#include "third_party/stb_truetype.h"
// Math
#include "utilities/math/v2.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>


// -------------------------
// Types
// -------------------------

typedef struct
{
    /// @brief UV coordinates in the atlas
    float u0, v0, u1, v1;
    /// @brief size of glyph in pixels
    float width, height;    
    /// @brief offset from cursor position to top-left of glyph
    float xOffset, yOffset;
    /// @brief how much to move the cursor after drawing this glyph
    float xAdvance;
} Glyph;

typedef struct
{
    // Font info
    char *font_family;
    float scale;
    /// @brief The pixel height the font was baked at
    float bakedPixelHeight;
    // Atlas
    unsigned char *atlasBitmap;
    GLuint atlasTextureID;
    V2 atlasSize;
    // Glyphs
    Glyph *glyphs;
} TextFont;

typedef struct{
    TextFont **fonts;
    size_t fonts_size;
    float fontPixelHeight;
    V2 atlasSize;
} TextFontManager;

// ------------------------- 
// Management 
// -------------------------

void TextFontManager_Select(TextFontManager *manager);

// ------------------------- 
// Creating & Freeing 
// -------------------------

TextFontManager *TextFontManager_Create(float fontPixelHeight, V2 atlasSize);
void TextFontManager_Free(TextFontManager *manager);

// ------------------------- 
// Text Fonts 
// -------------------------

TextFont* TextFont_Create(const char *font_family, const char *fontPath);
TextFont* TextFont_Get(const char *font_family);
TextFont* TextFont_GetDefault();

#endif