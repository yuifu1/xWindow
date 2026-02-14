#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

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

#define PALLET_SIZE 30


int main(int argc, char **argv) {

	Display *dpy = XOpenDisplay("");
	int screen = DefaultScreen(dpy);
	unsigned long white = WhitePixel(dpy, screen);
	unsigned long black = BlackPixel(dpy, screen);
	Window root = DefaultRootWindow(dpy);
	Window w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HEIGHT, BORDER, black, white);
	GC gc = XCreateGC(dpy, w, 0, NULL);

	Colormap cmap = DefaultColormap(dpy, 0);       /* カラーマップ構造体 */
	XColor c0, red, gray, green, blue;             /* カラー構造体 */
	XAllocNamedColor(dpy, cmap, "red", &red, &c0); /* COLOUR.pixel */
	XAllocNamedColor(dpy, cmap, "gray", &gray, &c0);
	XAllocNamedColor(dpy, cmap, "Light green", &green, &c0);
	XAllocNamedColor(dpy, cmap, "blue", &blue, &c0);

	unsigned long colors[5] = {black, red.pixel, gray.pixel, green.pixel, blue.pixel};

	XMapWindow(dpy, w);

	XSelectInput(dpy, w, ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask);
	XEvent event;

	int pos_x = -1, pos_y = -1; //init
	int s_pos_x = -1, s_pos_y = -1; // init

	unsigned long current = black;

	char showing_letter[9];
	KeySym key;
	char string[9];

	while(1){

		XNextEvent(dpy, &event);

		switch (event.type) {
			case ButtonPress:

				switch(event.xbutton.button){
					case MOUSE_LEFT:
						pos_x = event.xbutton.x;
						pos_y = event.xbutton.y;
					break;
					case MOUSE_MIDDLE:
						XClearWindow(dpy, w); // init

						XSetForeground(dpy, gc, red.pixel); // monster ball
						XFillArc(dpy, w, gc, 100, 100, 300, 300, 0, 180*64);

						XSetForeground(dpy, gc, gray.pixel);
						XFillArc(dpy, w, gc, 100, 120, 300, 300, 180*64, 180*64);

						XSetForeground(dpy, gc, black);
						XFillRectangle(dpy, w, gc, 110, 250, 280, 20);

						XSetForeground(dpy, gc, black);
						XFillArc(dpy, w, gc, 220, 225, 70, 70, 0, 360*64);

						XSetForeground(dpy, gc, white);
						XFillArc(dpy, w, gc, 225, 230, 60, 60, 0, 360*64);

						XSetForeground(dpy, gc, current);
					break;
					case MOUSE_RIGHT:
						s_pos_x = event.xbutton.x;
						s_pos_y = event.xbutton.y;
					break;

				}

			break;
			case ButtonRelease:
				if(pos_x == -1) break;
				if(event.xbutton.button == MOUSE_MIDDLE){
					XClearWindow(dpy, w); // init
				} else if(event.xbutton.button == MOUSE_LEFT){

					XGCValues xgcv;
					xgcv.line_width = 3;
					XChangeGC(dpy, gc, GCLineWidth, &xgcv); /* このgcを使って絵を描く */

					XSetForeground(dpy, gc, current);
					XDrawLine(dpy, w, gc, pos_x, pos_y, event.xbutton.x, event.xbutton.y);
				}
			break;
			case KeyPress:
				printf("Keycode = %d\n", event.xkey.keycode);

				if(XLookupString(&event, string, 9, &key, NULL) == 1){
					if(s_pos_x != -1){
						showing_letter[0] = string[0];
						showing_letter[1] = '\0';
						// printf("key = %s\n", showing_letter);
						XDrawString(dpy, w, gc, s_pos_x, s_pos_y, showing_letter, strlen(showing_letter));
						s_pos_x += 10; // letter space distance
					}
				}

				switch(event.xkey.keycode){
					case NUM_1:
						current = red.pixel;
					break;
					case NUM_2:
						current = green.pixel;
					break;
					case NUM_3:
						current = blue.pixel;
					break;
					case NUM_0:
						current = black;
					break;
				}

			break;
			case KeyRelease:
			break;
		}

		XSetForeground(dpy, gc, current);
		XFillRectangle(dpy, w, gc, 10, 10, 50, 50);

		int YY = 10;
		for(int i = 0;i < 5; ++i){// num of color
			XSetForeground(dpy, gc, colors[i]);
			// XFillRectangle(dpy, w, gc, WIDTH - PALLET_SIZE - 10, HEIGHT - YY, WIDTH - 10, HEIGHT - YY - PALLET_SIZE);
			// YY += PALLET_SIZE + 10;
		}

	}
}

