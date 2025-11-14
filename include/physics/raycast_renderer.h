#ifndef PHYSICS_RAYCAST_RENDERER_H
#define PHYSICS_RAYCAST_RENDERER_H

// Entity
#include "entity/entity.h"
// Physics
#include "physics/physics-manager.h"

// ----------------------------------------
// Types 
// ----------------------------------------

typedef struct RaycastRenderer{
    Ray *ray;
    Transform *lineT;
    Transform *endPointT;
    float thickness;
} RaycastRenderer;

// ----------------------------------------
// Creation & Freeing 
// ----------------------------------------

void RaycastRenderer_Init(RaycastRenderer *renderer, Ray *ray, float thickness);
void RaycastRenderer_Free(RaycastRenderer *renderer);

// ----------------------------------------
// Public API 
// ----------------------------------------

void RaycastRenderer_RenderRaycast(RaycastRenderer *renderer, float maxDistance, RaycastHit *hit, uint32_t color);
void RaycastRenderer_HideRaycast(RaycastRenderer *renderer);

#endif