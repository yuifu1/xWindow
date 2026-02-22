#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include <X11/Xft/Xft.h>
#include <unistd.h>

#define BORDER 2
#define WIDTH 500
#define HIGHT 350

#define BALL 50            //初期の円の数と
#define MAX_BALL 1000      //最大個数
#define MAX_Speed 3        //3程度を推奨
#define R 8.3

void ball(int, int, int);

int x_down = 100, y_down = 100; //箱の下限
const int x_up = 800, y_up = 800; //箱の上限
const int large = 20; //円の大きさ
int mode = 1;
int T = 273;
int mol = BALL;

int x_prot[MAX_BALL + 1], y_prot[MAX_BALL + 1], x_vector[MAX_BALL + 1], y_vector[MAX_BALL + 1];
int command[10];

Display* dpy;
Window w;
Window root;
int screen;
unsigned long black, white;
GC gc;

void main(int argc, char** argv) {
	char p_value[100], v_value[100], n_value[100], r_value[100], t_value[100];


	for(int i = 0; i < MAX_BALL + 1; i++) {
		x_prot[i] = rand() % (x_up - x_down - large) + x_down;
		y_prot[i] = rand() % (y_up - y_down - large) + y_down;
		if(i % 2 == 1) {
			x_vector[i] = 1;
			y_vector[i] = 1;
		} else {
			x_vector[i] = -1;
			y_vector[i] = -1;
		}
	}

	dpy = XOpenDisplay("");
	root = DefaultRootWindow(dpy);
	screen = DefaultScreen(dpy);

	white = WhitePixel(dpy, screen);
	black = BlackPixel(dpy, screen);


	w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HIGHT, BORDER, black, white);

	gc = XCreateGC(dpy, w, 0, NULL);

	XftDraw* xftdraw;
	XftColor xftcolor;
	XftFont* xftfont;

	xftdraw = XftDrawCreate(dpy, w,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen));

	XRenderColor xrcolor = {0, 0, 0, 0xffff};
	XftColorAllocValue(dpy, DefaultVisual(dpy, screen),DefaultColormap(dpy, screen), &xrcolor, &xftcolor);

	xftfont = XftFontOpenName(dpy, screen, "Sans:size=32");

	XMapWindow(dpy, w);

	XSelectInput(dpy, w,KeyPressMask | ButtonPressMask);
	XEvent event;

	while(1) {
		//イベント処理
		while(XPending(dpy)) {
			XNextEvent(dpy, &event);

			switch(event.type) {
				case KeyPress:
					if(event.xkey.keycode == 39) //lを押す
						if(y_up - y_down <= large + (MAX_Speed * T / 100) * 2 || x_up - x_down <= large + (MAX_Speed * T
							/ 100) * 2);
						else {
							y_down++;
							x_down++;
						}
					else if(event.xkey.keycode == 46) //sを押す
						if(y_down <= 0 || x_down <= 0);
						else {
							y_down--;
							x_down--;
						}

					else if(event.xkey.keycode == 58) {
						//mを押す
						if(mol >= MAX_BALL);
						else mol++;
					} else if(event.xkey.keycode == 41) {
						//fを押す
						if(mol <= 0);
						else mol--;
					} else if(event.xkey.keycode == 43) {
						//hを押す
						T++;
						if(y_up - y_down <= large + (MAX_Speed * T / 100) * 2 || x_up - x_down <= large + (MAX_Speed * T
							/ 100) * 2) {
							y_down = y_up - large - (MAX_Speed * T / 100) * 2;
							x_down = x_up - large - (MAX_Speed * T / 100) * 2;
						}
					} else if(event.xkey.keycode == 54) {
						//cを押す
						if(T <= 1);
						else T--;
					}

					for(int i = 9; i > 0; i--) {
						command[i] = command[i - 1];
					}
					// 挿入
					command[0] = event.xkey.keycode;

					if(command[0] == 38 && //上上下下左右左右BASを押す
						command[1] == 56 &&
						command[2] == 114 &&
						command[3] == 113 &&
						command[4] == 114 &&
						command[5] == 113 &&
						command[6] == 116 &&
						command[7] == 116 &&
						command[8] == 111 &&
						command[9] == 111) {
						mode = 2;
					} else mode = 1;
					break;
				default:
					break;
			}
		}


		//背景クリア
		XClearWindow(dpy, w);

		//枠の描画
		XSetForeground(dpy, gc, black);
		XDrawRectangle(dpy, w, gc, x_down, y_down, x_up - x_down, y_up - y_down);

		//それぞれの値の出力
		sprintf(p_value, "P = %.2f*10**3 [Pa]", (float)(mol * R * T / ((x_up - x_down) * (y_up - y_down))));
		XftDrawStringUtf8(xftdraw, &xftcolor, xftfont, x_up + 100, y_up - 200, (XftChar8*)p_value, strlen(p_value));

		sprintf(v_value, "V = %d [m**3]", (int)(x_up - x_down) * (y_up - y_down));
		XftDrawStringUtf8(xftdraw, &xftcolor, xftfont, x_up + 100, y_up - 150, (XftChar8*)v_value, strlen(v_value));

		sprintf(n_value, "n = %d [mol]", (int)(mol));
		XftDrawStringUtf8(xftdraw, &xftcolor, xftfont, x_up + 100, y_up - 100, (XftChar8*)n_value, strlen(n_value));

		sprintf(r_value, "R = %.1f [Pa*m**3/mol*K]", (float)(R));
		XftDrawStringUtf8(xftdraw, &xftcolor, xftfont, x_up + 100, y_up - 50, (XftChar8*)r_value, strlen(r_value));

		sprintf(t_value, "T = %d [K]", (int)(T));
		XftDrawStringUtf8(xftdraw, &xftcolor, xftfont, x_up + 100, y_up, (XftChar8*)t_value, strlen(t_value));

		//ボールの更新
		for(int i = 0; i < MAX_BALL + 1; i++) {
			if(x_prot[i] < x_down + MAX_Speed * T / 100) {
				x_prot[i] = x_down + MAX_Speed * T / 100;
				x_vector[i] *= -1;
			}
			if(x_prot[i] > x_up - large - MAX_Speed * T / 100) {
				x_prot[i] = x_up - large - MAX_Speed * T / 100;
				x_vector[i] *= -1;
			}
			if(y_prot[i] < y_down + MAX_Speed * T / 100) {
				y_prot[i] = y_down + MAX_Speed * T / 100;
				y_vector[i] *= -1;
			}
			if(y_prot[i] > y_up - large - MAX_Speed * T / 100) {
				y_prot[i] = y_up - large - MAX_Speed * T / 100;
				y_vector[i] *= -1;
			}
			ball(x_vector[i], y_vector[i], i);
		}

		XFlush(dpy);
		usleep(3000);
	}
}

void ball(int x_vector, int y_vector, int number) {
	char message[] = "DVD";

	x_prot[number] += x_vector * (rand() % (int)(MAX_Speed * T / 100 + 1));
	y_prot[number] += y_vector * (rand() % (int)(MAX_Speed * T / 100 + 1));

	if(mode == 1) {
		if(number < mol)XDrawArc(dpy, w, gc, x_prot[number], y_prot[number], large, large, 0, 360 * 64);
	} else {
		if(number < mol)XDrawArc(dpy, w, gc, x_prot[number], y_prot[number], large, large, 0, 360 * 64);
		else if(number == mol) XDrawString(dpy, w, gc, x_prot[number], y_prot[number], message, strlen(message));
	}
}
