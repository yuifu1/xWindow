#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <math.h>   /* use math lib*/
#include <unistd.h> // usleepのために必要
#define PI 3.14159265358979323846
#define airn 1.000293 // 空気の屈折率
// #define moleden 25000000000000000000000000 //分子密度
const double moleden = 2.545e25;
#define neipia 2.71828182846
double rambe(double lamb, double x);
double reiri(double lamb);
double taikikyori(double x);
double ganma(int *r, int *g, int *b);
int changecolor_active(Display *dpy, Window w, int r, int g, int b);
int tyousei255(int x);
int changecolor_sun(Display *dpy, GC gc, int redr, int greenr, int bluer);
double clamp01(double x);
double smoothstep(double edge0, double edge1, double x);

int main()
{

    Display *dpy;
    Window w;
    Window root;
    int screen;
    GC gc;
    XEvent e;
    int xx, yy, r, g, b, sunx, suny, rr, redrr, greenrr, bluerr;
    double redr, greenr, bluer, red, green, blue;

    xx = 400;
    yy = 300;
    rr = 20; // 太陽の半径

    dpy = XOpenDisplay("");
    root = DefaultRootWindow(dpy);
    screen = DefaultScreen(dpy);

    w = XCreateSimpleWindow(dpy, root, 100, 100, xx, yy, 1, BlackPixel(dpy, screen), WhitePixel(dpy, screen));
    XSelectInput(dpy, w, ExposureMask | PointerMotionMask);
    XMapWindow(dpy, w);
    gc = XCreateGC(dpy, w, 0, NULL);

    double y = 0;
    double bl = 450e-9, gl = 530e-9, rl = 650e-9; // 波長
    sunx = xx / 2;
    suny = yy / 2;

    XMapWindow(dpy, w);

    while (1)
    {
        XNextEvent(dpy, &e);

        if (e.type == MotionNotify)
        {
            y = (100 * abs(yy - e.xmotion.y) / yy) * 100.0; // マウス位置を 0km(地表) 〜 10km(対流圏) 程度にマッピング
            sunx = e.xmotion.x;
            suny = e.xmotion.y;
        }

        redr = rambe(rl, y); // 透過光の計算
        greenr = rambe(gl, y);
        bluer = rambe(bl, y);

        double t = clamp01(y / 10000.0); // 0.0: 地平線, 1.0: 高高度
        double day_t = smoothstep(0.2, 0.8, t);
        double eve_t = 1.0 - day_t;

        // 昼の青空と夕方の暖色をブレンド
        double day_r = 0.35, day_g = 0.6, day_b = 1.0;
        double eve_r = 1.0, eve_g = 0.45, eve_b = 0.15;
        red = day_r * day_t + eve_r * eve_t;
        green = day_g * day_t + eve_g * eve_t;
        blue = day_b * day_t + eve_b * eve_t;

        // 地平線付近は夕方の赤みを強める
        double horizon = 1.0 - t;
        red += 0.25 * eve_t * horizon;
        green += 0.05 * eve_t * horizon;
        blue -= 0.2 * eve_t * horizon;

        red = clamp01(red);
        green = clamp01(green);
        blue = clamp01(blue);

        redrr = (int)(redr * 255);     // 整数になおす
        greenrr = (int)(greenr * 255); // 整数になおす
        bluerr = (int)(bluer * 255);
        ganma(&redrr, &greenrr, &bluerr); // 太陽にもガンマ補正

        r = (int)(red * 255);   // 整数になおす
        g = (int)(green * 255); // 整数になおす
        b = (int)(blue * 255);

        r = tyousei255(r);
        g = tyousei255(g);
        b = tyousei255(b);

        ganma(&r, &g, &b);
        changecolor_active(dpy, w, r, g, b);
        changecolor_sun(dpy, gc, redrr, greenrr, bluerr);
        XFillArc(dpy, w, gc, sunx - rr, suny - rr, 2 * rr, 2 * rr, 0, 360 * 64);
        printf("red = %d, green = %d, blue = %d\n", r, g, b);
        printf("y = %f\n", y);
        XFlush(dpy);
    }
}

double rambe(double lamb, double y)
{ // ランベルト・ベールの法則
    double rambe;
    rambe = exp(-reiri(lamb) * taikikyori(y));
    return rambe;
}

double reiri(double lamb)
{ // レイリーの法則,lamb == 波長
    double reiri;
    reiri = (8 * pow(PI, 3) * (airn * airn - 1) * (airn * airn - 1)) / moleden / pow(lamb, 4) / 3; //*10^-24
    return reiri;
}

double taikikyori(double y)
{ // 大気距離
    double tkyori;
    tkyori = sqrt(6471e2 * 6471e2 - 6371e2 * 6371e2 - 2 * 6371e2 * y);
    return tkyori;
}

double ganma(int *r, int *g, int *b)
{
    *r = (int)(255 * pow(*r / 255.0, 1.0 / 2.2));
    *g = (int)(255 * pow(*g / 255.0, 1.0 / 2.2));
    *b = (int)(255 * pow(*b / 255.0, 1.0 / 2.2));
} // ガンマ補正

int changecolor_active(Display *dpy, Window w, int r, int g, int b)
{
    // RGB 8bitを X11 の Pixel 形式に変換 (0xRRGGBB)
    unsigned long color = (r << 16) | (g << 8) | b;
    XSetWindowBackground(dpy, w, color);
    XClearWindow(dpy, w);
    XFlush(dpy);
    return 0;
}

int changecolor_sun(Display *dpy, GC gc, int redr, int greenr, int bluer)
{
    unsigned long suncolor = (redr << 16) | (greenr << 8) | bluer;
    XSetForeground(dpy, gc, suncolor);
}

double clamp01(double x)
{
    if (x < 0.0)
    {
        return 0.0;
    }
    if (x > 1.0)
    {
        return 1.0;
    }
    return x;
}

double smoothstep(double edge0, double edge1, double x)
{
    double t;
    if (edge0 == edge1)
    {
        return (x < edge0) ? 0.0 : 1.0;
    }
    t = clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

int tyousei255(int x)
{
    if (x > 255)
    {
        x = 255;
    }
    return x;
}