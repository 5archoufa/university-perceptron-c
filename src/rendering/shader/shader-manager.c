#include "rendering/texture/texture-manager.h"
#include "rendering/shader/shader-manager.h"
#include "logging/logger.h"

#include <stdlib.h>

static LogConfig _logConfig = {"ShaderManager", LOG_LEVEL_INFO, LOG_COLOR_BLUE};
static ShaderManager* _manager = NULL;

// -------------------------
// Creation and Freeing
// -------------------------

ShaderManager *ShaderManager_Create()
{
    ShaderManager *manager = malloc(sizeof(ShaderManager));
    manager->shaders_size = 0;
    manager->shaders = NULL;

    // Pre-allocate some space
    manager->shaders_size = 10;
    manager->shaders = malloc(sizeof(Shader *) * manager->shaders_size);
    for (int i = 0; i < manager->shaders_size; i++)
    {
        manager->shaders[i] = NULL;
    }
    return manager;
}

void ShaderManager_Free(ShaderManager *manager)
{
    if (!manager)
    {
        return;
    }
    for (int i = 0; i < manager->shaders_size; i++)
    {
        Shader_Free(manager->shaders[i]);
    }
    free(manager->shaders);
    free(manager);
}

// -------------------------
// Functions
// -------------------------

void ShaderManager_Select(ShaderManager* manager)
{
    _manager = manager;
}

void ShaderManager_AddShader(Shader *shader)
{
    // Check if any slots are free before-hand
    for (int i = 0; i < _manager->shaders_size; i++)
    {
        if (_manager->shaders[i] == NULL)
        {
            _manager->shaders[i] = shader;
            return;
        }
    }
    _manager->shaders_size++;
    _manager->shaders = realloc(_manager->shaders, sizeof(Shader *) * _manager->shaders_size);
    _manager->shaders[_manager->shaders_size - 1] = shader;
}

Shader *ShaderManager_Get(char *name)
{
    for (int i = 0; i < _manager->shaders_size; i++)
    {
        if (strcmp(_manager->shaders[i]->name, name) == 0)
        {
            return _manager->shaders[i];
        }
    }
    LogWarning(&_logConfig, "Could not get Shader %s", name);
    return NULL;
}