#include "physics/mesh_bvh.h"
#include "logging/logger.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

// -------------------------
// Static Variables
// -------------------------

static LogConfig _logConfig = {"MeshBVH", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

// -------------------------
// Helper Functions
// -------------------------

/**
 * @brief Calculate AABB for a set of triangles
 */
static AABB CalculateTriangleAABB(BVHTriangle *triangles, uint32_t count)
{
    if (count == 0)
    {
        AABB empty = {{FLT_MAX, FLT_MAX, FLT_MAX}, {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
        return empty;
    }
    
    AABB result = triangles[0].bounds;
    
    for (uint32_t i = 1; i < count; i++)
    {
        AABB tri_bounds = triangles[i].bounds;
        
        // Expand to include this triangle
        if (tri_bounds.min.x < result.min.x) result.min.x = tri_bounds.min.x;
        if (tri_bounds.min.y < result.min.y) result.min.y = tri_bounds.min.y;
        if (tri_bounds.min.z < result.min.z) result.min.z = tri_bounds.min.z;
        
        if (tri_bounds.max.x > result.max.x) result.max.x = tri_bounds.max.x;
        if (tri_bounds.max.y > result.max.y) result.max.y = tri_bounds.max.y;
        if (tri_bounds.max.z > result.max.z) result.max.z = tri_bounds.max.z;
    }
    
    return result;
}

/**
 * @brief Comparison function for sorting triangles by centroid
 */
static int CompareTrianglesCentroid(void *axis_ptr, const void *a, const void *b)
{
    int axis = *(int*)axis_ptr;
    const BVHTriangle *tri_a = (const BVHTriangle*)a;
    const BVHTriangle *tri_b = (const BVHTriangle*)b;
    
    float val_a, val_b;
    switch (axis)
    {
        case 0: val_a = tri_a->centroid.x; val_b = tri_b->centroid.x; break;
        case 1: val_a = tri_a->centroid.y; val_b = tri_b->centroid.y; break;
        case 2: val_a = tri_a->centroid.z; val_b = tri_b->centroid.z; break;
        default: return 0;
    }
    
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

/**
 * @brief Sort triangles by centroid along specified axis
 */
static void SortTriangles(BVHTriangle *triangles, uint32_t count, int axis)
{
    // Use a simple insertion sort for small arrays, qsort for larger ones
    if (count <= 16)
    {
        // Insertion sort
        for (uint32_t i = 1; i < count; i++)
        {
            BVHTriangle key = triangles[i];
            int j = i - 1;
            
            float key_val, cmp_val;
            switch (axis)
            {
                case 0: key_val = key.centroid.x; break;
                case 1: key_val = key.centroid.y; break;
                case 2: key_val = key.centroid.z; break;
                default: continue;
            }
            
            while (j >= 0)
            {
                switch (axis)
                {
                    case 0: cmp_val = triangles[j].centroid.x; break;
                    case 1: cmp_val = triangles[j].centroid.y; break;
                    case 2: cmp_val = triangles[j].centroid.z; break;
                    default: cmp_val = 0; break;
                }
                
                if (cmp_val <= key_val) break;
                
                triangles[j + 1] = triangles[j];
                j--;
            }
            triangles[j + 1] = key;
        }
    }
    else
    {
        // For larger arrays, use qsort_r (Windows/MSVC compatible)
        #ifdef _WIN32
        qsort_s(triangles, count, sizeof(BVHTriangle), CompareTrianglesCentroid, &axis);
        #else
        qsort_r(triangles, count, sizeof(BVHTriangle), CompareTrianglesCentroid, &axis);
        #endif
    }
}

/**
 * @brief Find best split axis for BVH construction
 */
static int FindBestSplitAxis(AABB bounds)
{
    V3 extent = V3_SUB(bounds.max, bounds.min);
    
    if (extent.x >= extent.y && extent.x >= extent.z)
        return 0; // X axis
    else if (extent.y >= extent.z)
        return 1; // Y axis
    else
        return 2; // Z axis
}

/**
 * @brief Build BVH node recursively
 */
static BVHNode *BuildBVHNode(BVHTriangle *triangles, uint32_t count, 
                             uint32_t max_triangles_per_leaf, uint32_t depth,
                             uint32_t *total_nodes, uint32_t *leaf_nodes, uint32_t *max_depth)
{
    (*total_nodes)++;
    if (depth > *max_depth)
        *max_depth = depth;
    
    BVHNode *node = malloc(sizeof(BVHNode));
    memset(node, 0, sizeof(BVHNode));
    
    // Calculate bounding box for this node
    node->bounds = CalculateTriangleAABB(triangles, count);
    
    // Check if we should create a leaf
    if (count <= max_triangles_per_leaf)
    {
        // Create leaf node
        node->is_leaf = true;
        node->triangle_count = count;
        node->triangles = malloc(sizeof(BVHTriangle) * count);
        memcpy(node->triangles, triangles, sizeof(BVHTriangle) * count);
        (*leaf_nodes)++;
        return node;
    }
    
    // Create internal node - find best split
    int split_axis = FindBestSplitAxis(node->bounds);
    
    // Sort triangles along split axis
    SortTriangles(triangles, count, split_axis);
    
    // Split in the middle
    uint32_t mid = count / 2;
    
    // Recursively build children
    node->left = BuildBVHNode(triangles, mid, max_triangles_per_leaf, 
                              depth + 1, total_nodes, leaf_nodes, max_depth);
    node->right = BuildBVHNode(triangles + mid, count - mid, max_triangles_per_leaf,
                               depth + 1, total_nodes, leaf_nodes, max_depth);
    
    return node;
}

/**
 * @brief Ray-triangle intersection using Möller-Trumbore algorithm
 */
static bool RayIntersectsTriangle(V3 rayOrigin, V3 rayDir, 
                                  V3 v0, V3 v1, V3 v2,
                                  float *outDistance, V3 *outNormal)
{
    const float EPSILON = 0.0000001f;
    
    // Calculate triangle edges
    V3 edge1 = V3_SUB(v1, v0);
    V3 edge2 = V3_SUB(v2, v0);
    
    // Begin calculating determinant
    V3 h = V3_CROSS(rayDir, edge2);
    float a = V3_DOT(edge1, h);
    
    // Ray is parallel to triangle
    if (fabsf(a) < EPSILON)
        return false;
    
    float f = 1.0f / a;
    V3 s = V3_SUB(rayOrigin, v0);
    float u = f * V3_DOT(s, h);
    
    if (u < 0.0f || u > 1.0f)
        return false;
    
    V3 q = V3_CROSS(s, edge1);
    float v = f * V3_DOT(rayDir, q);
    
    if (v < 0.0f || u + v > 1.0f)
        return false;
    
    float t = f * V3_DOT(edge2, q);
    
    if (t > EPSILON)
    {
        *outDistance = t;
        *outNormal = V3_NORM(V3_CROSS(edge1, edge2));
        return true;
    }
    
    return false;
}

/**
 * @brief Ray-AABB intersection test
 */
static bool RayIntersectsAABB(V3 origin, V3 direction, AABB aabb, float max_distance)
{
    V3 invDir = {
        (fabsf(direction.x) > 0.0001f) ? 1.0f / direction.x : FLT_MAX,
        (fabsf(direction.y) > 0.0001f) ? 1.0f / direction.y : FLT_MAX,
        (fabsf(direction.z) > 0.0001f) ? 1.0f / direction.z : FLT_MAX
    };

    float t1 = (aabb.min.x - origin.x) * invDir.x;
    float t2 = (aabb.max.x - origin.x) * invDir.x;
    float t3 = (aabb.min.y - origin.y) * invDir.y;
    float t4 = (aabb.max.y - origin.y) * invDir.y;
    float t5 = (aabb.min.z - origin.z) * invDir.z;
    float t6 = (aabb.max.z - origin.z) * invDir.z;

    float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
    float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

    return (tmax >= tmin && tmax >= 0.0f && tmin <= max_distance);
}

/**
 * @brief Recursive BVH traversal for raycast
 */
static bool TraverseBVH(BVHNode *node, V3 origin, V3 direction, float max_distance, BVHRaycastHit *best_hit)
{
    if (!node)
        return false;
    
    // Early rejection - test ray against node bounds
    if (!RayIntersectsAABB(origin, direction, node->bounds, max_distance))
        return false;
    
    if (node->is_leaf)
    {
        // Test ray against all triangles in leaf
        bool hit_any = false;
        
        for (uint32_t i = 0; i < node->triangle_count; i++)
        {
            BVHTriangle *triangle = &node->triangles[i];
            
            float distance;
            V3 normal;
            if (RayIntersectsTriangle(origin, direction, 
                                      triangle->vertices[0], triangle->vertices[1], triangle->vertices[2],
                                      &distance, &normal))
            {
                if (distance < best_hit->distance)
                {
                    best_hit->hit = true;
                    best_hit->distance = distance;
                    best_hit->point = V3_ADD(origin, V3_SCALE(direction, distance));
                    best_hit->normal = normal;
                    best_hit->triangle = triangle;
                    best_hit->triangle_index = i; // Note: This is local to the leaf
                    hit_any = true;
                }
            }
        }
        
        return hit_any;
    }
    else
    {
        // Internal node - traverse children
        bool hit_left = TraverseBVH(node->left, origin, direction, best_hit->distance, best_hit);
        bool hit_right = TraverseBVH(node->right, origin, direction, best_hit->distance, best_hit);
        
        return hit_left || hit_right;
    }
}

// -------------------------
// Public Functions
// -------------------------

MeshBVH *MeshBVH_Create(Mesh *mesh, uint32_t max_triangles_per_leaf)
{
    if (!mesh || !mesh->vertices)
    {
        LogError(&_logConfig, "Invalid mesh provided to MeshBVH_Create");
        return NULL;
    }
    
    // Calculate triangle count
    uint32_t triangle_count;
    if (mesh->indices && mesh->indices_size > 0)
    {
        triangle_count = mesh->indices_size / 3;
    }
    else
    {
        triangle_count = mesh->vertices_size / 3;
    }
    
    if (triangle_count == 0)
    {
        LogWarning(&_logConfig, "Mesh has no triangles");
        return NULL;
    }
    
    Log(&_logConfig, "Creating BVH for mesh with %u triangles", triangle_count);
    
    MeshBVH *bvh = malloc(sizeof(MeshBVH));
    memset(bvh, 0, sizeof(MeshBVH));
    
    bvh->triangle_count = triangle_count;
    bvh->max_triangles_per_leaf = max_triangles_per_leaf;
    bvh->triangles = malloc(sizeof(BVHTriangle) * triangle_count);
    
    // Build triangle list with precomputed data
    for (uint32_t i = 0; i < triangle_count; i++)
    {
        BVHTriangle *tri = &bvh->triangles[i];
        
        // Get triangle vertices
        if (mesh->indices && mesh->indices_size > 0)
        {
            tri->indices[0] = mesh->indices[i * 3];
            tri->indices[1] = mesh->indices[i * 3 + 1];
            tri->indices[2] = mesh->indices[i * 3 + 2];
            
            tri->vertices[0] = mesh->vertices[tri->indices[0]].position;
            tri->vertices[1] = mesh->vertices[tri->indices[1]].position;
            tri->vertices[2] = mesh->vertices[tri->indices[2]].position;
        }
        else
        {
            tri->indices[0] = i * 3;
            tri->indices[1] = i * 3 + 1;
            tri->indices[2] = i * 3 + 2;
            
            tri->vertices[0] = mesh->vertices[i * 3].position;
            tri->vertices[1] = mesh->vertices[i * 3 + 1].position;
            tri->vertices[2] = mesh->vertices[i * 3 + 2].position;
        }
        
        // Calculate centroid
        tri->centroid = V3_SCALE(V3_ADD(V3_ADD(tri->vertices[0], tri->vertices[1]), tri->vertices[2]), 1.0f / 3.0f);
        
        // Calculate normal
        V3 edge1 = V3_SUB(tri->vertices[1], tri->vertices[0]);
        V3 edge2 = V3_SUB(tri->vertices[2], tri->vertices[0]);
        tri->normal = V3_NORM(V3_CROSS(edge1, edge2));
        
        // Calculate bounds
        tri->bounds.min = tri->vertices[0];
        tri->bounds.max = tri->vertices[0];
        
        for (int j = 1; j < 3; j++)
        {
            if (tri->vertices[j].x < tri->bounds.min.x) tri->bounds.min.x = tri->vertices[j].x;
            if (tri->vertices[j].y < tri->bounds.min.y) tri->bounds.min.y = tri->vertices[j].y;
            if (tri->vertices[j].z < tri->bounds.min.z) tri->bounds.min.z = tri->vertices[j].z;
            
            if (tri->vertices[j].x > tri->bounds.max.x) tri->bounds.max.x = tri->vertices[j].x;
            if (tri->vertices[j].y > tri->bounds.max.y) tri->bounds.max.y = tri->vertices[j].y;
            if (tri->vertices[j].z > tri->bounds.max.z) tri->bounds.max.z = tri->vertices[j].z;
        }
    }
    
    // Build BVH tree
    bvh->total_nodes = 0;
    bvh->leaf_nodes = 0;
    bvh->max_depth = 0;
    
    bvh->root = BuildBVHNode(bvh->triangles, triangle_count, max_triangles_per_leaf, 0,
                             &bvh->total_nodes, &bvh->leaf_nodes, &bvh->max_depth);
    
    LogSuccess(&_logConfig, "BVH created: %u nodes (%u leaves), max depth %u", 
               bvh->total_nodes, bvh->leaf_nodes, bvh->max_depth);
    
    return bvh;
}

static void FreeBVHNode(BVHNode *node)
{
    if (!node)
        return;
    
    if (node->is_leaf)
    {
        free(node->triangles);
    }
    else
    {
        FreeBVHNode(node->left);
        FreeBVHNode(node->right);
    }
    
    free(node);
}

void MeshBVH_Free(MeshBVH *bvh)
{
    if (!bvh)
        return;
    
    FreeBVHNode(bvh->root);
    free(bvh->triangles);
    free(bvh);
    
    LogSuccess(&_logConfig, "BVH freed");
}

bool MeshBVH_Raycast(MeshBVH *bvh, V3 origin, V3 direction, float max_distance, BVHRaycastHit *hit)
{
    if (!bvh || !bvh->root || !hit)
        return false;
    
    // Initialize hit result
    hit->hit = false;
    hit->distance = max_distance;
    hit->triangle = NULL;
    hit->triangle_index = 0;
    
    // Ensure direction is normalized
    direction = V3_NORM(direction);
    
    // Traverse BVH
    return TraverseBVH(bvh->root, origin, direction, max_distance, hit);
}

void MeshBVH_PrintStats(MeshBVH *bvh)
{
    if (!bvh)
    {
        LogError(&_logConfig, "Cannot print stats for NULL BVH");
        return;
    }
    
    Log(&_logConfig, "BVH Statistics:");
    Log(&_logConfig, "  Triangles: %u", bvh->triangle_count);
    Log(&_logConfig, "  Total nodes: %u", bvh->total_nodes);
    Log(&_logConfig, "  Leaf nodes: %u", bvh->leaf_nodes);
    Log(&_logConfig, "  Max depth: %u", bvh->max_depth);
    Log(&_logConfig, "  Max triangles per leaf: %u", bvh->max_triangles_per_leaf);
    
    float avg_triangles_per_leaf = bvh->leaf_nodes > 0 ? (float)bvh->triangle_count / bvh->leaf_nodes : 0.0f;
    Log(&_logConfig, "  Avg triangles per leaf: %.2f", avg_triangles_per_leaf);
}

void MeshBVH_UpdateTransform(MeshBVH *bvh, Mesh *mesh, V3 worldPos, V3 worldScale, 
                            V3 right, V3 up, V3 forward, V3 offset)
{
    if (!bvh || !mesh)
        return;
    
    // Transform offset into world space
    V3 offsetWorld = {
        offset.x * right.x + offset.y * up.x + offset.z * forward.x,
        offset.x * right.y + offset.y * up.y + offset.z * forward.y,
        offset.x * right.z + offset.y * up.z + offset.z * forward.z
    };
    
    // Update all triangle vertices to world space
    for (uint32_t i = 0; i < bvh->triangle_count; i++)
    {
        BVHTriangle *tri = &bvh->triangles[i];
        
        // Get original local vertices
        V3 localVerts[3];
        if (mesh->indices && mesh->indices_size > 0)
        {
            localVerts[0] = mesh->vertices[tri->indices[0]].position;
            localVerts[1] = mesh->vertices[tri->indices[1]].position;
            localVerts[2] = mesh->vertices[tri->indices[2]].position;
        }
        else
        {
            localVerts[0] = mesh->vertices[i * 3].position;
            localVerts[1] = mesh->vertices[i * 3 + 1].position;
            localVerts[2] = mesh->vertices[i * 3 + 2].position;
        }
        
        // Transform to world space
        for (int j = 0; j < 3; j++)
        {
            // Apply scale
            V3 scaled = V3_MUL(localVerts[j], worldScale);
            
            // Apply rotation
            tri->vertices[j] = (V3){
                scaled.x * right.x + scaled.y * up.x + scaled.z * forward.x + worldPos.x,
                scaled.x * right.y + scaled.y * up.y + scaled.z * forward.y + worldPos.y,
                scaled.x * right.z + scaled.y * up.z + scaled.z * forward.z + worldPos.z
            };
            
            // Apply offset
            tri->vertices[j] = V3_ADD(tri->vertices[j], offsetWorld);
        }
        
        // Recalculate derived data
        tri->centroid = V3_SCALE(V3_ADD(V3_ADD(tri->vertices[0], tri->vertices[1]), tri->vertices[2]), 1.0f / 3.0f);
        
        V3 edge1 = V3_SUB(tri->vertices[1], tri->vertices[0]);
        V3 edge2 = V3_SUB(tri->vertices[2], tri->vertices[0]);
        tri->normal = V3_NORM(V3_CROSS(edge1, edge2));
        
        // Recalculate bounds
        tri->bounds.min = tri->vertices[0];
        tri->bounds.max = tri->vertices[0];
        
        for (int j = 1; j < 3; j++)
        {
            if (tri->vertices[j].x < tri->bounds.min.x) tri->bounds.min.x = tri->vertices[j].x;
            if (tri->vertices[j].y < tri->bounds.min.y) tri->bounds.min.y = tri->vertices[j].y;
            if (tri->vertices[j].z < tri->bounds.min.z) tri->bounds.min.z = tri->vertices[j].z;
            
            if (tri->vertices[j].x > tri->bounds.max.x) tri->bounds.max.x = tri->vertices[j].x;
            if (tri->vertices[j].y > tri->bounds.max.y) tri->bounds.max.y = tri->vertices[j].y;
            if (tri->vertices[j].z > tri->bounds.max.z) tri->bounds.max.z = tri->vertices[j].z;
        }
    }
    
    // TODO: Rebuild BVH tree with updated triangle data
    // For now, we just update triangle data. For full optimization,
    // you'd want to rebuild the tree structure as well.
}