#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define BORDER 2
#define WIDTH  960
#define HIGHT  720

double camX=0,camY=0,camZ=0,angleX=0,angleY=0,angleZ=0,angleXsin,angleXcos,angleYsin,angleYcos,angleZsin,angleZcos;
double dy = 0;

const double FocalLength=320;
int disp[WIDTH][HIGHT];
double z_clip[WIDTH][HIGHT];


int draw_or_not(int NnegZ,double k,double l){
    double sum=k+l;
    switch(NnegZ){
        case 0b111:
            if(0<sum&&sum<1&&k>0&&l>0) return 1;
            else return 0;
        case 0b110:
        case 0b101:
        case 0b011:
            if(1<sum&&k>0&&l>0) return 1;
            else return 0;
        case 0b100:
        case 0b010:
        case 0b001:
            if(k<0&&l<0) return 1;
            else return 0;
    }
    if(NnegZ==0b111){
        if(0<sum&&sum<1&&k>0&&l>0) return 1;
        else return 0;
    }else if(NnegZ){
        if(1<sum) return 1;
        else return 0;
    }else if(NnegZ%3==1){
        if(sum<0) return 1;
        else return 0;
    }
}
void triangle(double X1,double Y1,double Z1,double X2,double Y2,double Z2,double X3,double Y3,double Z3,int color){//3頂点
    double Xa,Ya,Za,Xb,Yb,Zb,Xc,Yc,Zc,Xab,Yab,Zab,Xac,Yac,Zac,X=X1-camX,Y=Y1-camY,Z=Z1-camZ,k,l,z_cl;
    int NnegZ=0;//Number of Negative Z
    Xa = angleYcos*(angleZsin*Y + angleZcos*X) - angleYsin*Z;
    Ya = angleXsin*(angleYcos*Z + angleYsin*(angleZsin*Y + angleZcos*X)) + angleXcos*(angleZcos*Y - angleZsin*X);
    Za = angleXcos*(angleYcos*Z + angleYsin*(angleZsin*Y + angleZcos*X)) - angleXsin*(angleZcos*Y - angleZsin*X);
    X=X2-camX; Y=Y2-camY; Z=Z2-camZ;
    Xb = angleYcos*(angleZsin*Y + angleZcos*X) - angleYsin*Z;
    Yb = angleXsin*(angleYcos*Z + angleYsin*(angleZsin*Y + angleZcos*X)) + angleXcos*(angleZcos*Y - angleZsin*X);
    Zb = angleXcos*(angleYcos*Z + angleYsin*(angleZsin*Y + angleZcos*X)) - angleXsin*(angleZcos*Y - angleZsin*X);
    X=X3-camX; Y=Y3-camY; Z=Z3-camZ;
    Xc = angleYcos*(angleZsin*Y + angleZcos*X) - angleYsin*Z;
    Yc = angleXsin*(angleYcos*Z + angleYsin*(angleZsin*Y + angleZcos*X)) + angleXcos*(angleZcos*Y - angleZsin*X);
    Zc = angleXcos*(angleYcos*Z + angleYsin*(angleZsin*Y + angleZcos*X)) - angleXsin*(angleZcos*Y - angleZsin*X);
    if (Za==0) Za=0.001; if (Zb==0) Zb=0.001; if (Zc==0) Zc=0.001;
    //NnegZは負になるZ座標の組み合わせ。3.4.7.そのまま　2.5.ab交換　1.6.ac交換　0.描画しない
    if(Za>0) NnegZ++; NnegZ*=2; if(Zb>0) NnegZ++; NnegZ*=2; if(Zc>0) NnegZ++;
    if(NnegZ!=0){
        double Xs=Xa,Ys=Ya,Zs=Za;
        if(NnegZ%5==1){
            Xa=Xc; Ya=Yc; Za=Zc;
            Xc=Xs; Yc=Ys; Zc=Zs;
        }else if(NnegZ%3==2){
            Xa=Xb; Ya=Yb; Za=Zb;
            Xb=Xs; Yb=Ys; Zb=Zs;
        }
        Xab=Xb-Xa; Yab=Yb-Ya; Zab=Zb-Za; Xac=Xc-Xa; Yac=Yc-Ya; Zac=Zc-Za;
        double Xa2=Xa*FocalLength/Za, Ya2=Ya*FocalLength/Za, Xb2=Xb*FocalLength/Zb, Yb2=Yb*FocalLength/Zb, Xc2=Xc*FocalLength/Zc, Yc2=Yc*FocalLength/Zc;
        double Xab2=Xb2-Xa2, Yab2=Yb2-Ya2, Xac2=Xc2-Xa2, Yac2=Yc2-Ya2;

        double A=Yab*Zac-Zab*Yac, B=Zab*Xac-Xab*Zac, C=Xab*Yac-Yab*Xac;
        double D=A*Xa+B*Ya+C*Za;
        z_cl=D/(A*X/FocalLength+B*Y/FocalLength+C);

        for(int i=0;i<WIDTH;i++){
            for(int j=0;j<HIGHT;j++){
                X=i-WIDTH/2; Y=HIGHT/2-j;
                k=((X-Xa2)*Yac2-Xac2*(Y-Ya2))/(Xab2*Yac2-Xac2*Yab2);
                l=((X-Xa2)*Yab2-Xab2*(Y-Ya2))/(Xac2*Yab2-Xab2*Yac2);
                if(draw_or_not(NnegZ,k,l)==1){
                    z_cl=D/(A*X/FocalLength+B*Y/FocalLength+C);
                    if(0<z_cl && z_cl<z_clip[i][j]){
                        z_clip[i][j]=z_cl;
                        disp[i][j]=color;
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv){
    char keymap[32];
    Display *dpy;
    Window w, root;
    int screen,i,j;
    unsigned long black, white;
    GC gc;
    XEvent e;

    dpy = XOpenDisplay(NULL);
    root = DefaultRootWindow(dpy);
    screen = DefaultScreen(dpy);
    white = WhitePixel(dpy, screen);
    black = BlackPixel(dpy, screen);

    w = XCreateSimpleWindow(
        dpy, root, 100, 100,
        WIDTH, HIGHT, BORDER,
        black, white);

    gc = XCreateGC(dpy, w, 0, NULL);
    XSetForeground(dpy, gc, black);

    XSelectInput(dpy, w, ExposureMask);
    XMapWindow(dpy, w);

    XImage *ximage;
    int format, depth, bitmap_pad;

    depth = DefaultDepth(dpy, screen);

    if (depth == 1) {format = XYPixmap; bitmap_pad = 32;}
    else if (depth == 8) {format = ZPixmap; bitmap_pad = 8;}
    else {format = ZPixmap; bitmap_pad = 32;}

    ximage = XCreateImage(dpy, DefaultVisual(dpy, screen), depth, format, 0, 0, WIDTH, HIGHT, bitmap_pad, 0);
    ximage->data = (char *)malloc(ximage->bytes_per_line * HIGHT);

    while(1){
        //XClearWindow(dpy, w);
        KeyCode kw = XKeysymToKeycode(dpy, XK_w);
        KeyCode ks = XKeysymToKeycode(dpy, XK_s);
        KeyCode ka = XKeysymToKeycode(dpy, XK_a);
        KeyCode kd = XKeysymToKeycode(dpy, XK_d);
        KeyCode ke = XKeysymToKeycode(dpy, XK_e);
        KeyCode kq = XKeysymToKeycode(dpy, XK_q);
        KeyCode kr = XKeysymToKeycode(dpy, XK_r);

        KeyCode aU = XKeysymToKeycode(dpy, XK_Up);
        KeyCode aD = XKeysymToKeycode(dpy, XK_Down);
        KeyCode aL = XKeysymToKeycode(dpy, XK_Left);
        KeyCode aR = XKeysymToKeycode(dpy, XK_Right);

        XQueryKeymap(dpy, keymap);

        if(keymap[kr >> 3] & (1 << (kr & 7))){
            camX=0; camY=0; camZ=0; angleX=0; angleY=0; angleZ=0;
        }
        if(keymap[kw >> 3] & (1 << (kw & 7))){
            camX+=angleYsin;
            camZ+=angleYcos;
        }
        if(keymap[ks >> 3] & (1 << (ks & 7))){
            camX-=angleYsin;
            camZ-=angleYcos;
        }
        if(keymap[ka >> 3] & (1 << (ka & 7))){
            camX-=angleYcos;
            camZ+=angleYsin;
        }
        if(keymap[kd >> 3] & (1 << (kd & 7))){
            camX+=angleYcos;
            camZ-=angleYsin;
        }
        if(keymap[ke >> 3] & (1 << (ke & 7))){
            camY+=1;
            dy = 10;
        }
        if(keymap[kq >> 3] & (1 << (kq & 7))){
            camY-=1;
        }
        if(keymap[aU >> 3] & (1 << (aU & 7))){
            angleX-=0.1;
        }
        if(keymap[aD >> 3] & (1 << (aD & 7))){
            angleX+=0.1;
        }
        if(keymap[aR >> 3] & (1 << (aR & 7))){
            angleY+=0.1;
        }
        if(keymap[aL >> 3] & (1 << (aL & 7))){
            angleY-=0.1;
        }

        if (0 < camY + dy) {
            camY += dy;
            dy -= 1;
        } else {
            camY = 0;
            dy = 0;
        }

        angleXsin=sin(angleX); angleXcos=cos(angleX); angleYsin=sin(angleY); angleYcos=cos(angleY);
        angleZsin=sin(angleZ); angleZcos=cos(angleZ);
        for(i=0;i<WIDTH;i++){//dispをきれいにする
            for(j=0;j<HIGHT;j++){
                disp[i][j]=0x000000;
                z_clip[i][j]=114514;
            }
        }
        //triangle(0,300,30,-20,0,40,20,0,40,0xff0000);
        triangle(-20,40,30,-20,-10,30,20,-10,30,0xff0000);
        triangle(-20,40,30,20,40,30,20,-10,30,0xff0000);

        triangle(-20,40,30,-20,40,60,20,40,60,0x00ff00);
        triangle(20,40,60,-20,40,30,20,40,30,0x00ff00);

        triangle(20,40,30,20,-10,30,20,-10,60,0x0000ff);
        triangle(20,40,30,20,40,60,20,-10,60,0x0000ff);

        disp[WIDTH/2][HIGHT/2]=0xffffff;

        for(i=0;i<WIDTH;i++){
            for(j=0;j<HIGHT;j++){
                XPutPixel(ximage,i,j,disp[i][j]);
            }
        }
        XPutImage(dpy,w,gc,ximage,0,0,0,0,WIDTH,HIGHT);
    }
}