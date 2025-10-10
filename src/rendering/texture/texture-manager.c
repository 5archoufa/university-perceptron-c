#include "rendering/texture/texture-manager.h"
#include "rendering/texture/texture.h"

#include <stdlib.h>

static TextureManager* _manager = NULL;

// -------------------------
// Creation and Freeing
// -------------------------

TextureManager *TextureManager_Create()
{
    TextureManager *manager = malloc(sizeof(TextureManager));
    manager->textures_size = 0;
    manager->textures = NULL;

    // Pre-allocate some space
    manager->textures_size = 10;
    manager->textures = malloc(sizeof(Texture *) * manager->textures_size);
    for (int i = 0; i < manager->textures_size; i++)
    {
        manager->textures[i] = NULL;
    }
    return manager;
}

void TextureManager_Free(TextureManager *manager)
{
    if (!manager)
    {
        return;
    }
    for (int i = 0; i < manager->textures_size; i++)
    {
        Texture_Free(manager->textures[i]);
    }
    free(manager->textures);
    free(manager);
}

// -------------------------
// Functions
// -------------------------

void TextureManager_Select(TextureManager* manager)
{
    _manager = manager;
}

void TextureManager_Cleanup()
{
    for (int i = 0; i < _manager->textures_size; i++)
    {
        if(_manager->textures[i] == NULL) continue;
        if (_manager->textures[i] && _manager->textures[i]->refCount <= 0)
        {
            Texture_Free(_manager->textures[i]);
            _manager->textures[i] = NULL;
        }
    }
}

void TextureManager_AddTexture(Texture *texture)
{
    // Check if any slots are free before-hand
    for (int i = 0; i < _manager->textures_size; i++)
    {
        if (_manager->textures[i] == NULL)
        {
            _manager->textures[i] = texture;
            return;
        }
    }
    _manager->textures_size++;
    _manager->textures = realloc(_manager->textures, sizeof(Texture *) * _manager->textures_size);
    _manager->textures[_manager->textures_size - 1] = texture;
}