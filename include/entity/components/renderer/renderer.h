#ifndef EC_RENDERER_H
#define EC_RENDERER_H

#include "entity/components/renderer/bounds.h"
#include "entity/components/camera/camera.h"
#include <stdlib.h>
#include "entity/entity.h"
#include <X11/Xlib.h>

typedef struct EC_Renderer EC_Renderer;

extern const unsigned long PIXEL_BLACK;
extern const unsigned long PIXEL_WHITE;


struct EC_Renderer
{
    Component *component;
    void *renderData;
    void (*renderData_Free)(EC_Renderer *);
    void(*Render)(EC_Camera* camera, EC_Renderer* renderer);
    Bounds bounds;
};
void EC_Renderer_Free(Component *component);
EC_Renderer *EC_Renderer_Create(Entity *entity,
                                void *renderData,
                                void (*Render)(EC_Camera *camera, EC_Renderer *renderer),
                                void (*renderData_Free)(EC_Renderer *));

#endif