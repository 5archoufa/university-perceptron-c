#include "entity/components/camera/camera.h"
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include "ui/window.h"
#include "entity/entity.h"
#include "entity/components/renderer/renderer.h"
#include "entity/components/renderer/bounds.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

static bool EC_Camera_IsInView(EC_Camera *camera, Bounds *bounds)
{
    V3 *position = &camera->component->entity->position;
    
    float right = position->x + camera->viewport.x * 0.5;
    float left = position->x - camera->viewport.x * 0.5;
    float up = position->y + camera->viewport.y * 0.5;
    float down = position->y - camera->viewport.y * 0.5;
    // printf("Camera Viewport: R:%f L:%f U:%f D:%f\n", right, left, up, down);

    float sr = bounds->position.x + bounds->dimensions.x;
    float sl = bounds->position.x;
    float su = bounds->position.y + bounds->dimensions.y;
    float sd = bounds->position.y;
    // printf("Shape Bounds: R:%f L:%f U:%f D:%f\n", sr, sl, su, sd);
    if (sr < left || sl > right || sd > up || su < down)
    {
        return false;
    }
    return true;
}

static void Camera_LateUpdate(Component *component)
{
    EC_Camera *camera = component->self;
    int width = camera->image->width;
    uint32_t *pixels = (uint32_t *)camera->image->data;

    // Paint foreground
    for (int y = 0; y < camera->image->height; y++)
    {
        for (int x = 0; x < camera->image->width; x++)
        {
            pixels[(int)y * width + (int)x] = PIXEL_BLACK;
        }
    }

    // Render Renderers
    int rendererCount =  camera->world->rendererCount;
    EC_Renderer** renderers = camera->world->renderers;
    EC_Renderer *renderer = NULL;
    for (int i = 0; i < rendererCount; i++)
    {
        renderer = renderers[i];
        renderer->UpdateBounds(renderer);
        if (!EC_Camera_IsInView(camera, &renderer->bounds))
        {
            printf("Skipping renderer %s\n", renderer->component->entity->name);
            continue;
        }
        renderer->Render(camera, renderer);
    }

    // Blit Image
    XShmPutImage(MainWindow->display, MainWindow->window, MainWindow->gc,
                 camera->image, 0, 0, 0, 0, camera->image->width, camera->image->height, False);
    XSync(MainWindow->display, False);
}

V2_INT Camera_WorldToScreen_V2(EC_Camera* EC_camera, V2* position){
    float x = position->x - EC_camera->position->x+ EC_camera->viewport.x * 0.5;
    float y = position->y - EC_camera->position->y + EC_camera->viewport.y * 0.5;
    return (V2_INT){(int)(x * EC_camera->pixelsPerMeter), (int)(y * EC_camera->pixelsPerMeter)};
}

V2_INT Camera_WorldToScreen_V3(EC_Camera* EC_camera, V3* position){
    float x = position->x - EC_camera->position->x + EC_camera->viewport.x * 0.5;
    float y = position->y - EC_camera->position->y + EC_camera->viewport.y * 0.5;
    return (V2_INT){(int)(x * EC_camera->pixelsPerMeter), (int)(y * EC_camera->pixelsPerMeter)};
}

EC_Camera *EC_Camera_Create(Entity *entity, XImage *image, V2 viewport)
{
    // Create Camera
    EC_Camera *camera = malloc(sizeof(EC_Camera));
    camera->viewport = (V2){viewport.x, viewport.y};
    camera->image = image;
    camera->world = World_Get(entity);
    camera->position = &entity->position;
    // Create Component
    Component *component = Component_Create(camera, entity, EC_T_CAMERA, EC_Camera_Free, NULL, NULL, NULL, Camera_LateUpdate, NULL);
    camera->component = component;
    camera->pixelsPerMeter = 1;
    return camera;
}

void EC_Camera_Free(Component *component)
{
    EC_Camera *camera = (EC_Camera *)component->self;
    free(camera);
}

void RenderCameraDebug(MyWindow *myWindow, EC_Camera *camera)
{
    // Edges
    for (int x = 0; x < camera->viewport.x; x++)
    {
        for (int y = 0; y < 10; y++)
        {
            XDrawPoint(myWindow->display, myWindow->window, myWindow->gc, x, y);
        }
    }
    for (int x = 0; x < camera->viewport.x; x++)
    {
        for (int y = 0; y < 10; y++)
        {
            XDrawPoint(myWindow->display, myWindow->window, myWindow->gc, x, camera->viewport.y - 1 - y);
        }
    }

    for (int y = 0; y < camera->viewport.y; y++)
    {
        for (int x = 0; x < 10; x++)
        {
            XDrawPoint(myWindow->display, myWindow->window, myWindow->gc, camera->viewport.x - 1 - x, y);
        }
    }

    for (int y = 0; y < camera->viewport.y; y++)
    {
        for (int x = 0; x < 10; x++)
        {
            XDrawPoint(myWindow->display, myWindow->window, myWindow->gc, x, y);
        }
    }

    // Center
    int cx = camera->viewport.x * 0.5;
    int cy = camera->viewport.y * 0.5;
    for (int x = cx - 5; x < cx + 5; x++)
    {
        for (int y = cy - 5; y < cy + 5; y++)
        {
            XDrawPoint(myWindow->display, myWindow->window, myWindow->gc, x, y);
        }
    }
    for (int y = 0; y < camera->viewport.y; y++)
    {
        XDrawPoint(myWindow->display, myWindow->window, myWindow->gc, cx, y);
    }
    for (int x = 0; x < camera->viewport.x; x++)
    {
        XDrawPoint(myWindow->display, myWindow->window, myWindow->gc, x, cy);
    }
}