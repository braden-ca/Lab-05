#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool gameOver;
const int width = 20;
const int height = 20;
int score;
int panX = 100;
int panY = 100;

struct Global {
    Display *dpy;
    Window win;
    GC gc;
    int xres, yres;
} g;

void x11_cleanup_xwindows(void);
void x11_init_xwindows(void);
void x11_clear_window(void);
void render(void);

 
void render(void)
{
        XClearWindow(g.dpy, g.win);

        // Font
        XSetFont(g.dpy, g.gc, XLoadFont(g.dpy, "9x15bold"));
        
        // Foreground color
        XSetForeground(g.dpy, g.gc, 0xffffff);
        XFillRectangle(g.dpy, g.win, g.gc, 0, 0, g.xres, g.yres);

        // Include text
        XSetForeground(g.dpy, g.gc, 0x48494B);
        XDrawString(g.dpy, g.win, g.gc, 125, 20, "GoldRushLite", 12);

        XSetForeground(g.dpy, g.gc, 0x48494B);
        XDrawString(g.dpy, g.win, g.gc, 1, 20, "Timer: ", 7);

        XSetForeground(g.dpy, g.gc, 0x48494B);
        XDrawString(g.dpy, g.win, g.gc, 300, 20, "Score: ", 7);
        
        // Draw border
        XDrawRectangle(g.dpy, g.win, g.gc, 0, 0, width * 20, height * 20);

        // Draw pan
        XFillRectangle(g.dpy, g.win, g.gc, panX, panY, 20, 20);

        XFlush(g.dpy);

}

void x11_setFont(unsigned int idx)
{
    char *fonts[] = { "fixed","5x8","6x9","6x10","6x12","6x13","6x13bold",
        "7x13","7x13bold","7x14","8x13","8x13bold","8x16","9x15","9x15bold",
        "10x20","12x24" };
    Font f = XLoadFont(g.dpy, fonts[idx]);
    XSetFont(g.dpy, g.gc, f);
}

void x11_cleanup_xwindows(void)
{
    XDestroyWindow(g.dpy, g.win);
    XCloseDisplay(g.dpy);
}

void x11_init_xwindows(void)
{
    int scr;



    if (!(g.dpy = XOpenDisplay(NULL))) {
        fprintf(stderr, "ERROR: could not open display!\n");
        exit(EXIT_FAILURE);
    }
    scr = DefaultScreen(g.dpy);
    g.xres = 400;
    g.yres = 200;
    g.win = XCreateSimpleWindow(g.dpy, RootWindow(g.dpy, scr), 1, 1,
                            g.xres, g.yres, 0, 0x00ffffff, 0x00000000);
    XStoreName(g.dpy, g.win, "cs3600 xwin sample");
    g.gc = XCreateGC(g.dpy, g.win, 0, NULL);
    XMapWindow(g.dpy, g.win);
    XSelectInput(g.dpy, g.win, ExposureMask | StructureNotifyMask |
                                PointerMotionMask | ButtonPressMask |
                                ButtonReleaseMask | KeyPressMask);
}


int main(){
 x11_init_xwindows();
    while (!gameOver) {
        render();
        XEvent e;
        XNextEvent(g.dpy, &e);
        if (e.type == Expose) {
            // Redraw the window
            render();
        }
    }
x11_cleanup_xwindows();
    return 0;
}
