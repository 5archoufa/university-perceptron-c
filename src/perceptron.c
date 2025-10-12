#include "perceptron.h"
// C
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
// Window
#include "ui/window.h"
// Input
#include "input/input_manager.h"
// Logging
#include "logging/logger.h"
// Game
#include "game/game.h"
// Entity
#include "entity/entity.h"
// Mesh
#include "rendering/mesh/mesh-manager.h"
// Material
#include "rendering/material/material-manager.h"
#include "rendering/material/material.h"
// Texture
#include "rendering/texture/texture-manager.h"
// Shader
#include "rendering/shader/shader-manager.h"
#include "rendering/shader/shader.h"
// OpenGL
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Constants
const int UPDATE_RATE_LIMIT = 60;
const int FIXEDUPDATE_RATE_LIMIT = 40;
const V3 WORLD_UP = {0, 1, 0};
const vec3 WORLD_UP_vec3 = {0, 1, 0};
const V3 WORLD_RIGHT = {1, 0, 0};
const V3 WORLD_FORWARD = {0, 0, 1};
const V3 WORLD_DOWN = {0, -1, 0};
const V3 WORLD_LEFT = {-1, 0, 0};
const V3 WORLD_BACK = {0, 0, -1};
// External
static float _desiredDeltaTime = 1 / (float)UPDATE_RATE_LIMIT;
float DeltaTime = 1 / (float)UPDATE_RATE_LIMIT;
float FixedDeltaTime = 1 / (float)FIXEDUPDATE_RATE_LIMIT;
float PerceptronTime = 0.0f;
GLuint ShaderProgram;
// Window configuration - imageWidth/Height is the low-res render target, windowWidth/Height is the actual window size
const float m = 0.4f;
MyWindowConfig WindowConfig = {(int)(1280 * m), (int)(720 * m), 1280, 720};

static LogConfig _logConfig = {
    "Perceptron", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // Update the window config with new dimensions
    WindowConfig.windowWidth = width;
    WindowConfig.windowHeight = height;

    // The viewport will be adjusted during BlitToScreen based on aspect ratio
    glViewport(0, 0, width, height);

    // Log the resize for debugging
    printf("Window resized to: %dx%d (render target remains: %dx%d)\n",
           width, height, (int)WindowConfig.imageWidth, (int)WindowConfig.imageHeight);
}

int main()
{
    // Set random seed
    srand((unsigned int)time(NULL));
    printf("Perceptron: Hello World.\n");
    // Initialize GLFW
    if (!glfwInit())
    {
        LogError(&_logConfig, "Failed to initialize GLFW");
        return -1;
    }

    // Configure GLFW for OpenGL 4.6 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    LogSuccess(&_logConfig, "GLFW initialized successfully for OpenGL 4.6 Core.");

    // Create GLFW window
    GLFWwindow *window = glfwCreateWindow(WindowConfig.windowWidth, WindowConfig.windowHeight, "Perceptron", NULL, NULL);
    if (!window)
    {
        LogError(&_logConfig, "Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    LogSuccess(&_logConfig, "Window created successfully: %dx%d\n", WindowConfig.windowWidth, WindowConfig.windowHeight);
    // Make the OpenGL context current
    glfwMakeContextCurrent(window);
    LogSuccess(&_logConfig, "OpenGL context made current.\n");
    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LogError(&_logConfig, "Failed to initialize GLAD\n");
        return -1;
    }
    // Configure OpenGL state
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Query actual framebuffer size (may differ from window size on high DPI displays)
    int actualWidth, actualHeight;
    glfwGetFramebufferSize(window, &actualWidth, &actualHeight);
    WindowConfig.windowWidth = actualWidth;
    WindowConfig.windowHeight = actualHeight;
    glViewport(0, 0, actualWidth, actualHeight);
    LogSuccess(&_logConfig, "Framebuffer size: %dx%d\n", actualWidth, actualHeight);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE); // Optional but common
    glCullFace(GL_BACK);    // Cull back-facing triangles
    glFrontFace(GL_CCW);    // Counter-clockwise = front

    // ============ Input ============ //
    InputManager_Init(window);

    // ============ Shaders ============ //
    ShaderManager *shaderManager = ShaderManager_Create();
    ShaderManager_Select(shaderManager);

    // ============ Materials ============ //
    MaterialManager *materialManager = MaterialManager_Create();
    MaterialManager_Select(materialManager);

    // ============ Meshes ============ //
    MeshManager *meshManager = MeshManager_Create();
    MeshManager_Select(meshManager);

    // ============ Textures ============ //
    TextureManager *textureManager = TextureManager_Create();
    TextureManager_Select(textureManager);

    // ============ 3D Renderers ============ //
    Shader *toonShader = ShaderManager_Get(SHADER_TOON_SOLID);
    Material *renderer3D_defaultMaterial = Material_Create(toonShader, 0, NULL);
    EC_Renderer3D_SetDefaultMaterial(renderer3D_defaultMaterial);

    // ============ Entities ============ //
    Entity_InitSystem();

    // ============ Physics ============ //
    PhysicsManager *physicsManager = PhysicsManager_Create();

    // ============ Start Infinite Loop ============ //
    Game_Awake();
    Game_Start();

    double fixedAccumulator = 0.0;
    double updateAccumulator = 0.0;
    int maxPhysicsSteps = 5;
    int physicsSteps = 0;
    // FPS
    int frameCounter = 0;
    // Temporary Variables
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    double lastTime = ts.tv_sec + ts.tv_nsec * 1e-9;
    double frameTimer = 0.0;
    // Infinite Loop
    bool isRunning = true;
    while (isRunning)
    {
        if (glfwWindowShouldClose(window))
        {
            isRunning = false;
            Log(&_logConfig, "GLFW Window close requested. Exiting Main Loop...");
            break;
        }

        glfwPollEvents();
        // Measure Time
        clock_gettime(CLOCK_MONOTONIC, &ts);
        double now = ts.tv_sec + ts.tv_nsec * 1e-9;
        float time = now - lastTime;
        lastTime = now;
        fixedAccumulator += time;
        updateAccumulator += time;
        PerceptronTime += time;
        frameTimer += time;
        // Update Entities
        if (updateAccumulator >= _desiredDeltaTime)
        {
            // DeltaTime = updateAccumulator;
            updateAccumulator -= _desiredDeltaTime;
            frameCounter++;
            // Calculate Framerate
            if (frameTimer >= 1.0)
            {
                printf("FPS: %d\n", frameCounter);
                frameCounter = 0;
                frameTimer -= 1.0;
            }
            // Update Loop
            Game_Update();
            // Late Update Loop
            Game_LateUpdate();
            glfwSwapBuffers(window);
            Game_EndOfFrame();
            // Input
            InputManager_ResetInputs();

            // End-of-Frame Cleanup
            MeshManager_Cleanup();
            MaterialManager_Cleanup();
            TextureManager_Cleanup();
        }
        while (fixedAccumulator >= FixedDeltaTime && physicsSteps <= maxPhysicsSteps)
        {
            physicsSteps++;
            Game_FixedUpdate();
            fixedAccumulator -= FixedDeltaTime;
        }
        // Cleanup

        physicsSteps = 0;
    }

    // -------------------------
    // Free
    // -------------------------
    // Input
    InputManager_Free();
    // Game
    Game_Free();
    // Physics
    PhysicsManager_Free(physicsManager);
    // Entities
    Entity_FreeCache();
    // Meshes
    MeshManager_Free(meshManager);
    // Shaders
    ShaderManager_Free(shaderManager);
    // Materials
    MaterialManager_Free(materialManager);
    // Textures
    TextureManager_Free(textureManager);
    // Game
    // MyWindow_Free(myWindow);
    // Cleanup GLFW
    glDeleteProgram(ShaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}