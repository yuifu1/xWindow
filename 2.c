#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>

#define BORDER 2
#define WIDTH  500
#define HIGHT 500

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
	Window w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HIGHT, BORDER, black, white);
	GC gc = XCreateGC(dpy, w, 0, NULL);

	XMapWindow(dpy, w);

	Colormap cmap = DefaultColormap(dpy, 0);             /* カラーマップ構造体 */
	XColor c0, red, gray, green;              /* カラー構造体 */
	XAllocNamedColor(dpy, cmap, "red", &red, &c0); /* COLOUR.pixel */
	XAllocNamedColor(dpy, cmap, "gray", &gray, &c0);
	XAllocNamedColor(dpy, cmap, "Light green", &green, &c0);

	XSelectInput(dpy, w, ExposureMask);

	XEvent event;

	int x = 100;
	int y = 100;
	int r = 50;

	XPoint center = {x+(r/2), y+(r/2)};

	// float rad = PI / 2.0;
	// float rad = 0;
	// float deg = (180.0 * rad) / PI;
	float deg = 0;

	while (1) {

		if(XEventsQueued(dpy, QueuedAfterReading)){
			XNextEvent(dpy, &event);
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

			XSetForeground(dpy, gc, black);// 外接正角形
			XDrawRectangle(dpy, w, gc, x, y, r, r); 
			XDrawLine(dpy, w, gc, x+(r/2), y, x+(r/2), y+r);

			deg += 0.03;
			if(deg > 360) deg = 0.0;
			usleep(500);
			XFlush(dpy);

		}

		
	}

	// while(1){

	// 	XNextEvent(dpy, &event);

	// 	switch (event.type) {
	// 		case Expose:
	// 			// 再描画処理


	// 			draw_ball_full(dpy, w, gc,
    //                100, 100, 300, rad,
    //                red.pixel,
    //                gray.pixel,
    //                black,
    //                white);



	// 			// XSetForeground(dpy, gc, red.pixel); //ippannka
	// 			// XFillArc(dpy, w, gc, x, y, r, r-(r/15), 0, 180*64);

	// 			// XSetForeground(dpy, gc, gray.pixel);
	// 			// XFillArc(dpy, w, gc, x, y+(r/15), r, r-(r/15), 180*64, 180*64);

	// 			// XSetForeground(dpy, gc, black);
	// 			// XFillRectangle(dpy, w, gc, x+(r/30), y+(r/2)-(r/30), r-(r/15), r/15);

	// 			// XSetForeground(dpy, gc, black);
	// 			// XFillArc(dpy, w, gc, x+(r/2)-(r/10), y+(r/2)-(r/10), r/5, r/5, 0, 360*64);

	// 			// XSetForeground(dpy, gc, white);
	// 			// XFillArc(dpy, w, gc, x+(r/2)-(r/10)+(r/60), y+(r/2)-(r/10)+(r/60), (r/5)-(r/30), (r/5)-(r/30), 0, 360*64);



	// 			// XSetForeground(dpy, gc, red.pixel); normal
	// 			// XFillArc(dpy, w, gc, 100, 100, 300, 280, 0, 180*64);

	// 			// XSetForeground(dpy, gc, gray.pixel);
	// 			// XFillArc(dpy, w, gc, 100, 120, 300, 280, 180*64, 180*64);

	// 			// XSetForeground(dpy, gc, black);
	// 			// XFillRectangle(dpy, w, gc, 110, 240, 280, 20);

	// 			// XSetForeground(dpy, gc, black);
	// 			// XFillArc(dpy, w, gc, 215, 215, 70, 70, 0, 360*64);

	// 			// XSetForeground(dpy, gc, white);
	// 			// XFillArc(dpy, w, gc, 220, 220, 60, 60, 0, 360*64);



	// 			XSetForeground(dpy, gc, black);//外接正角形
	// 			XDrawRectangle(dpy, w, gc, x, y, r, r); 
	// 			XDrawLine(dpy, w, gc, x+(r/2), y, x+(r/2), y+r);

	// 		break;
	// 		case KeyPress:
	// 			// キー入力処理
	// 		break;
	// 		case ButtonPress:
	// 			// ボタン押下処理
	// 		break;
	// 	}

	// }
}

