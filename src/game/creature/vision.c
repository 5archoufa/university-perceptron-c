#include "game/creature/vision.h"
#include "utilities/math/stupid_math.h"
#include <math.h>

// ----------------------------------------
// Initialization & Freeing
// ----------------------------------------

/// @brief Initializes the CreatureVision structure with the specified parameters.
/// @param vision
/// @param distribution V2_INT where x = columns, y = rows of rays
/// @param viewDistance This is the maximum distance the creature can see. The offsetFromOrigin will be subtracted from this value.
/// @param fov Field of view in x (horizontal) and y (vertical) dimensions
/// @param offsetFromOrigin
/// @param layermask
/// @param renderRays
void CreatureVision_Init(CreatureVision *vision, V2_INT distribution, float viewDistance, V2 fov, float fov_yOffset, float offsetFromOrigin, uint32_t layermask, bool renderRays)
{
    vision->viewDistance = viewDistance - offsetFromOrigin;
    vision->fov = fov;
    vision->fov_yOffset = fov_yOffset;
    vision->distribution = distribution;
    vision->offsetFromOrigin = offsetFromOrigin;
    vision->layermask = layermask;
    size_t raycastHits_size = distribution.x * distribution.y;
    vision->raycastHits_size = raycastHits_size;

    // Allocate raycast hits
    vision->raycastHits = malloc(sizeof(RaycastHit) * raycastHits_size);
    for (size_t i = 0; i < raycastHits_size; i++)
    {
        RaycastHit_Init(&vision->raycastHits[i]);
    }

    // Allocate rays
    vision->rays = malloc(sizeof(Ray) * raycastHits_size);
    for (size_t i = 0; i < raycastHits_size; i++)
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
    if (vision->raycastHits_size == 0)
        return;

    // Normalize the forward vector
    V3 forwardNorm = V3_NORM(forward);
    
    // Build a coordinate system from the forward vector
    V3 worldUp = {0, 1, 0};
    
    // Calculate right vector (perpendicular to forward and up)
    V3 right = V3_NORM(V3_CROSS(worldUp, forwardNorm));
    
    // Recalculate up vector to ensure orthogonality
    V3 up = V3_NORM(V3_CROSS(forwardNorm, right));

    // Calculate the angle between each ray in both dimensions
    V2 angleStep = {
        .x = (vision->distribution.x > 1) ? vision->fov.x / (vision->distribution.x - 1) : 0.0f,
        .y = (vision->distribution.y > 1) ? vision->fov.y / (vision->distribution.y - 1) : 0.0f
    };
    
    V2 halfViewAngle = {
        .x = vision->fov.x * 0.5f,
        .y = vision->fov.y * 0.5f
    };

    // Iterate through rows (y) and columns (x)
    for (size_t row = 0; row < vision->distribution.y; row++)
    {
        for (size_t col = 0; col < vision->distribution.x; col++)
        {
            // Calculate 1D array index from 2D position
            size_t index = row * vision->distribution.x + col;

            // Calculate the angle for this ray relative to forward direction
            V2 currentAngle = {
                .x = -halfViewAngle.x + (angleStep.x * col),  // Horizontal angle
                .y = -halfViewAngle.y + (angleStep.y * row)   // Vertical angle
            };
            
            V2 angleInRadians = {
                .x = Radians(currentAngle.x),
                .y = Radians(currentAngle.y)
            };

            // Build ray direction in local space
            // Start with forward direction, then rotate
            // Horizontal rotation around the up axis
            V3 tempDir = forwardNorm;
            V3 horizontalRotated = {
                tempDir.x * cosf(angleInRadians.x) + right.x * sinf(angleInRadians.x),
                tempDir.y * cosf(angleInRadians.x) + right.y * sinf(angleInRadians.x),
                tempDir.z * cosf(angleInRadians.x) + right.z * sinf(angleInRadians.x)
            };
            
            // Vertical rotation around the right axis
            V3 rayDirection = {
                horizontalRotated.x * cosf(angleInRadians.y) + up.x * sinf(angleInRadians.y),
                horizontalRotated.y * cosf(angleInRadians.y) + up.y * sinf(angleInRadians.y),
                horizontalRotated.z * cosf(angleInRadians.y) + up.z * sinf(angleInRadians.y)
            };

            // Normalize the direction
            rayDirection = V3_NORM(rayDirection);

            // Update the stored ray
            vision->rays[index].origin = V3_ADD(position, V3_SCALE(rayDirection, vision->offsetFromOrigin));
            vision->rays[index].direction = rayDirection;

            // Perform the raycast
            PhysicsManager_Raycast(&vision->rays[index], vision->viewDistance, &vision->raycastHits[index], vision->layermask);
        }
    }

    // Update raycast renderer visuals
    if (vision->raycastRenderers != NULL)
    {
        CreatureVision_UpdateRaycastRenderers(vision);
    }
}

void CreatureVision_UpdateRaycastRenderers(CreatureVision *vision)
{
    if (vision->raycastRenderers == NULL)
        return;

    for (size_t i = 0; i < vision->raycastHits_size; i++)
    {
        RaycastHit *hit = &vision->raycastHits[i];

        // Determine color based on hit
        uint32_t color = hit->hit ? 0xFF0000FF : 0x00FF00FF; // Red if hit, Green if no hit

        // Render the raycast
        RaycastRenderer_RenderRaycast(&vision->raycastRenderers[i], vision->viewDistance, hit, color);
    }
}