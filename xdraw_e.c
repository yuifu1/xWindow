/* xdraw.c */
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define BORDER 2
#define WIDTH  350
#define HIGHT 250

int main(int argc,char **argv)
{
	Display *dpy;
	Window w;
	Window root;
	int    screen;
	unsigned long black, white;
	GC       gc;
        XEvent e;

	dpy = XOpenDisplay("");
	root = DefaultRootWindow (dpy);
	screen = DefaultScreen (dpy);
	white = WhitePixel (dpy, screen);
	black = BlackPixel (dpy, screen);

	w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HIGHT, BORDER, black, white);
	gc = XCreateGC(dpy, w, 0, NULL);
        XSelectInput(dpy,w,ButtonPressMask | ButtonReleaseMask);
	XMapWindow(dpy, w);
	while(1){
	   XNextEvent(dpy,&e);
	   switch (e.type){
              case ButtonPress :
		 printf("x=%d y=%d button=%d \n",e.xbutton.x,e.xbutton.y,e.xbutton.button);
		 break;
	      case ButtonRelease :
		 XDrawPoint(dpy,w,gc,e.xbutton.x,e.xbutton.y);
	   }
	}
}

