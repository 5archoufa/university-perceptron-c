#ifndef MY_WINDOW_H
#define MY_WINDOW_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xrender.h>

typedef struct MyWindow MyWindow;
extern MyWindow *MainWindow;

typedef struct MyWindowConfig
{
    size_t imageWidth, imageHeight;
    size_t windowWidth, windowHeight;
} MyWindowConfig;

struct MyWindow
{
    Display *display;
    MyWindowConfig config;
    XImage *image;
    Window window;
    XShmSegmentInfo *shminfo;
    Pixmap pix;
    Picture picture_source;
    Picture picture_dest;
    GC gc;
    Atom WM_DELETE;
};

Display *Display_Create();
MyWindow *MyWindow_Create(Display *display, MyWindowConfig config);
void MyWindow_Free(MyWindow *myWindow);
void Display_Free(Display *display);
bool MyWindow_Handle(MyWindow *myWindow);

#endif