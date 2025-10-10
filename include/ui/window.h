#ifndef MY_WINDOW_H
#define MY_WINDOW_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct MyWindowConfig
{
    size_t imageWidth, imageHeight;
    size_t windowWidth, windowHeight;
} MyWindowConfig;

#endif