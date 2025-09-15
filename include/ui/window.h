#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct MyWindowConfig
{
    unsigned int height, width;

} MyWindowConfig;

typedef struct MyWindow
{
    Display *display;
    MyWindowConfig *config;
    Window window;
    Atom WM_DELETE;
} MyWindow;

Display *CreateDisplay()
{
    Display *d = XOpenDisplay(NULL);
    if (!d)
    {
        fprintf(stderr, "Can't open display\n");
        return NULL;
    }
    return d;
}

MyWindow *CreateMyWindow(Display *display, MyWindowConfig *config)
{
    int scr = DefaultScreen(display);
    printf("%d%d", config->width, config->height);
    Window window = XCreateSimpleWindow(display, RootWindow(display, scr),
                                        100, 100, config->width, config->height, 1,
                                        BlackPixel(display, scr), WhitePixel(display, scr));
    /* Ask for Expose and button/key events and ConfigureNotify (resize) */
    XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
    /* Handle the window manager close button */
    Atom WM_DELETE = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &WM_DELETE, 1);
    XMapWindow(display, window); /* show it */

    MyWindow *myWindow = malloc(sizeof(MyWindow));
    myWindow->window = window;
    myWindow->display = display;
    myWindow->config = config;
    myWindow->WM_DELETE = WM_DELETE;
    return myWindow;
}

void FreeMyWindow(MyWindow *myWindow)
{
    XDestroyWindow(myWindow->display, myWindow->window);
}

void FreeDisplay(Display *display)
{
    XCloseDisplay(display);
}

bool HandleMyWindow(MyWindow *myWindow)
{
    XEvent ev;
    if (XPending(myWindow->display))
    {
        XNextEvent(myWindow->display, &ev);
        switch (ev.type)
        {
        case Expose:
            /* paint once on expose if you want */
            break;
        case ClientMessage:
            if ((Atom)ev.xclient.data.l[0] == myWindow->WM_DELETE)
                return false;
            break;
        }
    }

    return true;
}