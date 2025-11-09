#ifndef MESH_BVH_H
#define MESH_BVH_H

#include "utilities/math/v3.h"
#include "physics/aabb.h"
#include "rendering/mesh/mesh.h"
#include <stddef.h>
#include <stdbool.h>

// -------------------------
// Types
// -------------------------

typedef struct BVHTriangle BVHTriangle;
typedef struct BVHNode BVHNode;
typedef struct MeshBVH MeshBVH;

/**
 * @brief Represents a triangle in the BVH with precomputed data
 */
struct BVHTriangle
{
    uint32_t indices[3];
    V3 vertices[3];
    V3 normal;
    V3 centroid;
    AABB bounds;
};

/**
 * @brief BVH Node - can be either leaf or internal node
 */
struct BVHNode
{
    AABB bounds;

    // For internal nodes
    struct BVHNode *left;
    struct BVHNode *right;

    // For leaf nodes
    BVHTriangle *triangles;
    uint32_t triangle_count;

    bool is_leaf;
};

/**
 * @brief Complete BVH structure for a mesh
 */
struct MeshBVH
{
    BVHNode *root;
    BVHTriangle *triangles;
    uint32_t triangle_count;
    uint32_t max_triangles_per_leaf;

    // Statistics (for debugging/optimization)
    uint32_t total_nodes;
    uint32_t leaf_nodes;
    uint32_t max_depth;
};

// -------------------------
// Ray-BVH intersection result
// -------------------------

typedef struct
{
    bool hit;
    float distance;
    V3 point;
    V3 normal;
    uint32_t triangle_index;
    BVHTriangle *triangle;
} BVHRaycastHit;

// -------------------------
// Creation & Destruction
// -------------------------

/**
 * @brief Create a BVH for the given mesh
 * @param mesh The mesh to create BVH for
 * @param max_triangles_per_leaf Maximum triangles per leaf node (4-16 recommended)
 * @return Pointer to created BVH or NULL on failure
 */
MeshBVH *MeshBVH_Create(Mesh *mesh, uint32_t max_triangles_per_leaf);

/**
 * @brief Free a BVH and all its resources
 * @param bvh The BVH to free
 */
void MeshBVH_Free(MeshBVH *bvh);

// -------------------------
// Raycast Operations
// -------------------------

/**
 * @brief Perform raycast against BVH
 * @param bvh The BVH to test against
 * @param origin Ray origin in world space
 * @param direction Ray direction (should be normalized)
 * @param max_distance Maximum ray distance
 * @param hit Output structure for hit information
 * @return True if ray hit something, false otherwise
 */
bool MeshBVH_Raycast(MeshBVH *bvh, V3 origin, V3 direction, float max_distance, BVHRaycastHit *hit);

// -------------------------
// Debug & Statistics
// -------------------------

/**
 * @brief Print BVH statistics for debugging
 * @param bvh The BVH to analyze
 */
void MeshBVH_PrintStats(MeshBVH *bvh);

/**
 * @brief Update BVH triangles after mesh transformation
 * @param bvh The BVH to update
 * @param mesh Source mesh
 * @param transform Transformation to apply to vertices
 * @note Call this when the mesh transform changes significantly
 */
void MeshBVH_UpdateTransform(MeshBVH *bvh, Mesh *mesh, V3 worldPos, V3 worldScale,
                             V3 right, V3 up, V3 forward, V3 offset);

#endif // MESH_BVH_H