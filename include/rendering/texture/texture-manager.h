#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <stddef.h>

// forward declare Texture so this header doesn't need to include texture.h
typedef struct Texture Texture;

typedef struct TextureManager TextureManager;

struct TextureManager{
    size_t textures_size;
    Texture **textures;
};

// -------------------------
// Creation and Freeing
// -------------------------

TextureManager *TextureManager_Create();
void TextureManager_Free(TextureManager *manager);

// -------------------------
// Functions
// -------------------------

void TextureManager_Select(TextureManager* manager);
void TextureManager_AddTexture(Texture *texture);
void TextureManager_Cleanup();

#endif
