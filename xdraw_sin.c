/* xdraw_sin.c */
#include <stdio.h>
#include <math.h>    // cc -lm
#include <X11/Xlib.h> // cc -l X11
#include <X11/Xutil.h>

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

	XSelectInput(dpy, w, ButtonPressMask | ExposureMask);
	XSelectInput(dpy, exit, ButtonPressMask | ExposureMask);

	XEvent event;

	int pos_x = -1, pos_y = -1; //init
	int s_pos_x = -1, s_pos_y = -1; // init

	unsigned long current = black;

	char showing_letter[9];
	KeySym key;
	char string[9];

	while(1){
		XNextEvent(dpy, &event);
		switch(event.type){
			case Expose:
					if (event.xexpose.window == exit) {
						XDrawString(dpy, exit, gc, 15, 20, "Exit", 4);
					} else {
						XDrawLine(dpy, w, gc, 50, 175, 450, 175);
						XDrawLine(dpy, w, gc, 50, 50, 50, 300);
						for(int i = 0; i < 400; ++i){// O = (50, 175)
							XDrawPoint(dpy, w, gc, 50+i, 175-100*sin(i/100.0));
							// XDrawPoint(dpy, w, gc, 50+i, 175-100*cos(i/100.0));
							// XDrawPoint(dpy, w, gc, 50+i, 175-100*tan(i/100.0));
						}
					}
			break;
			case ButtonPress:
				if(event.xany.window == exit) return 0;
			break;
		}
	}
}

