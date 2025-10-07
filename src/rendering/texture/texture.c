#include "rendering/texture/texture.h"
#include "rendering/texture/texture-manager.h"
// C
#include <stdlib.h>
#include <stdio.h>

void Texture_FreeFromGPU(Texture *texture)
{
    if (texture->textureID != 0)
    {
        glDeleteTextures(1, &texture->textureID);
        texture->textureID = 0;
    }
}

void Texture_UploadToGPU(Texture *texture)
{
    Texture_FreeFromGPU(texture);
    glGenTextures(1, &texture->textureID);
    glBindTexture(GL_TEXTURE_2D, texture->textureID);
    printf("Uploading texture: %dx%d, textureID: %u, data ptr: %p\n", 
           texture->width, texture->height, texture->textureID, texture->data);
    // Upload pixel data (ABGR format = GL_RGBA with GL_UNSIGNED_BYTE)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 texture->width, texture->height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error after texture upload: %d\n", err);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture_Free(Texture *texture)
{
    if (!texture)
    {
        return;
    }
    Texture_FreeFromGPU(texture);
    free(texture->data);
    free(texture);
}

Texture *Texture_Create(int width, int height)
{
    Texture *texture = malloc(sizeof(Texture));
    texture->width = width;
    texture->height = height;
    texture->data = malloc(width * height * sizeof(uint32_t));
    texture->textureID = 0;
    texture->refCount = 0;
    TextureManager_AddTexture(texture); // Add to global list for cleanup
    return texture;
}

void Texture_Resize(Texture *texture, int width, int height)
{
    Texture_FreeFromGPU(texture);
    if (texture->width != width || texture->height != height)
    {
        free(texture->data);
        texture->data = malloc(width * height * sizeof(uint32_t));
        texture->width = width;
        texture->height = height;
    }
}

Texture *Texture_CreateEmpty()
{
    Texture *texture = malloc(sizeof(Texture));
    texture->width = 0;
    texture->height = 0;
    texture->data = NULL;
    texture->textureID = 0;
    return texture;
}