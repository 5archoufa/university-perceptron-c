#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>

typedef struct Texture Texture;

struct Texture
{
    int width;
    int height;
    uint32_t *data; // ABGR
    GLuint textureID; 
    /// @brief Reference count for shared textures. Do not modify this directly, use Texture_AddRef and Texture_Release.
    int refCount;
};

void Texture_Free(Texture *texture);
Texture *Texture_Create(int width, int height);
void Texture_Resize(Texture *texture, int width, int height);
Texture* Texture_CreateEmpty();
void Texture_UploadToGPU(Texture *texture);

#endif
