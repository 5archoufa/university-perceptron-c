// Game
#include "game/game.h"
// C
#include "physics/raycast_renderer.h"
// Physics
#include "physics/physics-manager.h"
// Mesh
#include "rendering/mesh/mesh.h"

// ----------------------------------------
// Static Variables
// ----------------------------------------

static Mesh *_lineMesh = NULL;
static Mesh *_endPointMesh = NULL;
static bool _isInitialized = false;

// ----------------------------------------
// Static Methods
// ----------------------------------------

static void Game_OnQuit()
{
    if (_lineMesh != NULL)
    {
        Mesh_MarkUnreferenced(_lineMesh);
        _lineMesh = NULL;
    }
    if (_endPointMesh != NULL)
    {
        Mesh_MarkUnreferenced(_endPointMesh);
        _endPointMesh = NULL;
    }
}

// ----------------------------------------
// Initialization & Freeing
// ----------------------------------------

void RaycastRenderer_Init(RaycastRenderer *renderer, Ray *ray, float thickness)
{
    if (!_isInitialized)
    {
        Game_SubscribeOnQuit(Game_OnQuit);
        _lineMesh = Mesh_CreateCube(true, V3_ONE, (V3){0.5f, 0.5f, 0.0f}, 0xFFFFFFFF);
        _endPointMesh = Mesh_CreateSphere(1.0f, 5, 5, (V3){0.5f, 0.5f, 1.0f}, 0xFFFFFFFF, false);
        _isInitialized = true;
    }
    renderer->thickness = thickness;
    renderer->ray = ray;
    // ============ Line ============ //
    // Entity
    Entity *lineEntity = Entity_Create(World_GetRoot(), false, "RaycastRenderer_Line", TS_WORLD, (V3){0, 0, 0}, QUATERNION_IDENTITY, (V3){0.5, 0.5f, 0.0f});
    renderer->lineT = &lineEntity->transform;
    // Mesh Renderer
    EC_MeshRenderer *meshRenderer = EC_MeshRenderer_Create(lineEntity, _lineMesh, (V3){thickness, thickness, 0.0f}, NULL);
    // ============ Hit Endpoint ============ //
    // Entity
    Entity *endPointEntity = Entity_Create(World_GetRoot(), false, "RaycastRenderer_EndPoint", TS_WORLD, (V3){0, 0, 0}, QUATERNION_IDENTITY, V3_SCALE(V3_ONE, 0.3f));
    renderer->endPointT = &endPointEntity->transform;
    // Mesh Renderer
    meshRenderer = EC_MeshRenderer_Create(endPointEntity, _endPointMesh, V3_ONE, NULL);
    // ============ Set Inactive ============ //
    Entity_SetActiveSelf(lineEntity, false);
    Entity_SetActiveSelf(endPointEntity, false);
}

void RaycastRenderer_Free(RaycastRenderer *renderer)
{
    // Destroy Entities
    Entity_Free(renderer->lineT->entity, true);
    Entity_Free(renderer->endPointT->entity, true);
    // Nullify pointers
    renderer->lineT = NULL;
    renderer->endPointT = NULL;
    renderer->ray = NULL;
}

// ----------------------------------------
// Public API
// ----------------------------------------

void RaycastRenderer_RenderRaycast(RaycastRenderer *renderer, float maxDistance, RaycastHit *hit, uint32_t color)
{
    if (!renderer->lineT->entity->isActiveSelf)
    {
        Entity_SetActiveSelf(renderer->lineT->entity, true);
    }
    if (!renderer->endPointT->entity->isActiveSelf)
    {
        Entity_SetActiveSelf(renderer->endPointT->entity, true);
    }

    // Calculate line length and endpoint
    float lineLength;
    V3 endPoint;
    if (hit->hit)
    {
        lineLength = hit->distance;
        endPoint = hit->point;
    }
    else
    {
        lineLength = maxDistance;
        endPoint = V3_ADD(renderer->ray->origin, V3_SCALE(V3_NORM(renderer->ray->direction), maxDistance));
    }

    // Position the line at midpoint between origin and endpoint
    T_WPos_Set(renderer->lineT, renderer->ray->origin);
    // Rotate in direction of ray
    T_WRot_Set(renderer->lineT, Quat_LookDirection(renderer->ray->direction));
    T_WRot_Set(renderer->endPointT, Quat_LookDirection(renderer->ray->direction));
    // Scale the line (thickness on X,Y and length on Z)
    T_LSca_Set(renderer->lineT, (V3){renderer->thickness, renderer->thickness, lineLength});
    // Position the endpoint
    T_WPos_Set(renderer->endPointT, endPoint);
}

void RaycastRenderer_HideRaycast(RaycastRenderer *renderer)
{
    Entity_SetActiveSelf(renderer->lineT->entity, false);
    Entity_SetActiveSelf(renderer->endPointT->entity, false);
}