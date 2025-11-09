#include "rendering/render_target.h"
// C
#include <stdlib.h>
#include <stdio.h>

// ------------------------- 
// Creation & Freeing 
// -------------------------

RenderTarget *RenderTarget_Create(int width, int height)
{
    RenderTarget *rt = malloc(sizeof(RenderTarget));
    rt->width = width;
    rt->height = height;

    // Create framebuffer
    glGenFramebuffers(1, &rt->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, rt->FBO);

    // Create color texture
    glGenTextures(1, &rt->colorTexture);
    glBindTexture(GL_TEXTURE_2D, rt->colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // CRITICAL: Use GL_NEAREST for pixel-perfect scaling!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt->colorTexture, 0);

    // Create depth renderbuffer
    glGenRenderbuffers(1, &rt->depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, rt->depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rt->depthRBO);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR: Framebuffer is not complete!\n");
        free(rt);
        return NULL;
    }

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return rt;
}

void RenderTarget_Free(RenderTarget *rt)
{
    glDeleteFramebuffers(1, &rt->FBO);
    glDeleteTextures(1, &rt->colorTexture);
    glDeleteRenderbuffers(1, &rt->depthRBO);
    free(rt);
}