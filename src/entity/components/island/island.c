#include "entity/components/island/island.h"
#include "entity/components/renderer/rd_island.h"
#include "input/input_manager.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>

static const int INPUT_INTER_UP = 0;
static const int INPUT_INTER_DOWN = 1;

static void EC_Island_Free(Component* component){
    EC_Island* island = (EC_Island*)component->self;
    free(island);
}

static void EC_Island_Update(Component* component){
    EC_Island* island = (EC_Island*)component->self;
    if(island->inputContext->keys[INPUT_INTER_UP].isPressed){
        
    }else if(island->inputContext->keys[INPUT_INTER_DOWN].isPressed){
        
    }
}

static EC_Island* EC_Island_Create(Entity* entity, EC_Renderer* ec_renderer_island){
    EC_Island* ec_island = malloc(sizeof(EC_Island));
    ec_island->ec_renderer_island = ec_renderer_island;
    // Input Context
    int keys[] = {
        XKeysymToKeycode(MainWindow->display, XK_8),
        XKeysymToKeycode(MainWindow->display, XK_2),
    };
    InputContext* inputContext = InputContext_Create("IslandInputContext",
                                                         2,
                                                         keys,
                                                         0,
                                                         NULL,
                                                         0,
                                                         NULL);
    ec_island->inputContext = inputContext;
    // Component
    ec_island->component = Component_Create(ec_island, entity, EC_T_ISLAND, EC_Island_Free, NULL, NULL, EC_Island_Update, NULL, NULL);
    return ec_island;
}

EC_Island* Prefab_Island(Entity* parent, V3 position, float rotation, V2 scale, V2 pivot){
    Entity* entity = Entity_Create(parent, "Island", position, rotation, scale, pivot);
    // Island Renderer
    EC_Renderer* ec_renderer_island = RD_Island_CreateWithRenderer(entity, 3000, 2000, 50, 1);
    // Island
    EC_Island* e_island = EC_Island_Create(entity, ec_renderer_island);
    return e_island;
}