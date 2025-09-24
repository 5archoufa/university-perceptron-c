#ifndef RENDERER_LIST_H
#define RENDERER_LIST_H

#include <stdlib.h>
#include "entity/entity.h"
#include "entity/components/renderer/renderer.h"

extern int RendererCount;
extern Component **Renderers;

void RendererList_Init();
void RendererList_Add(EC_Renderer *renderer);
void RendererList_Free();
#endif