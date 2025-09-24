#include "entity/components/renderer/renderer-list.h"
#include "entity/components/renderer/renderer.h"
#include "entity/entity.h"
#include <stdlib.h>

int RendererCount = 0;
Component **Renderers = NULL;

void RendererList_Init()
{
    printf("RendererList_Init()\n");
    RendererCount = 0;
    Renderers = NULL;
}

void RendererList_Add(EC_Renderer *renderer)
{
    RendererCount++;
    printf("RendererList_Add(%s) now at %d renderers.\n", renderer->component->entity->name, RendererCount);
    if (Renderers == NULL)
    {
        Renderers = malloc(sizeof(Component *));
    }
    else
    {
        Renderers = realloc(Renderers, RendererCount * sizeof(Component *));
    }
    Renderers[RendererCount - 1] = renderer->component;
}

void RendererList_Free(){
    free(Renderers);
}