#ifndef GAME_TRIGLE_EC_TRIGLE_H
#define GAME_TRIGLE_EC_TRIGLE_H

// Camera
#include "entity/components/ec_camera/ec_camera.h"
// Entity
#include "entity/entity.h"
// C
#include <stdbool.h>
// Input
#include "input/input_manager.h"
// GUI
#include "entity/components/gui/ec_gui.h"

// ------------------------- 
// Types 
// -------------------------

typedef struct{
    char *name;
    int index;
    Entity *e_space;
    /// @brief This is a child of the Trigle World-Space GUI. Spawning UI elements under this entity will make them appear in the trigle GUI.
    Entity *e_GUIParent;
} TriglePage;

typedef enum {
    TRIGLE_STATE_HIDDEN,
    TRIGLE_STATE_SIDE,
    TRIGLE_STATE_FOCUSED,
} TrigleState;

typedef struct {
    Component *component;
    // ============ State ============ //
    TrigleState state;
    // ============ Trigle Space ============ //
    /// @brief The parent entity of the trigle space - the space containing all trigle objects
    Entity *e_trigleSpace;
    // ============ GUI ============ //
    /// @brief The world space GUI for the trigle
    EC_GUI *ec_gui_worldSpace;
    // ============ Trigle Space Camera ============ //
    EC_Camera *ec_camera;
    // ============ Page Blitting ============ //
    /// @brief The mesh renderer component for the trigle
    EC_MeshRenderer *meshRendererer_cameraTarget;
    // ============ Pages ============ //
    size_t pages_size;
    TriglePage *pages;
    // ============ Input ============ //
    InputListener *inputListener_GAMEPLAY;
    InputListener *inputListener_TRIGLE;
} EC_Trigle;

// ------------------------- 
// Prefabs 
// -------------------------

EC_Trigle *Prefab_Trigle(Entity *parent, char *e_name, TransformSpace TS, V3 position, Quaternion rotation, V3 scale);

// ------------------------- 
// Pages 
// -------------------------

void EC_Trigle_AddPage(EC_Trigle *ec_trigle, char *pageName);
void EC_Trigle_SetActivePage(EC_Trigle *ec_trigle, int pageIndex);
TriglePage *EC_Trigle_GetPage(EC_Trigle *ec_trigle, int pageIndex);

// ------------------------- 
// State Management 
// -------------------------

void EC_Trigle_SetState(EC_Trigle *ec_trigle, TrigleState newState);

#endif