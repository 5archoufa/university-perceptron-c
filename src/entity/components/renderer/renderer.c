#include "entity/components/renderer/renderer.h"
#include "entity/components/renderer/renderer-list.h"

const unsigned long PIXEL_BLACK = 0x000000;
const unsigned long PIXEL_WHITE = 0xFFFFFF;

void EC_Renderer_Free(Component *component)
{
    EC_Renderer *renderer = component->self;
    renderer->renderData_Free(renderer);
    free(renderer);
}

EC_Renderer *EC_Renderer_Create(Entity *entity,
                                void *renderData,
                                void (*Render)(EC_Camera *camera, EC_Renderer *renderer),
                                void (*renderData_Free)(EC_Renderer *))
{
    EC_Renderer *renderer = malloc(sizeof(EC_Renderer));
    renderer->renderData = renderData;
    renderer->renderData_Free = renderData_Free;
    renderer->Render = Render;
    Bounds_Setup(&renderer->bounds, &entity->position, V2_ZERO, &entity->scale, &entity->pivot);
    Component *component = Component_Create(renderer, entity, EC_RENDERER, EC_Renderer_Free, NULL, NULL, NULL, NULL);
    renderer->component = component;
    RendererList_Add(renderer);
    return renderer;
}