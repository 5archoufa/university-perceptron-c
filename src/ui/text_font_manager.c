#include "ui/text_font_manager.h"
// C
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
// Logging
#include "logging/logger.h"
// Math
#include "utilities/math/v2.h"
// stbtt_fontinfo
#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb_truetype.h"

// -------------------------
// Static Variables
// -------------------------

static TextFontManager *_manager = NULL;
static LogConfig _logConfig = {"Text", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

// -------------------------
// Creation & Deletion
// -------------------------

TextFontManager *TextFontManager_Create(float fontPixelHeight, V2 atlasSize)
{
    TextFontManager *manager = malloc(sizeof(TextFontManager));
    manager->fonts = NULL;
    manager->fonts_size = 0;
    manager->fontPixelHeight = fontPixelHeight;
    manager->atlasSize = atlasSize;
    LogCreate(&_logConfig, "TextFontManager");
    TextFontManager_Select(manager);
    return manager;
}

static void TextFont_Free(TextFont *font)
{
    LogFree(&_logConfig, "TextFont: %s", font->font_family);
    free(font->font_family);
    free(font->glyphs);
    glDeleteTextures(1, &font->atlasTextureID);
    free(font->atlasBitmap);
    free(font);
}

void TextFontManager_Free(TextFontManager *manager)
{
    LogFree(&_logConfig, "TextFontManager");
    for (size_t i = 0; i < manager->fonts_size; i++)
    {
        TextFont_Free(manager->fonts[i]);
    }
    free(manager->fonts);
    if (_manager == manager)
    {
        _manager = NULL;
        LogWarning(&_logConfig, "The freed TextFontManager was the currently selected one. _manager is now NULL.");
    }
    free(manager);
}

// -------------------------
// Management
// -------------------------

void TextFontManager_Select(TextFontManager *manager)
{
    if (manager == _manager)
    {
        return;
    }
    LogSuccess(&_logConfig, "Selected a different TextFontManager");
    _manager = manager;
}

// -------------------------
// Utilities
// -------------------------

static void TextFontManager_AddFont(TextFont *font)
{
    _manager->fonts_size++;
    _manager->fonts = realloc(_manager->fonts, _manager->fonts_size * sizeof(TextFont *));
    _manager->fonts[_manager->fonts_size - 1] = font;
}

// -------------------------
// Text Fonts
// -------------------------

TextFont *TextFont_Get(const char *font_family)
{
    if (_manager == NULL)
    {
        LogError(&_logConfig, "No TextFontManager selected. Cannot get font: %s", font_family);
        return NULL;
    }
    for (size_t i = 0; i < _manager->fonts_size; i++)
    {
        if (strcmp(_manager->fonts[i]->font_family, font_family) == 0)
        {
            return _manager->fonts[i];
        }
    }
    LogWarning(&_logConfig, "Font not found: %s", font_family);
    return NULL;
}

TextFont *TextFont_GetDefault()
{
    if (_manager == NULL || _manager->fonts_size == 0)
    {
        LogError(&_logConfig, "No TextFontManager selected or no fonts available.");
        return NULL;
    }
    return _manager->fonts[0];
}

TextFont *TextFont_Create(const char *font_family, const char *fontPath)
{
    LogWarning(&_logConfig, "Creating font: %s from path: %s", font_family, fontPath);
    
    // Load the font file into memory
    unsigned char *fontBuffer = File_LoadBinary(fontPath); // use fontPath parameter
    if (!fontBuffer)
    {
        LogError(&_logConfig, "Failed to load font file: %s", fontPath);
        return NULL;
    }
    
    LogWarning(&_logConfig, "Font file loaded successfully");
    
    // Initialize stb_truetype
    stbtt_fontinfo ttFont;
    if (!stbtt_InitFont(&ttFont, fontBuffer, 0))
    {
        free(fontBuffer);
        LogError(&_logConfig, "Failed to initialize font: %s", font_family);
        return NULL;
    }
    
    LogWarning(&_logConfig, "stb_truetype font initialized successfully");
    // Allocate the TextFont struct
    TextFont *font = malloc(sizeof(TextFont));
    // Font family name
    font->font_family = malloc(strlen(font_family) + 1);
    strcpy(font->font_family, font_family);
    // Calculate scale for desired pixel height
    float scale = stbtt_ScaleForPixelHeight(&ttFont, _manager->fontPixelHeight);
    font->bakedPixelHeight = _manager->fontPixelHeight;
    
    // Allocate atlas bitmap (grayscale 8-bit)
    int atlasWidth = _manager->atlasSize.x;
    int atlasHeight = _manager->atlasSize.y;
    
    unsigned char *atlasBitmap = calloc(atlasWidth * atlasHeight, sizeof(unsigned char));
    // Prepare baked chars array
    int asciiStart = 32;
    int asciiEnd = 126;
    int asciiCount = asciiEnd - asciiStart + 1;
    stbtt_bakedchar bakedChars[asciiCount]; // ASCII 32..126
    
    // Bake font bitmap into atlas
    int bakedResult = stbtt_BakeFontBitmap(fontBuffer, 0, _manager->fontPixelHeight,
                             atlasBitmap, atlasWidth, atlasHeight,
                             asciiStart, asciiCount, bakedChars);
    
    if (bakedResult <= 0)
    {
        free(atlasBitmap);
        free(fontBuffer);
        free(font->font_family);
        free(font);
        LogError(&_logConfig, "Failed to bake font bitmap for font: %s", font_family);
        return NULL;
    }
    
    // Calculate the actual used height by finding the bottom-most pixel
    // Scan from top to bottom to find where glyphs end
    int actualHeight = 0;
    for (int y = 0; y < atlasHeight; y++) {
        int hasPixel = 0;
        for (int x = 0; x < atlasWidth; x++) {
            if (atlasBitmap[y * atlasWidth + x] > 0) {
                hasPixel = 1;
                break;
            }
        }
        if (hasPixel) {
            actualHeight = y + 1; // Keep updating to find the last row with pixels
        }
    }
    
    // Add some padding to the actual height
    actualHeight += 4; // Small padding
    if (actualHeight < 32) actualHeight = 32; // Minimum height
    
    // Create optimized bitmap with actual dimensions
    unsigned char *optimizedBitmap = calloc(atlasWidth * actualHeight, sizeof(unsigned char));
    
    // Copy only the used portion
    for (int y = 0; y < actualHeight; y++) {
        memcpy(optimizedBitmap + y * atlasWidth, 
               atlasBitmap + y * atlasWidth, 
               atlasWidth);
    }
    
    // Scale the V coordinates accordingly
    float vScale = (float)atlasHeight / (float)actualHeight;
    
    Glyph *glyphs = malloc(sizeof(Glyph) * asciiCount);
    for (int i = 0; i < asciiCount; ++i)
    {
        stbtt_bakedchar *b = &bakedChars[i];
        
        // Calculate UV coordinates with V scaling for optimized texture
        float u0 = b->x0 / (float)atlasWidth;
        float v0 = (b->y0 / (float)atlasHeight) * vScale;  // Scale V to optimized space
        float u1 = b->x1 / (float)atlasWidth;
        float v1 = (b->y1 / (float)atlasHeight) * vScale;  // Scale V to optimized space
        
        glyphs[i] = (Glyph){
            .u0 = u0,
            .v0 = v0,
            .u1 = u1,
            .v1 = v1,
            .width = (float)(b->x1 - b->x0),
            .height = (float)(b->y1 - b->y0),
            .xOffset = b->xoff,
            .yOffset = b->yoff,
            .xAdvance = b->xadvance};
    }
    
    // Store data in TextFont with optimized dimensions
    font->glyphs = glyphs;
    font->atlasBitmap = optimizedBitmap; // Use optimized bitmap
    font->atlasSize = (V2){atlasWidth, actualHeight}; // Use actual height
    font->scale = scale;
    
    // Free the original oversized bitmap
    free(atlasBitmap);

    // Upload the Atlas to the GPU
    glGenTextures(1, &font->atlasTextureID);
    glBindTexture(GL_TEXTURE_2D, font->atlasTextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, actualHeight, 0,
                 GL_RED, GL_UNSIGNED_BYTE, optimizedBitmap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // ============ Save atlas as BMP ============ // 
    char outputPath[atlasWidth];
    // Extract directory from fontPath and create output filename
    const char *lastSlash = strrchr(fontPath, '/');
    const char *lastBackslash = strrchr(fontPath, '\\');
    const char *lastSeparator = (lastSlash > lastBackslash) ? lastSlash : lastBackslash;
    
    if (lastSeparator) {
        // Copy directory path
        size_t dirLen = lastSeparator - fontPath + 1;
        strncpy(outputPath, fontPath, dirLen);
        outputPath[dirLen] = '\0';
        strcat(outputPath, font_family);
        strcat(outputPath, "_atlas.bmp");
    } else {
        // No directory, just use current directory
        sprintf(outputPath, "%s_atlas.bmp", font_family);
    }
    
    LogWarning(&_logConfig, "Optimized atlas saved to: %s (%dx%d)", outputPath, atlasWidth, actualHeight);
    
    // Save as BMP (8-bit grayscale) using optimized dimensions
    FILE *file = fopen(outputPath, "wb");
    if (file) {
        // BMP file header (14 bytes)
        uint16_t bfType = 0x4D42; // "BM"
        uint32_t bfSize = 54 + 256*4 + atlasWidth * actualHeight; // File size with actual height
        uint16_t bfReserved1 = 0;
        uint16_t bfReserved2 = 0;
        uint32_t bfOffBits = 54 + 256*4; // Offset to pixel data
        
        fwrite(&bfType, 2, 1, file);
        fwrite(&bfSize, 4, 1, file);
        fwrite(&bfReserved1, 2, 1, file);
        fwrite(&bfReserved2, 2, 1, file);
        fwrite(&bfOffBits, 4, 1, file);
        
        // BMP info header (40 bytes)
        uint32_t biSize = 40;
        int32_t biWidth = atlasWidth;
        int32_t biHeight = -actualHeight; // Negative for top-down, use actual height
        uint16_t biPlanes = 1;
        uint16_t biBitCount = 8; // 8-bit grayscale
        uint32_t biCompression = 0;
        uint32_t biSizeImage = atlasWidth * actualHeight; // Use actual height
        int32_t biXPelsPerMeter = 2835; // ~72 DPI
        int32_t biYPelsPerMeter = 2835;
        uint32_t biClrUsed = 256;
        uint32_t biClrImportant = 0;
        
        fwrite(&biSize, 4, 1, file);
        fwrite(&biWidth, 4, 1, file);
        fwrite(&biHeight, 4, 1, file);
        fwrite(&biPlanes, 2, 1, file);
        fwrite(&biBitCount, 2, 1, file);
        fwrite(&biCompression, 4, 1, file);
        fwrite(&biSizeImage, 4, 1, file);
        fwrite(&biXPelsPerMeter, 4, 1, file);
        fwrite(&biYPelsPerMeter, 4, 1, file);
        fwrite(&biClrUsed, 4, 1, file);
        fwrite(&biClrImportant, 4, 1, file);
        
        // Color palette (256 grayscale entries)
        for (int i = 0; i < 256; i++) {
            uint8_t color[4] = {i, i, i, 0}; // B, G, R, Reserved
            fwrite(color, 4, 1, file);
        }
        
        // Write optimized bitmap data (BMP stores bottom-up, but we use negative height for top-down)
        size_t written = fwrite(optimizedBitmap, 1, atlasWidth * actualHeight, file);
        fclose(file);
        
        LogWarning(&_logConfig, "Optimized atlas saved successfully as BMP (%zu bytes)", written);
        
    } else {
        LogError(&_logConfig, "Failed to save atlas image to %s", outputPath);
    }

    // Add font to manager
    TextFontManager_AddFont(font);
    LogCreate(&_logConfig, "Created TextFont: %s", font_family);
    return font;
}

