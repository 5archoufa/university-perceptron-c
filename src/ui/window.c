#include "../../include/ui/window.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

MyWindow *MainWindow = NULL;

Display *Display_Create()
{
    Display *d = XOpenDisplay(NULL);
    if (!d)
    {
        fprintf(stderr, "Can't open display\n");
        return NULL;
    }
    return d;
}

MyWindow *MyWindow_Create(Display *display, MyWindowConfig config)
{
    int screen = DefaultScreen(display);
    printf("%d%d", config.width, config.height);
    Window window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                        100, 100, config.width, config.height, 1,
                                        BlackPixel(display, screen), WhitePixel(display, screen));
    // Ask for Expose and button/key events and ConfigureNotify (resize)
    XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
    // Handle the window manager close button
    Atom WM_DELETE = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &WM_DELETE, 1);
    XMapWindow(display, window);
    // XShm image
    XShmSegmentInfo* shminfo = malloc(sizeof(XShmSegmentInfo));
    XImage *image = XShmCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen),
                                    ZPixmap, NULL, shminfo, config.width, config.height);
    shminfo->shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777);
    shminfo->shmaddr = image->data = shmat(shminfo->shmid, 0, 0);
    shminfo->readOnly = False;
    if (!XShmAttach(display, shminfo))
    {
        printf("XShmAttach failed\n");
    }else{

        printf("XShmAttach succeeded\n");
    }
    // MyWindow
    MyWindow *myWindow = malloc(sizeof(MyWindow));
    myWindow->window = window;
    myWindow->display = display;
    myWindow->image = image;
    myWindow->shminfo = shminfo;
    myWindow->config = config;
    myWindow->gc = XCreateGC(display, window, 0, NULL);
    myWindow->WM_DELETE = WM_DELETE;
    if (MainWindow == NULL)
    {
        MainWindow = myWindow;
    }
    return myWindow;
}

void MyWindow_Free(MyWindow *myWindow)
{
    XShmDetach(myWindow->display, myWindow->shminfo);
    shmdt(myWindow->shminfo->shmaddr);
    shmctl(myWindow->shminfo->shmid, IPC_RMID, 0);
    XDestroyImage(myWindow->image);
    XDestroyWindow(myWindow->display, myWindow->window);
    XFreeGC(myWindow->display, myWindow->gc);
    XCloseDisplay(myWindow->display);
    free(myWindow->shminfo);
    free(myWindow);
}

void Display_Free(Display *display)
{
    XCloseDisplay(display);
}

bool MyWindow_Handle(MyWindow *myWindow)
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