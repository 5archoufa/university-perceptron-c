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

float DeltaTime = 0.1;

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
    config.width = 1600;
    config.height = 900;
    MyWindow *myWindow = MyWindow_Create(display, config);

    // Input
    int xi_opcode, event, error;
    if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error))
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

    // Infinite Loop
    bool isRunning = true;
    while (isRunning)
    {
        // Input Events
        XEvent event;
        while (XPending(display))
        {
            XNextEvent(display, &event);
            InputManager_ResetInputs();
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

        // Update Entities
        Game_Update();
        Game_LateUpdate();
        Game_FixedUpdate();
        //StateMachine_Tick(sm_perceptron);

        // Render
        XFlush(myWindow->display);

        // Handle window events
        if (!MyWindow_Handle(myWindow))
        {
            isRunning = false;
        }
        else
        {
            sleep(DeltaTime);
        }
    }

    // Free up memory
    InputManager_Free();
    Game_Free();
    SMPerceptron_Free();
    MyWindow_Free(myWindow);

    return 0;
}