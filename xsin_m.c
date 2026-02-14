/* xsin_m.c */
#include <stdio.h>
#include <math.h>	/*　数学関数を使う　*/
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define BORDER 2
#define WIDTH 500
#define HIGHT 350

int  main(int argc, char **argv){
	Display *dpy;
	Window w, quit;
 	Window root;
 	int screen;
 	unsigned long black, white;
 	GC gc;
 	XEvent e;
	int x, y;
	float i = 0.0;
	dpy = XOpenDisplay("");

	root = DefaultRootWindow(dpy);
	screen = DefaultScreen(dpy);
	white = WhitePixel(dpy, screen);
	black = BlackPixel(dpy, screen);

	w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HIGHT, BORDER, black, white);
/*　サブウインドウを作る　*/
	quit = XCreateSimpleWindow(dpy, w, 10, 3, 30, 12, BORDER, black, white);
	gc = XCreateGC(dpy, w, 0, NULL);

	XSelectInput(dpy, w, ButtonPressMask);
	XSelectInput(dpy, quit, ButtonPressMask);	/*　サブウインドウ用　*/

	XMapWindow(dpy, w);
	XMapSubwindows(dpy, w);	/*　サブウインドウも表示　*/

	XSetForeground(dpy, gc, black);
	while(1){
		if(XEventsQueued(dpy, QueuedAfterReading)){ /* for ground */
			XNextEvent(dpy, &e);
			switch(e.type){
				case ButtonPress :
					if(e.xany.window == quit)return 0;
					i = 0.0;
					XClearWindow(dpy, w);
			}
		}else{	/* back ground */
			XDrawString(dpy, quit, gc, 4, 10, "Exit", 4);
			XDrawLine(dpy, w, gc, 50, 175, 450, 175);	/* x軸 */
			XDrawLine(dpy, w, gc, 50, 50, 50, 300);		/* y軸 */
			XDrawPoint(dpy, w, gc, 50+i, 175-100*sin(i/30.0));	/* 点 */
			i += 0.1;
			if(i > 400) i = 0.0;
			usleep(500);
			XFlush(dpy);
		}
	}
}
