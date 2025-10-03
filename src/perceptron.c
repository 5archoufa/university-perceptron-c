#include "perceptron.h"
#include <stdio.h>
#include "neural_networks/layer.h"
#include "neural_networks/neural_network.h"
#include "ui/window.h"
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include "utilities/math/v3.h"
#include "input/input_manager.h"
#include <X11/extensions/XInput2.h>
#include "logging/logger.h"
#include "entity/entity.h"
#include "game/game.h"
#include "state_machine/instances/sm_perceptron/sm_perceptron.h"
#include <time.h>

// Constants
const int UPDATE_RATE_LIMIT = 40;
const int FIXEDUPDATE_RATE_LIMIT = 40;
// External
float DeltaTime = 1 / (float)UPDATE_RATE_LIMIT;
float FixedDeltaTime = 1 / (float)FIXEDUPDATE_RATE_LIMIT;

static LogConfig logConfig = {
    "Perceptron", LOG_LEVEL_INFO, LOG_COLOR_BLUE};

int main()
{
    printf("Perceptron: Hello World.\n");
    // Create Display
    Display *display = Display_Create();
    if (display == NULL)
    {
        printf("Failed to continue without a display.\n");
        return 1;
    }
    // Create Window
    MyWindowConfig config;
    config.windowWidth = 1600 * 1.2;
    config.windowHeight = 900 * 1.2;
    config.imageWidth = 800;
    config.imageHeight = 450;
    MyWindow *myWindow = MyWindow_Create(display, config);

    // Input
    int xi_opcode, xi_event, error;
    if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &xi_event, &error))
    {
        fprintf(stderr, "X Input extension not available.\n");
        exit(1);
    }

    // Check XInput2 version
    int major = 2, minor = 0;
    if (XIQueryVersion(display, &major, &minor) != Success)
    {
        fprintf(stderr, "XInput2 not available.\n");
        exit(1);
    }
    XIEventMask evmask;
    unsigned char mask[(XI_LASTEVENT + 7) / 8] = {0};
    evmask.deviceid = XIAllDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;
    XISetMask(mask, XI_KeyPress);
    XISetMask(mask, XI_KeyRelease);
    XISetMask(mask, XI_ButtonPress);
    XISetMask(mask, XI_ButtonRelease);
    XISetMask(mask, XI_Motion);
    XISelectEvents(display, myWindow->window, &evmask, 1);
    XFlush(display);
    InputManager_Init();

    Game_Awake();
    Game_Start();

    // State Machine
    // StateMachine* sm_perceptron = SMPerceptron_Init();

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
    double frameTimer;
    // Infinite Loop
    bool isRunning = true;
    while (isRunning)
    {
        XEvent event;
        // Measure Time
        clock_gettime(CLOCK_MONOTONIC, &ts);
        double now = ts.tv_sec + ts.tv_nsec * 1e-9;
        float time = now - lastTime;
        lastTime = now;
        fixedAccumulator += time;
        updateAccumulator += time;
        // Calculate Framerate
        frameTimer += time;
        frameCounter++;
        if (frameTimer >= 1.0)
        {
            printf("FPS: %d\n", frameCounter);
            frameCounter = 0;
            frameTimer -= 1.0;
        }

        // Input Events
            while (XPending(display))
            {
                InputManager_ResetInputs();
                XNextEvent(display, &event);
                if (XGetEventData(display, &event.xcookie))
                {
                    InputManager_HandleXEvent(&event);// Input Events
            while (XPending(display))
            {
                InputManager_ResetInputs();
                XNextEvent(display, &event);
                if (XGetEventData(display, &event.xcookie))
                {
                    InputManager_HandleXEvent(&event);
                    XFreeEventData(display, &event.xcookie);
                }
                if (event.type == ClientMessage)
                {
                    if ((Atom)event.xclient.data.l[0] == myWindow->WM_DELETE)
                    {
                        isRunning = false;
                        Log(&logConfig, "WM_DELETE Called. Exiting Main Loop...");
                        break;
                    }
                }
            }
                    XFreeEventData(display, &event.xcookie);
                }
                if (event.type == ClientMessage)
                {
                    if ((Atom)event.xclient.data.l[0] == myWindow->WM_DELETE)
                    {
                        isRunning = false;
                        Log(&logConfig, "WM_DELETE Called. Exiting Main Loop...");
                        break;
                    }
                }
            }

        // Update Entities
        if (updateAccumulator >= DeltaTime)
        {
            updateAccumulator -= DeltaTime;

            Game_Update();
            Game_LateUpdate();
        }
        while (fixedAccumulator >= FixedDeltaTime && physicsSteps <= maxPhysicsSteps)
        {
            physicsSteps++;
            Game_FixedUpdate();
            fixedAccumulator -= FixedDeltaTime;
        }
        physicsSteps = 0;
        // StateMachine_Tick(sm_perceptron);

        // Render
        // XFlush(myWindow->display);

        // Handle window events
        if (!MyWindow_Handle(myWindow))
        {
            isRunning = false;
        }
        else
        {
        }
    }

    // Free up memory
    InputManager_Free();
    Game_Free();
    SMPerceptron_Free();
    MyWindow_Free(myWindow);

    return 0;
}