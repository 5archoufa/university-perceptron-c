#include "game/creature/vision.h"
#include "utilities/math/stupid_math.h"
#include <math.h>

// ----------------------------------------
// Initialization & Freeing
// ----------------------------------------

/// @brief Initializes the CreatureVision structure with the specified parameters.
/// @param vision 
/// @param raycastHits_size 
/// @param viewDistance This is the maximum distance the creature can see. The offsetFromOrigin will be subtracted from this value.
/// @param viewAngle 
/// @param offsetFromOrigin 
/// @param layermask 
/// @param renderRays 
void CreatureVision_Init(CreatureVision *vision, int raycastHits_size, float viewDistance, float viewAngle, float offsetFromOrigin, uint32_t layermask, bool renderRays)
{
    vision->viewDistance = viewDistance - offsetFromOrigin;
    vision->viewAngle = viewAngle;
    vision->offsetFromOrigin = offsetFromOrigin;
    vision->layermask = layermask;
    vision->raycastHits_size = raycastHits_size;
    
    // Allocate raycast hits
    vision->raycastHits = malloc(sizeof(RaycastHit) * raycastHits_size);
    for (size_t i = 0; i < vision->raycastHits_size; i++)
    {
        RaycastHit_Init(&vision->raycastHits[i]);
    }
    
    // Allocate rays
    vision->rays = malloc(sizeof(Ray) * raycastHits_size);
    for (size_t i = 0; i < vision->raycastHits_size; i++)
    {
        vision->rays[i].origin = (V3){0, 0, 0};
        vision->rays[i].direction = (V3){1, 0, 0};
    }
    
    // Raycast Renderers
    if (renderRays)
    {
        vision->raycastRenderers = malloc(sizeof(RaycastRenderer) * raycastHits_size);
        for (size_t i = 0; i < vision->raycastHits_size; i++)
        {
            RaycastRenderer_Init(&vision->raycastRenderers[i], &vision->rays[i], 0.05f);
        }
    }
    else
    {
        vision->raycastRenderers = NULL;
    }
}

void CreatureVision_Free(CreatureVision *vision)
{
    if (vision->raycastHits != NULL)
    {
        free(vision->raycastHits);
        vision->raycastHits = NULL;
    }
    
    if (vision->rays != NULL)
    {
        free(vision->rays);
        vision->rays = NULL;
    }
    
    if (vision->raycastRenderers != NULL)
    {
        for (size_t i = 0; i < vision->raycastHits_size; i++)
        {
            RaycastRenderer_Free(&vision->raycastRenderers[i]);
        }
        free(vision->raycastRenderers);
        vision->raycastRenderers = NULL;
    }
    
    vision->raycastHits_size = 0;
}

// ----------------------------------------
// Vision System Functions
// ----------------------------------------

void CreatureVision_PerformVision(CreatureVision *vision, V3 position, V3 forward)
{
    if (vision->raycastHits_size == 0) return;
    
    // Calculate the angle between each ray
    float angleStep = vision->viewAngle / (vision->raycastHits_size - 1);
    float halfViewAngle = vision->viewAngle * 0.5f;
    
    for (size_t i = 0; i < vision->raycastHits_size; i++)
    {
        // Calculate the angle for this ray relative to forward direction
        float currentAngle = -halfViewAngle + (angleStep * i);
        float angleInRadians = Radians(currentAngle);
        
        // Create ray direction by rotating the forward vector around Y-axis
        V3 rayDirection = {
            forward.x * cosf(angleInRadians) - forward.z * sinf(angleInRadians),
            forward.y,
            forward.x * sinf(angleInRadians) + forward.z * cosf(angleInRadians)
        };
        
        // Normalize the direction
        rayDirection = V3_NORM(rayDirection);
        
        // Update the stored ray
        vision->rays[i].origin = V3_ADD(position, V3_SCALE(rayDirection, vision->offsetFromOrigin));
        vision->rays[i].direction = rayDirection;
        
        // Perform the raycast
        PhysicsManager_Raycast(&vision->rays[i], vision->viewDistance, &vision->raycastHits[i], vision->layermask);
    }
    
    // Update raycast renderer visuals
    if (vision->raycastRenderers != NULL)
    {
        CreatureVision_UpdateRaycastRenderers(vision);
    }
}

void CreatureVision_UpdateRaycastRenderers(CreatureVision *vision)
{
    if (vision->raycastRenderers == NULL) return;
    
    for (size_t i = 0; i < vision->raycastHits_size; i++)
    {
        RaycastHit *hit = &vision->raycastHits[i];
        
        // Determine color based on hit
        uint32_t color = hit->hit ? 0xFF0000FF : 0x00FF00FF; // Red if hit, Green if no hit
        
        // Render the raycast
        RaycastRenderer_RenderRaycast(&vision->raycastRenderers[i], vision->viewDistance, hit, color);
    }
}