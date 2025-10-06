#include "entity/components/renderer/renderer.h"
#include "world/world.h"
#include <stdint.h>

// Include opaque alpha (0xFF) in constants so Color_Over treats them as fully opaque by default.
const uint32_t PIXEL_BLACK = 0xFF000000u; // ARGB: opaque black
const uint32_t PIXEL_WHITE = 0xFFFFFFFFu; // ARGB: opaque white
const uint32_t PIXEL_RED = 0xFFFF0000u;   // ARGB: opaque red
const uint32_t PIXEL_GREEN = 0xFF00FF00u; // ARGB: opaque green
const uint32_t PIXEL_BLUE = 0xFF0000FFu;  // ARGB: opaque blue

void EC_Renderer_Free(Component *component)
{
    EC_Renderer *renderer = component->self;
    renderer->renderData_Free(renderer);
    free(renderer);
}

EC_Renderer *EC_Renderer_Create(Entity *entity,
                                void *renderData,
                                void (*Render)(EC_Camera *camera, EC_Renderer *renderer),
                                void (*Render3D)(EC_Camera *camera, EC_Renderer *renderer, Rect unboundedRect, Rect boundedRect),
                                void (*renderData_Free)(EC_Renderer *),
                                void (*UpdateBounds)(EC_Renderer *))
{
    EC_Renderer *renderer = malloc(sizeof(EC_Renderer));
    renderer->renderData = renderData;
    renderer->renderData_Free = renderData_Free;
    renderer->Render = Render;
    renderer->Render3D = Render3D;
    Bounds_Setup(&renderer->bounds, V3_ZERO, V3_ZERO, V3_ZERO);
    renderer->UpdateBounds = UpdateBounds;
    Component *component = Component_Create(renderer, entity, EC_T_RENDERER, EC_Renderer_Free, NULL, NULL, NULL, NULL, NULL);
    renderer->component = component;
    renderer->isVisible = false;
    World_Renderer_Add(renderer);
    // Update Bounds
    UpdateBounds(renderer);
    return renderer;
}

inline uint32_t Color_SetAlpha(uint32_t color, float alpha)
{
    return (color & 0x00FFFFFF) | ((uint32_t)(alpha * 255.0f) << 24);
}

inline uint32_t Color_Over(uint32_t fg, uint32_t bg)
{
    uint32_t af = fg >> 24;
    
    // Early exit for common cases
    if (af == 255) return fg;
    if (af == 0) return bg;
    
    uint32_t ab = bg >> 24;
    
    // Extract and convert to [0, 255*255] range to avoid float
    uint32_t rf = (fg >> 16) & 0xFF;
    uint32_t gf = (fg >> 8) & 0xFF;
    uint32_t bf = fg & 0xFF;
    
    uint32_t rb = (bg >> 16) & 0xFF;
    uint32_t gb = (bg >> 8) & 0xFF;
    uint32_t bb = bg & 0xFF;
    
    // Work in 255-scaled integer space
    uint32_t inv_af = 255 - af;
    uint32_t ab_inv_af = (ab * inv_af + 127) / 255;
    
    uint32_t aout = af + ab_inv_af;
    
    // Compute premultiplied RGB
    uint32_t rout = rf * af + (rb * ab_inv_af);
    uint32_t gout = gf * af + (gb * ab_inv_af);
    uint32_t bout = bf * af + (bb * ab_inv_af);
    
    // Un-premultiply: divide by aout, with rounding
    rout = (rout + aout / 2) / aout;
    gout = (gout + aout / 2) / aout;
    bout = (bout + aout / 2) / aout;
    
    return (aout << 24) | (rout << 16) | (gout << 8) | bout;
}

inline uint32_t Color_Modulate_Grayscale(uint32_t color, float gray)
{
    // Clamp grayscale to [0, 1]
    if (gray < 0.0f) gray = 0.0f;
    if (gray > 1.0f) gray = 1.0f;
    
    uint32_t a = (color >> 24) & 0xFF;
    uint32_t r = (color >> 16) & 0xFF;
    uint32_t g = (color >> 8) & 0xFF;
    uint32_t b = color & 0xFF;
    
    // Multiply each channel by grayscale
    uint8_t gray_val = (uint8_t)(gray * 255.0f + 0.5f);
    
    r = (r * gray_val + 127) / 255;
    g = (g * gray_val + 127) / 255;
    b = (b * gray_val + 127) / 255;
    
    return (a << 24) | (r << 16) | (g << 8) | b;
}
