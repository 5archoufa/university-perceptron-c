#include <stdio.h>
#include "neural_networks/layer.h"
#include "neural_networks/neural_network.h"
#include "ui/window.h"
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include "ui/x11/shape.h"
#include "utilities/math/v3.h"
#include "ui/x11/circle.h"

int main()
{
    printf("Perceptron: Hello World.\n");
    int weightCounts[] = {3, 1, 3, 2};
    NeuralNetwork *neuralNetwork = CreateNeuralNetwork_RandomWeights("Test", 2, 4, weightCounts);
    PrintNeuralNetwork(neuralNetwork);
    FreeNeuralNetwork(neuralNetwork);

    // Create Display
    Display *display = CreateDisplay();
    if (display == NULL)
    {
        printf("Failed to continue without a display.\n");
        return 1;
    }
    // Create Window
    MyWindowConfig config;
    config.width = 1900;
    config.height = 900;
    MyWindow *myWindow = CreateMyWindow(display, &config);

    V3 center = {1000, 500, 0};
    Shape* circle = CreateCircle(center, 300);

    GC gc = XCreateGC(myWindow->display, myWindow->window, 0, NULL);

    // Infinite Loop
    bool running = true;
    while (running)
    {
        // Draw
        Pixel* circlePixels = circle->Draw(circle->self);
        while(circlePixels != NULL){
            printf("pixel: %f, %f\n", circlePixels->x, circlePixels->y);
            XDrawPoint(myWindow->display, myWindow->window, gc, (int)circlePixels->x, (int)circlePixels->y);
            circlePixels = circlePixels->next;
        }
        FreePixels(circlePixels);
        XFlush(myWindow->display);
        // Handle window events
        if (!HandleMyWindow(myWindow))
        {
            running = false;
        }
        sleep(0.5);
    }

    XFreeGC(myWindow->display, gc);
    FreeDisplay(myWindow->display);
    FreeMyWindow(myWindow);

    return 0;
}