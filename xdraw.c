/* xdraw.c */
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>

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
        char here[] = "I'm here!";
        char moji[256];            /* 日本語メッセージが入る場所 */

        XFontSet fs;             /* XFontsetで日本語フォントを選ぶ（日） */
        char **miss,*def;
        int n_miss;              /* 変数として宣言しておく（日） */

	Colormap cmap;             /* カラーマップ構造体 */
	XColor c0,c1;              /* カラー構造体 */

        setlocale(LC_ALL,"");        /* 1st. lacaleをセットして（日） */

	dpy = XOpenDisplay("");

	root = DefaultRootWindow (dpy);
	screen = DefaultScreen (dpy);
	white = WhitePixel (dpy, screen);
	black = BlackPixel (dpy, screen);
	cmap = DefaultColormap(dpy,0);  /* カラーマップを得る */
	XAllocNamedColor(dpy,cmap,"green",&c1,&c0);  /* c1.pixel */

	w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HIGHT, BORDER, black, white);

	gc = XCreateGC(dpy, w, 0, NULL);

        fs=XCreateFontSet(dpy,"-*-*-medium-r-normal-*-16-*",&miss,&n_miss,&def);
        /* 少なくともdpyに値がはいってからでないと駄目（日） */

	XMapWindow(dpy, w);

	while(1){
        XSetForeground (dpy,gc,black);  /* blackにする */
	XDrawString(dpy, w, gc, WIDTH / 2, HIGHT / 2,here, strlen(here));

        sprintf(moji,"漢字の例");       /* 画面に出す文字列を変数の中に入れる */
        XmbDrawString(dpy, w, fs, gc, 20, 40,moji,strlen(moji));   /* 漢字表示 */
	XDrawPoint(dpy, w, gc,  30, 30);

        XSetForeground (dpy,gc,c1.pixel);  /* greenにする */

	XDrawLine(dpy, w, gc,  33, 30, 300, 200);
	XDrawRectangle(dpy, w, gc,  43, 40, 100, 70);
	XDrawArc(dpy, w, gc,  100, 100, 30, 30, 0, 360*64);

	/* event loop */
	}
}

