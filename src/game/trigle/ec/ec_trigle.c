#include "game/trigle/ec/ec_trigle.h"
// Perceptron
#include "perceptron.h"
#include "game/game.h"
// Entity
#include "entity/entity.h"
// Shader
#include "rendering/shader/shader-manager.h"
// Camera
#include "entity/components/ec_camera/ec_camera.h"
// Input
#include "input/input_manager.h"

// -------------------------
// Static Variables
// -------------------------

static ButtonState *INPUT_MOUSE_LEFT = NULL;
static ButtonState *INPUT_MOUSE_RIGHT = NULL;
static MotionState *INPUT_MOUSE_MOTION = NULL;
static KeyState *INPUT_OPEN_TRIGLE = NULL;
static KeyState *INPUT_CLOSE_TRIGLE = NULL;

// -------------------------
// Entity Events
// -------------------------

static void Update(Component *component)
{
    EC_Trigle *ec_trigle = component->self;
    switch (ec_trigle->state)
    {
    case TRIGLE_STATE_HIDDEN:
        if (INPUT_OPEN_TRIGLE->isPressed)
        {
            EC_Trigle_SetState(ec_trigle, TRIGLE_STATE_SIDE);
        }
        break;
    case TRIGLE_STATE_SIDE:
    case TRIGLE_STATE_FOCUSED:
        if (INPUT_CLOSE_TRIGLE->isPressed)
        {
            EC_Trigle_SetState(ec_trigle, TRIGLE_STATE_HIDDEN);
        }
        else
        {
            // Input Listener :: GAMEPLAY
            if (INPUT_MOUSE_LEFT->isDown) // Move camera
            {
                V2 mouseDelta = INPUT_MOUSE_MOTION->delta;
                mouseDelta = V2_NORM(mouseDelta);
                if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
                {
                    Transform *t_camera = &ec_trigle->ec_camera->component->entity->transform;
                    float movementSpeed = 10 * DeltaTime;
                    V3 addition = (V3){-mouseDelta.x * movementSpeed, 0.0f, mouseDelta.y * movementSpeed};
                    T_LPos_Add(t_camera, addition);
                }
            }
        }
        break;
    }
}

// -------------------------
// Creation & Freeing
// -------------------------

static void EC_Trigle_Free(Component *component)
{
    EC_Trigle *ec_trigle = component->self;
    if (!ec_trigle)
        return;

    // Note: InputListeners are owned by the InputContexts and will be freed when contexts are freed
    // Do not call InputListener_Free here to avoid double-free
    Entity_Free(ec_trigle->e_trigleSpace, true);
    free(ec_trigle);
}

static EC_Trigle *EC_Trigle_Create(Entity *entity, Entity *e_trigleSpace, EC_Camera *ec_camera, EC_MeshRenderer *meshRendererer_cameraTarget, EC_GUI *ec_gui_worldSpace)
{
    EC_Trigle *ec_trigle = malloc(sizeof(EC_Trigle));
    ec_trigle->e_trigleSpace = e_trigleSpace;
    ec_trigle->ec_camera = ec_camera;
    ec_trigle->meshRendererer_cameraTarget = meshRendererer_cameraTarget;
    ec_trigle->ec_gui_worldSpace = ec_gui_worldSpace;
    // ============ Pages ============ //
    ec_trigle->pages_size = 0;
    ec_trigle->pages = NULL;
    // ============ Input :: CONTEXT_GAMEPLAY ============ //
    ec_trigle->inputListener_GAMEPLAY = InputContext_AddListener(INPUT_CONTEXT_GAMEPLAY, "Trigle");
    InputMapping *mapping = InputListener_AddMapping(ec_trigle->inputListener_GAMEPLAY, INPUT_CONTEXT_GAMEPLAY->id);
    INPUT_MOUSE_LEFT = InputMapping_AddButton(mapping, GLFW_MOUSE_BUTTON_LEFT);
    INPUT_MOUSE_RIGHT = InputMapping_AddButton(mapping, GLFW_MOUSE_BUTTON_RIGHT);
    INPUT_MOUSE_MOTION = InputMapping_AddMotion(mapping, 0);
    // ============ Input :: CONTEXT_TRIGLE ============ //
    ec_trigle->inputListener_TRIGLE = InputContext_AddListener(INPUT_CONTEXT_TRIGLE, "Trigle");
    mapping = InputListener_AddMapping(ec_trigle->inputListener_TRIGLE, INPUT_CONTEXT_TRIGLE->id);
    INPUT_OPEN_TRIGLE = InputMapping_AddKey(mapping, GLFW_KEY_T);
    INPUT_CLOSE_TRIGLE = InputMapping_AddKey(mapping, GLFW_KEY_ESCAPE);
    // ============ Component ============ //
    ec_trigle->component = Component_Create(ec_trigle, entity, EC_T_GAME_TRIGLE,
                                            EC_Trigle_Free, NULL, NULL, Update, NULL, NULL);
    return ec_trigle;
}

// -------------------------
// State
// -------------------------

void EC_Trigle_SetState(EC_Trigle *ec_trigle, TrigleState newState)
{
    if (newState == ec_trigle->state)
    {
        return;
    }

    switch (newState)
    {
    case TRIGLE_STATE_HIDDEN:
        if (EC_StateMachine_TrySetState(SM_GAME, SM_STATE_GAMEPLAY, 0, NULL))
        {
            Entity_SetActiveSelf(ec_trigle->e_trigleSpace, false);
        }
        break;
    case TRIGLE_STATE_SIDE:
        if (EC_StateMachine_TrySetState(SM_GAME, SM_STATE_GAMEPLAY, 0, NULL))
        {
            Entity_SetActiveSelf(ec_trigle->e_trigleSpace, true);
        }
        break;
    case TRIGLE_STATE_FOCUSED:
        if (EC_StateMachine_TrySetState(SM_GAME, SM_STATE_TRIGLE, 0, NULL))
        {
            Entity_SetActiveSelf(ec_trigle->e_trigleSpace, true);
        }
        break;
    }
    ec_trigle->state = newState;
}

// -------------------------
// Prefabs
// -------------------------

EC_Trigle *Prefab_Trigle(Entity *parent, char *e_name, TransformSpace TS, V3 position, Quaternion rotation, V3 scale)
{
    // Entity
    Entity *e_trigle = Entity_Create(parent, false, e_name, TS, position, rotation, scale);
    // Book Mesh
    V3 meshScale_book = {1.5f, 1.0f, 0.2f};
    Mesh *mesh_book = Mesh_CreateCube(false, meshScale_book, (V3){0.5, 0.5, 1}, 0xff2a3f6d); // #6d3f2aff
    EC_MeshRenderer *meshRenderer_book = EC_MeshRenderer_Create(e_trigle, mesh_book, meshScale_book, NULL);
    // Create Trigle Space
    Entity *worldRoot = World_GetRoot();
    Entity *e_trigleSpace = Entity_Create(worldRoot, true, "Trigle Space", TS_WORLD, V3_ZERO, QUATERNION_IDENTITY, V3_ONE);
    EC_GUI *ec_gui_worldSpace = EC_GUI_Create(e_trigleSpace, GUI_RENDER_MODE_WORLD_SPACE, (V2){800 * 1.28, 600 * 1.28});
    Entity_SetLayer(e_trigleSpace, E_LAYER_TRIGLE, true);
    // Create Camera
    V2 viewport = {512 * 1.5f, 512};
    RenderTarget *renderTarget = RenderTarget_Create(viewport.x, viewport.y);
    Shader *shader_cameraBlit = ShaderManager_Get(SHADER_QUAD_BLIT);
    EC_Camera *ec_camera = Prefab_RenderTargetCamera(e_trigleSpace,
                                                     "Trigle Camera",
                                                     TS_LOCAL,
                                                     (V3){0, 10, 0},
                                                     Quat_FromEuler((V3){90.0f, 0, 0}),
                                                     V3_ONE,
                                                     viewport,
                                                     renderTarget,
                                                     shader_cameraBlit,
                                                     RGBA_FromHexaRGBA(0x7b4a33ff) // #7b4a33ff
    );
    EC_Camera_ClearCullingMask(ec_camera);
    EC_Camera_AddToCullingMask(ec_camera, E_LAYER_TRIGLE);

    // Camera Target Quad (the display quad in main world)
    Shader *shader_triglePage = ShaderManager_Get(SHADER_TRIGLE_PAGE);
    Material *material_triglePage = Material_Create(shader_triglePage, 0, NULL);
    // Hook the camera texture to the trigle page material BEFORE creating the mesh renderer
    Material_SetTexture(material_triglePage, "cameraTexture", renderTarget->colorTexture);
    Entity *e_cameraBlitTarget = Entity_Create(e_trigle, true, "Camera Blit Target", TS_LOCAL, (V3){0, 0, 0.001}, QUATERNION_IDENTITY, V3_ONE);
    EC_MeshRenderer *meshRenderer_cameraTarget = Prefab_Quad(e_cameraBlitTarget,
                                                             false,
                                                             TS_LOCAL,
                                                             V3_ZERO,
                                                             Quat_FromEuler((V3){0, 180, 0}),
                                                             V3_ONE,
                                                             (V2){meshScale_book.x - 0.04f, meshScale_book.y - 0.02f},
                                                             0xFFFFFFFF,
                                                             material_triglePage);

    return EC_Trigle_Create(e_trigle, e_trigleSpace, ec_camera, meshRenderer_cameraTarget, ec_gui_worldSpace);
}

// -------------------------
// Pages
// -------------------------

void EC_Trigle_SetActivePage(EC_Trigle *ec_trigle, int pageIndex)
{
    for (int i = 0; i < ec_trigle->pages_size; i++)
    {
        TriglePage *page = &ec_trigle->pages[i];
        if (i == pageIndex)
        {
            Entity_SetActiveSelf(page->e_space, true);
            Entity_SetActiveSelf(page->e_GUIParent, true);
        }
        else
        {
            Entity_SetActiveSelf(page->e_space, false);
            Entity_SetActiveSelf(page->e_GUIParent, false);
        }
    }
}

void EC_Trigle_AddPage(EC_Trigle *ec_trigle, char *pageName)
{
    // Add new page to array
    ec_trigle->pages_size++;
    ec_trigle->pages = realloc(ec_trigle->pages, ec_trigle->pages_size * sizeof(TriglePage));
    TriglePage *page = &ec_trigle->pages[ec_trigle->pages_size - 1];
    // Page Name
    page->name = strdup(pageName);
    // Page Index
    page->index = ec_trigle->pages_size - 1;
    // Page space
    Entity *e_space = Entity_Create(ec_trigle->e_trigleSpace, true, pageName, TS_LOCAL, V3_ZERO, QUATERNION_IDENTITY, V3_ONE);
    Entity *e_GUIParent = Entity_Create(ec_trigle->ec_gui_worldSpace->component->entity, true, "Page GUI Parent", TS_LOCAL, V3_ZERO, QUATERNION_IDENTITY, V3_ONE);
    page->e_space = e_space;
    page->e_GUIParent = e_GUIParent;
}

TriglePage *EC_Trigle_GetPage(EC_Trigle *ec_trigle, int pageIndex)
{
    if (pageIndex < 0 || pageIndex >= ec_trigle->pages_size)
    {
        return NULL;
    }
    return &ec_trigle->pages[pageIndex];
}