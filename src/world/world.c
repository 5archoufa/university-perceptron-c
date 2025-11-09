#include "world/world.h"
// Perceptron
#include "perceptron.h"
// Entity
#include "entity/entity.h"
// Logging
#include "logging/logger.h"
// Lighting
#include "entity/components/lighting/ec_light.h"
// Shader
#include "rendering/shader/shader.h"
#include "rendering/shader/shader-manager.h"

static LogConfig _logConfig = {"World", LOG_LEVEL_INFO, LOG_COLOR_BLUE};
static World **_worlds = NULL;
static int _worldCount = 0;
/// @brief Pointer to the currently selected world
static World *_world = NULL;
static int LeapYear_MonthLimits[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int CommonYear_MonthLimits[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// -------------------------
// Time Utilities
// -------------------------

/// @brief Get the size of the buffer needed to hold a date/time string.
size_t *DateTime_GetStringBufferSize()
{
    static size_t bufferSize = 20; // "YYYY-MM-DD HH:MM:SS" + null terminator
    return &bufferSize;
}

void DateTime_ToString(SimulatedDateTime *dt, char *buffer, size_t bufferSize)
{
    snprintf(buffer, bufferSize, "%04d-%02d-%02d %02d:%02d:%05.2f",
             dt->year,
             dt->month,
             dt->day,
             dt->hours,
             dt->minutes,
             dt->seconds);
}

SimulatedDateTime *World_GetDateTime()
{
    return &_world->dateTime;
}

inline static bool DateTime_IsLeapYear(int year)
{
    // Leap year if divisible by 4, not divisible by 100 unless also divisible by 400
    if (year % 4 != 0)
        return false;
    else if (year % 100 != 0)
        return true;
    else if (year % 400 != 0)
        return false;
    else
        return true;
}

inline static int DateTime_GetMonthLimit(int month, int year)
{
    // month is 1-based
    if (DateTime_IsLeapYear(year))
        return LeapYear_MonthLimits[month - 1];
    else
        return CommonYear_MonthLimits[month - 1];
}

inline static void DateTime_UpdateNormalized(SimulatedDateTime *dt)
{
    // ============ Time ============ //
    dt->normalizedSeconds = dt->seconds / 60.0f;
    dt->normalizedMinutes = (dt->minutes + dt->normalizedSeconds) / 60.0f;
    dt->normalizedHours = (dt->hours + dt->normalizedMinutes) / 24.0f;

    // ============ Date ============ //
    int monthLimit = DateTime_GetMonthLimit(dt->month, dt->year);
    dt->normalizedDay = ((dt->day - 1) + dt->normalizedHours) / monthLimit;
    dt->normalizedMonth = ((dt->month - 1) + dt->normalizedDay) / 12.0f;
}

inline static void DateTime_AddDays(SimulatedDateTime *dateTime, int days)
{
    dateTime->day += days;

    int monthLimit = DateTime_GetMonthLimit(dateTime->month, dateTime->year);

    // Handle overflow of days across months and years
    while (dateTime->day > monthLimit)
    {
        dateTime->day -= monthLimit;
        dateTime->month += 1;

        if (dateTime->month > 12)
        {
            dateTime->month = 1;
            dateTime->year += 1;
        }

        monthLimit = DateTime_GetMonthLimit(dateTime->month, dateTime->year);
    }

    DateTime_UpdateNormalized(dateTime);
}

inline static void DateTime_AddHours(SimulatedDateTime *dateTime, int hours)
{
    dateTime->hours += hours;

    if (dateTime->hours >= 24)
    {
        int addDays = dateTime->hours / 24;
        dateTime->hours %= 24;
        DateTime_AddDays(dateTime, addDays);
    }
    else
    {
        DateTime_UpdateNormalized(dateTime);
    }
}

inline static void DateTime_AddMinutes(SimulatedDateTime *dateTime, int minutes)
{
    dateTime->minutes += minutes;

    if (dateTime->minutes >= 60)
    {
        int addHours = dateTime->minutes / 60;
        dateTime->minutes %= 60;
        DateTime_AddHours(dateTime, addHours);
    }
    else
    {
        DateTime_UpdateNormalized(dateTime);
    }
}

inline static void DateTime_AddSeconds(SimulatedDateTime *dateTime, float seconds)
{
    dateTime->seconds += seconds;

    if (dateTime->seconds >= 60.0f)
    {
        int addMinutes = (int)(dateTime->seconds / 60.0f);
        dateTime->seconds = fmodf(dateTime->seconds, 60.0f);
        DateTime_AddMinutes(dateTime, addMinutes);
    }
    else
    {
        DateTime_UpdateNormalized(dateTime);
    }
}

// -------------------------
// Entity Events
// -------------------------

void World_Update()
{
    // Update all entities in the world
    for (int i = 0; i < _world->parent->transform.children_size; i++)
    {
        Entity_Update(_world->parent->transform.children[i]->entity);
    }
    // Update simulated time
    DateTime_AddSeconds(&_world->dateTime, DeltaTime * _world->simulationSecondScalor);
}

// -------------------------
// World Management
// -------------------------

void World_Select(World *world)
{
    if(world == _world) {
        return;
    }
    LogSuccess(&_logConfig, "Selected world '%s'", world->name);
    _world = world;
}

// -------------------------
// Creation and Freeing
// -------------------------

World *World_Create(char *name, float simulationSecondScalor)
{
    World *world = malloc(sizeof(World));
    // Time
    world->dateTime = (SimulatedDateTime){
        .year = 1,
        .month = 1,
        .day = 1,
        .hours = 0,
        .minutes = 0,
        .seconds = 0.0f};
    DateTime_AddHours(&world->dateTime, 6);
    world->simulationSecondScalor = simulationSecondScalor;
    // Skybox
    Shader* skyboxShader = ShaderManager_Get(SHADER_SKYBOX);
    world->skyboxMaterial = Material_Create(skyboxShader, 0, NULL);
    Material_MarkReferenced(world->skyboxMaterial);
    world->skyboxMesh = Mesh_CreateCube(world->skyboxMaterial, (V3){100.0f, 100.0f, 100.0f}, V3_HALF, 0xffffffff);
    Mesh_MarkReferenced(world->skyboxMesh);
    LogSuccess(&_logConfig, "Created skybox mesh and material for world '%s'", name);
    // Name
    world->name = name;
    // Physics
    world->physicsManager = PhysicsManager_Create(FixedDeltaTime);
    // Renderers
    world->meshRenderers_size = 0;
    world->meshRenderers = NULL;
    // Directional Lights
    world->lights_directional_size = 0;
    world->lights_directional = NULL;
    // Point Lights
    world->lights_point_size = 0;
    world->lights_point = NULL;
    // GUI
    world->guis_size = 0;
    world->guis = NULL;
    // Entity
    world->parent = Entity_Create_WorldParent(world, V3_ZERO, QUATERNION_IDENTITY, V3_ONE);
    // Update Worlds container
    _worldCount++;
    _worlds = realloc(_worlds, sizeof(World *) * _worldCount);
    _worlds[_worldCount - 1] = world;
    // Select the created world
    World_Select(world);
    return world;
}

void World_Free(World *world, bool resizeWorlds)
{
    world->isMarkedForFreeing = true;
    LogFree(&_logConfig, "World<%s>, Resize List of Worlds<%d>", world->name, resizeWorlds);
    // Update world count
    if (resizeWorlds) // Resize Worlds container
    {
        for (int i = 0; i < _worldCount; i++)
        {
            if (_worlds[i] == world)
            {
                _worldCount--;
                // Shift remaining worlds down
                for (int j = i; j < _worldCount - 1; j++)
                {
                    _worlds[j] = _worlds[j + 1];
                }
                _worlds = realloc(_worlds, sizeof(World *) * _worldCount);
                break;
            }
        }
    }
    // Skybox
    Mesh_MarkUnreferenced(world->skyboxMesh);
    // Free Entities
    Entity_Free(world->parent, false);
    // Free struct
    free(world->meshRenderers);
    free(world->lights_directional);
    free(world->lights_point);
    free(world->guis);
    free(world);
}

void World_All_Free()
{
    LogFree(&_logConfig, "%d Worlds", _worldCount);
    for (int i = 0; i < _worldCount; i++)
    {
        World_Free(_worlds[i], false);
    }
    free(_worlds);
}

// -------------------------
// Lighting
// -------------------------

void World_Light_Add(EC_Light *ec_light)
{
    Log(&_logConfig, "Adding light to world from entity '%s'", ec_light->component->entity->name);

    switch (ec_light->type)
    {
    case LS_T_DIRECTIONAL:
        _world->lights_directional_size++;
        _world->lights_directional = realloc(_world->lights_directional, sizeof(EC_Light *) * _world->lights_directional_size);
        _world->lights_directional[_world->lights_directional_size - 1] = ec_light;
        break;
    case LS_T_POINT:
        _world->lights_point_size++;
        _world->lights_point = realloc(_world->lights_point, sizeof(EC_Light *) * _world->lights_point_size);
        _world->lights_point[_world->lights_point_size - 1] = ec_light;
        break;
    default:
        LogError(&_logConfig, "A light source (%s) of an unknown type has been added to the world. It will not be buffered nor considered in rendering.", ec_light->component->entity->name);
        break;
    }
}

void World_Light_Remove(EC_Light *ec_light)
{
    if (_world->isMarkedForFreeing)
    {
        return;
    }
    switch (ec_light->type)
    {
    case LS_T_DIRECTIONAL:
        for (int i = 0; i < _world->lights_directional_size; i++)
        {
            if (_world->lights_directional[i] == ec_light)
            {
                _world->lights_directional_size--;
                // Shift remaining lights down
                for (int j = i; j < _world->lights_directional_size; j++)
                {
                    _world->lights_directional[j] = _world->lights_directional[j + 1];
                }
                _world->lights_directional = realloc(_world->lights_directional, sizeof(EC_Light *) * _world->lights_directional_size);
                break;
            }
        }
        break;
    case LS_T_POINT:
        for (int i = 0; i < _world->lights_point_size; i++)
        {
            if (_world->lights_point[i] == ec_light)
            {
                _world->lights_point_size--;
                // Shift remaining lights down
                for (int j = i; j < _world->lights_point_size; j++)
                {
                    _world->lights_point[j] = _world->lights_point[j + 1];
                }
                _world->lights_point = realloc(_world->lights_point, sizeof(EC_Light *) * _world->lights_point_size);
                break;
            }
        }
        break;
    default:
        break;
    }
}

// -------------------------
// 3D Renderers
// -------------------------

void World_Renderer3D_Add(EC_MeshRenderer *ec_meshRenderer)
{
    World *world = World_Get(ec_meshRenderer->component->entity);
    world->meshRenderers_size++;
    world->meshRenderers = realloc(world->meshRenderers, sizeof(EC_MeshRenderer) * world->meshRenderers_size);
    if (world->meshRenderers_size == 1)
    {
        world->meshRenderers[0] = ec_meshRenderer;
    }
    else
    {
        int i = world->meshRenderers_size - 1;
        uint32_t shaderID = ec_meshRenderer->material->shader->id;
        for (; i >= 1; i--)
        {
            if (world->meshRenderers[i - 1]->material->shader->id > shaderID)
                world->meshRenderers[i] = world->meshRenderers[i - 1];
            else
            {
                world->meshRenderers[i] = ec_meshRenderer;
                break;
            }
        }
    }
}

void World_Renderer3D_Remove(EC_MeshRenderer *ec_meshRenderer)
{
    World *world = World_Get(ec_meshRenderer->component->entity);
    if (world->isMarkedForFreeing)
    {
        return;
    }
    for (int i = 0; i < world->meshRenderers_size; i++)
    {
        if (world->meshRenderers[i] == ec_meshRenderer)
        {
            world->meshRenderers_size--;
            // Shift remaining renderers down
            for (int j = i; j < world->meshRenderers_size; j++)
            {
                world->meshRenderers[j] = world->meshRenderers[j + 1];
            }
            world->meshRenderers = realloc(world->meshRenderers, sizeof(EC_MeshRenderer) * world->meshRenderers_size);
            break;
        }
    }
}

// -------------------------
// GUI
// -------------------------

void World_GUI_Add(EC_GUI *ec_gui)
{
    Log(&_logConfig, "Adding GUI to world from entity '%s'", ec_gui->component->entity->name);

    for (size_t i = 0; i < _world->guis_size; i++)
    {
        if (_world->guis[i] == NULL)
        {
            _world->guis[i] = ec_gui;
            return;
        }
    }

    _world->guis_size++;
    _world->guis = realloc(_world->guis, sizeof(EC_GUI *) * _world->guis_size);
    _world->guis[_world->guis_size - 1] = ec_gui;
}

void World_GUI_Remove(EC_GUI *ec_gui)
{
    Log(&_logConfig, "Removing GUI from world from entity '%s'", ec_gui->component->entity->name);

    for (size_t i = 0; i < _world->guis_size; i++)
    {
        if (_world->guis[i] == ec_gui)
        {
            // Shift remaining GUIs down
            for (size_t j = i; j < _world->guis_size - 1; j++)
            {
                _world->guis[j] = _world->guis[j + 1];
            }
            _world->guis_size--;
            _world->guis = realloc(_world->guis, sizeof(EC_GUI *) * _world->guis_size);
            break;
        }
    }
}

// -------------------------
// World Management
// -------------------------

Entity *World_GetRoot()
{
    return _world->parent;
}

World *World_Get(Entity *entity)
{
    Transform *parentT = entity->transform.parent;
    while (parentT->parent != NULL)
    {
        parentT = parentT->parent;
    }
    for (int i = 0; i < _worldCount; i++)
    {
        if (_worlds[i]->parent == parentT->entity)
        {
            return _worlds[i];
        }
    }
    return NULL;
}