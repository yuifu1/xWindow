/* xdraw.c */
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>

#define BORDER 2
#define WIDTH  500
#define HIGHT 1000

int main(int argc,char **argv) {

	Display *dpy;
	Window w;
	Window root;
	int screen;
	unsigned long black, white;
	GC gc;

	char moji[256];            /* 日本語メッセージが入る場所 */

	Colormap cmap;             /* カラーマップ構造体 */
	XColor c0,c1;              /* カラー構造体 */

	dpy = XOpenDisplay("");

	root = DefaultRootWindow(dpy);
	screen = DefaultScreen(dpy);
	white = WhitePixel(dpy, screen);
	black = BlackPixel(dpy, screen);
	cmap = DefaultColormap(dpy,0);  /* カラーマップを得る */
	XAllocNamedColor(dpy, cmap, "green", &c1, &c0);  /* c1.pixel */

	w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HIGHT, BORDER, black, white);

	gc = XCreateGC(dpy, w, 0, NULL);

	setlocale(LC_ALL,""); //1st. lacaleをセットして（日）
	XFontSet fs;             /* XFontsetで日本語フォントを選ぶ（日） */
	char **miss,*def;
	int n_miss;              /* 変数として宣言しておく（日） */
	fs = XCreateFontSet(dpy, "-*-*-medium-r-normal-*-16-*", &miss, &n_miss, &def); //少なくともdpyに値がはいってからでないと駄目(日)

	XMapWindow(dpy, w);

	while(1){

		XSetForeground (dpy,gc,black);  /* blackにする */

		XDrawLine(dpy, w, gc, 0, 150, 250, 0);
		XDrawLine(dpy, w, gc, 250, 0, 500, 150); // roof

		XDrawRectangle(dpy, w, gc, 70, 108, 360, 360);// house box

		XDrawRectangle(dpy, w, gc, 100, 200, 120, 120); // window
		XDrawLine(dpy, w, gc, 100, 200, 220, 320);
		XDrawLine(dpy, w, gc, 100, 320, 220, 200);

		sprintf(moji, "皇居");// name
		XmbDrawString(dpy, w, fs, gc, 300, 300, moji, strlen(moji));



	}
}

