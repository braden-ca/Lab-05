#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define TIMER_INTERVAL 30

bool gameOver;
const int width = 20;
const int height = 20;
int score;
int panX = 100;
int panY = 100;
time_t startTime;
int roundCount =1;

struct Global {
    Display *dpy;
    Window win;
    GC gc;
    int xres, yres;
} g;

typedef struct {
    int x, y;
    int isGold; // 1 for gold, 0 for dynamite/rock
    int speed;  // Speed of falling object
}o;

o fallingObjectGold;
o fallingObjectDynamite;

void x11_cleanup_xwindows(void);
void x11_init_xwindows(void);
void x11_clear_window(void);
void render(void);

void NewObject(o *obj) {
    obj->x = rand() % (g.xres - 20); 
    obj->y = 0;                  
    obj->isGold = rand() % 2;        
    obj->speed = rand() % 5 + 1;
}

void moveObject(o *obj) {
    obj->y += obj->speed;

    // Ensure the object stays within the screen boundaries
    if (obj->x < 0) obj->x = 0;
    if (obj->x > (g.xres - 20)) obj->x = g.xres - 20;
    if (obj->y > (g.yres - 20)) {

        NewObject(obj);
    }
}

void checkCollision(o *obj) {
      // Check collision with the pan
      if (obj->x + 20 >= panX && obj->x < panX + 20 && obj->y + 20 >= panY && obj->y < panY + 20) {
          if (obj->isGold) {
              score += 10; // Gold object - add points
          } else {
              score -= 5;  // Dynamite object - subtract points
          }
          NewObject(obj);
      }
  }

void startTimer() {
    time(&startTime);
}

int isTimeUp() {
    time_t currentTime;
    time(&currentTime);
    return (currentTime - startTime) >= TIMER_INTERVAL;
}

void render(void)
{
    XClearWindow(g.dpy, g.win);

    // Font
    // XSetFont(g.dpy, g.gc, XLoadFont(g.dpy, "9x15bold"));

    // Foreground color
    XSetForeground(g.dpy, g.gc, 0xffffff);
    XFillRectangle(g.dpy, g.win, g.gc, 0, 0, g.xres, g.yres);

    // Include text
    XSetForeground(g.dpy, g.gc, 0x48494B);
    XDrawString(g.dpy, g.win, g.gc, 125, 20, "GoldRushLite", 12);
    
    //Draw Timer
    XSetForeground(g.dpy, g.gc, 0x48494B);
    time_t currentTime;
    time(&currentTime);
    int remainingTime = TIMER_INTERVAL - (int)(currentTime - startTime);
    char timerString[20];
    snprintf(timerString, sizeof(timerString), "Timer: %d", remainingTime);
    XDrawString(g.dpy, g.win, g.gc, 1, 20, timerString, strlen(timerString));

    //Draw Score
     XSetForeground(g.dpy, g.gc, 0x48494B);
     char scoreString[20];
     snprintf(scoreString, sizeof(scoreString), "Score: %d", score);
     XDrawString(g.dpy, g.win, g.gc, 300, 20, scoreString, strlen(scoreString));

    //Draw Pan
    XFillRectangle(g.dpy, g.win, g.gc, panX, panY, 20, 20);

    // Draw falling objects
    XSetForeground(g.dpy, g.gc, 0xFFD700); // Gold color
    XFillRectangle(g.dpy, g.win, g.gc, fallingObjectGold.x, fallingObjectGold.y, 20, 20);

    XSetForeground(g.dpy, g.gc, 0xFF4500); // Dynamite color
    XFillRectangle(g.dpy, g.win, g.gc, fallingObjectDynamite.x, fallingObjectDynamite.y, 20, 20);

    XFlush(g.dpy);

}

void updateRound() {
    roundCount++;
    fallingObjectGold.speed += 1;  // Increase the speed for each new round
    fallingObjectDynamite.speed += 1; 
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
    XStoreName(g.dpy, g.win, "GoldRushLite ");
    g.gc = XCreateGC(g.dpy, g.win, 0, NULL);
    XMapWindow(g.dpy, g.win);
    XSelectInput(g.dpy, g.win, ExposureMask | StructureNotifyMask |
            PointerMotionMask | ButtonPressMask |
            ButtonReleaseMask | KeyPressMask);
}


int main(){
    x11_init_xwindows();

    startTimer();

    NewObject(&fallingObjectGold);
    NewObject(&fallingObjectDynamite);

    while (!gameOver) {
        if(isTimeUp()){
            gameOver = true;
        }

        render();

        moveObject(&fallingObjectGold);
        moveObject(&fallingObjectDynamite);

        XEvent e;
        XNextEvent(g.dpy, &e);
        if (e.type == Expose) {
            // Redraw the window
            render();
        }
        if (e.type == KeyPress || e.type == KeyRelease) {
            int key = XLookupKeysym(&e.xkey, 0);
            switch (key) {
                case XK_Left:
                    panX -= 5;
                    break;
                case XK_Right:
                    panX += 5;
                    break;
                case XK_Up:
                    panY -= 5;
                    break;
                case XK_Down:
                    panY += 5;
                    break;
                default:
                    break;
            }
        }
    } 
    x11_cleanup_xwindows();
    return 0;
}
