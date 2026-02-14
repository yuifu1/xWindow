/* xdraw_sin.c */
#include <stdio.h>
#include <math.h>    // cc -lm
#include <X11/Xlib.h> // cc -l X11
#include <X11/Xutil.h>
#include <unistd.h>

#define BORDER 2
#define WIDTH  500
#define HEIGHT 500

#define MOUSE_LEFT 1
#define MOUSE_MIDDLE 2 // wheel click
#define MOUSE_RIGHT 3

#define KEY_R 27
#define KEY_B 56
#define KEY_G 42
#define KEY_SPACE 65

#define NUM_0 90
#define NUM_1 87
#define NUM_2 88
#define NUM_3 89

#define PI 3.1415926535

void rotate_point(XPoint *p, XPoint center, double rad) { //chatgpt
    double dx = p->x - center.x;
    double dy = p->y - center.y;

    p->x = (short)(dx * cos(-rad) - dy * sin(-rad) + center.x);
    p->y = (short)(dx * sin(-rad) + dy * cos(-rad) + center.y);
}

int main(int argc, char **argv) {

	Display *dpy = XOpenDisplay("");
	int screen = DefaultScreen(dpy);
	unsigned long white = WhitePixel(dpy, screen);
	unsigned long black = BlackPixel(dpy, screen);
	Window root = DefaultRootWindow(dpy);
	Window w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HEIGHT, BORDER, black, white);
	XMapWindow(dpy, w);
	GC gc = XCreateGC(dpy, w, 0, NULL);

	Colormap cmap = DefaultColormap(dpy, 0);       /* カラーマップ構造体 */
	XColor c0, red, gray, green, blue;             /* カラー構造体 */
	XAllocNamedColor(dpy, cmap, "red", &red, &c0); /* COLOUR.pixel */
	XAllocNamedColor(dpy, cmap, "gray", &gray, &c0);
	XAllocNamedColor(dpy, cmap, "Light green", &green, &c0);
	XAllocNamedColor(dpy, cmap, "blue", &blue, &c0);

	unsigned long colors[5] = {black, red.pixel, gray.pixel, green.pixel, blue.pixel};

	Window exit = XCreateSimpleWindow(dpy, w, 10, 10, 50, 30, BORDER, black, white); // exit window
	XMapSubwindows(dpy, w);

	XSelectInput(dpy, w, PointerMotionMask);
	XSelectInput(dpy, exit, ButtonPressMask | ExposureMask);

	XEvent event;

	int pos_x = -1, pos_y = -1; //init
	int s_pos_x = -1, s_pos_y = -1; // init

	unsigned long current = black;

	char showing_letter[9];
	KeySym key;
	char string[9];

	int x = 100;
	int y = 100;
	int r = 50;
	XPoint center = {x+(r/2), y+(r/2)};

	float deg = 0;
	float rad = 0;

	while(1){
		if(XEventsQueued(dpy, QueuedAfterReading)){
			XNextEvent(dpy, &event);
			switch(event.type){
				case MotionNotify:
					x = event.xmotion.x;
					y = event.xmotion.y;
					center.x = x+(r/2);
					center.y = y+(r/2);
				break;
				case ButtonPress:
					if(event.xany.window == exit) return 0;
					//
				break;
				case Expose:
					XDrawString(dpy, exit, gc, 10, 18, "Exit", 4);
				break;
			}
		} else {

			float pos = 180.0+deg;
			if(pos > 360) pos -= 360;

			XClearWindow(dpy, w);

			XSetForeground(dpy, gc, red.pixel);
			XFillArc(dpy, w, gc, x, y, r, r, deg*64.0, 180*64);

			XSetForeground(dpy, gc, gray.pixel);
			XFillArc(dpy, w, gc, x, y, r, r, pos*64, 180*64);

			XSetForeground(dpy, gc, black);
			XPoint point[4];
			point[0].x = x;
			point[0].y = y+(r/2)-(r/30);
			point[1].x = x+r;
			point[1].y = y+(r/2)-(r/30);
			point[2].x = x+r;
			point[2].y = y+(r/2)-(r/30)+(r/15);
			point[3].x = x;
			point[3].y = y+(r/2)-(r/30)+(r/15);
			float rad = PI*deg / 180.0; 
			rotate_point(point, center, rad);
			rotate_point(point+1, center, rad);
			rotate_point(point+2, center, rad);
			rotate_point(point+3, center, rad);
			XFillPolygon(dpy, w, gc, point, 4, Convex, CoordModeOrigin);

			XSetForeground(dpy, gc, black);// 中心外側円
			XFillArc(dpy, w, gc, x+(r/2)-(r/10), y+(r/2)-(r/10), r/5, r/5, 0, 360*64);

			XSetForeground(dpy, gc, white);// 中心内側円
			XFillArc(dpy, w, gc, x+(r/2)-(r/10)+(r/60), y+(r/2)-(r/10)+(r/60), (r/5)-(r/30), (r/5)-(r/30), 0, 360*64);

			// XSetForeground(dpy, gc, black);// 外接正角形
			// XDrawRectangle(dpy, w, gc, x, y, r, r); 
			// XDrawLine(dpy, w, gc, x+(r/2), y, x+(r/2), y+r);

			deg += 0.03;
			if(deg > 360) deg = 0.0;
			usleep(500);
			XFlush(dpy);

		}
	}
}