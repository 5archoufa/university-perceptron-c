#ifndef GAME_CREATURE_VISION_H
#define GAME_CREATURE_VISION_H

// Physics
#include "physics/physics-manager.h"
#include "physics/raycast_renderer.h"
// Math
#include "utilities/math/v3.h"
// C
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------
// Types 
// ----------------------------------------

typedef struct CreatureVision
{
    /// @brief Maximum distance the creature can see
    float viewDistance;
    /// @brief In degrees, the total angle of vision
    float viewAngle;
    /// @brief Offset from the origin position to cast rays from
    float offsetFromOrigin;
    /// @brief Bitmask representing which layers the creature can see
    uint32_t layermask;
    /// @brief Number of raycast hits stored
    size_t raycastHits_size;
    /// @brief Pointer to an array of raycast hits
    RaycastHit *raycastHits;
    /// @brief Pointer to an array of rays used for vision
    Ray *rays;
    /// @brief An array of raycast renderers for debugging vision rays
    RaycastRenderer *raycastRenderers;
} CreatureVision;

// ----------------------------------------
// Initialization & Freeing 
// ----------------------------------------

void CreatureVision_Init(CreatureVision *vision, int raycastHits_size, float viewDistance, float viewAngle, float offsetFromOrigin, uint32_t layermask, bool renderRays);
void CreatureVision_Free(CreatureVision *vision);

// ----------------------------------------
// Vision System Functions 
// ----------------------------------------

void CreatureVision_PerformVision(CreatureVision *vision, V3 position, V3 forward);
void CreatureVision_UpdateRaycastRenderers(CreatureVision *vision);

#endif