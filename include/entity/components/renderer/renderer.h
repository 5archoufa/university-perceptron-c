#ifndef EC_RENDERER_H
#define EC_RENDERER_H

typedef struct Component Component;
typedef struct EC_Renderer EC_Renderer;
typedef struct EC_Camera EC_Camera;
typedef struct Entity Entity;
typedef struct World World;

#include "entity/components/renderer/bounds.h"
#include "entity/components/camera/camera.h"
#include <stdlib.h>
#include "world/world.h"
#include "entity/entity.h"
#include <X11/Xlib.h>
#include <stdint.h>

extern const uint32_t PIXEL_BLACK;
extern const uint32_t PIXEL_WHITE;

struct EC_Renderer
{
    Component *component;
    void *renderData;
    void (*renderData_Free)(EC_Renderer *);
    void (*Render)(EC_Camera *camera, EC_Renderer *renderer);
    void (*UpdateBounds)(EC_Renderer *renderer);
    Bounds bounds;
};
void EC_Renderer_Free(Component *component);
EC_Renderer *EC_Renderer_Create(Entity *entity,
                                void *renderData,
                                void (*Render)(EC_Camera *camera, EC_Renderer *renderer),
                                void (*renderData_Free)(EC_Renderer *),
                                void (*UpdateBounds)(EC_Renderer *));
uint32_t Color_SetAlpha(uint32_t color, float alpha);
uint32_t Color_Over(uint32_t fg, uint32_t bg);
uint32_t Color_Modulate_Grayscale(uint32_t color, float gray);

#endif