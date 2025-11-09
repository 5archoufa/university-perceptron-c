#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// -------------------------
// Types
// -------------------------

typedef struct RenderTarget
{
    GLuint FBO;
    GLuint colorTexture;
    GLuint depthRBO;
    int width;
    int height;
} RenderTarget;

// -------------------------
// Functions
// -------------------------

RenderTarget *RenderTarget_Create(int width, int height);
void RenderTarget_Free(RenderTarget *rt);

#endif