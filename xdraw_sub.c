/* xdraw_sub.c */
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>

#define BORDER 2
#define WIDTH  500
#define HIGHT 350

int main(int argc, char **argv) {
	Display *dpy;
	Window w,quit;
	Window root;
	int    screen;
	unsigned long black, white;
	GC       gc;
	XEvent  e;

	dpy = XOpenDisplay("");

	root = DefaultRootWindow (dpy);
	screen = DefaultScreen (dpy);
	white = WhitePixel (dpy, screen);
	black = BlackPixel (dpy, screen);
	w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HIGHT, BORDER, black, white);

	quit = XCreateSimpleWindow(dpy, w, 10, 3, 30, 12, BORDER, black, white);
	gc = XCreateGC(dpy, w, 0, NULL);
	XSelectInput(dpy,w,ButtonPressMask);
	XSelectInput(dpy,quit,ButtonPressMask);

	XMapWindow(dpy, w);
	XMapSubwindows(dpy,w);

        XSetForeground(dpy,gc,black);
 	while(1){
	   XNextEvent(dpy,&e);
	   switch(e.type){
	   case ButtonPress:
	     if(e.xany.window==quit)return;
	        XDrawString(dpy, quit, gc, 4, 10,"Exit", 4);
	        XDrawPoint(dpy,w,gc,e.xbutton.x,e.xbutton.y);
	}
      }
}

