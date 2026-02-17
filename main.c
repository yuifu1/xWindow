#include <stdio.h>
#include <math.h>     // cc -lm
#include <X11/Xlib.h> // cc -l X11
// #include <X11/Xutil.h>
#include <unistd.h>
#include <stdbool.h>

#define BORDER 3
#define WIDTH  1000
#define HEIGHT 700

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

#define PI 3.1415926535

#define g 9.80665
#define e 0.88

#define t 0.1

#define MAX 5
#define WALL_MAX 10

unsigned long white, black, red, gray;

typedef struct {
	float x;
	float y;
} Vector;

typedef struct {
	Vector pos1;
	Vector pos2;
} Wall;

typedef struct {
	Vector center; // 中心座標
	Vector v; // 速度ベクトル
	Vector a; // 加速度ベクトル
	float r_speed; // 回転速度
	float r; // 半径
	float rad; // 角度
	float mass; // 質量
} Ball;

int last = 0;
Ball placed_balls[MAX];

int wall_last = 0;
Wall placed_walls[WALL_MAX];

Ball* addBall(const Vector center, const Vector v, const Vector a, const float r_speed, const float r, const float rad, const float mass) {
	const Ball ball = {
		center,
		v,
		a,
		r_speed,
		r,
		rad,
		mass
	};
	if (++last >= MAX) last = 0;
	placed_balls[last] = ball;
	return placed_balls + last;
}

Wall* addWall(const Vector pos1, const Vector pos2) {
	const Wall wall = {pos1, pos2};
	if (++wall_last >= WALL_MAX) wall_last = 0;
	placed_walls[wall_last] = wall;
	return placed_walls + wall_last;
}

void rotate_point(Vector *p, const Vector center, const float rad) {
    const float dx = p->x - center.x;
    const float dy = p->y - center.y;
    p->x = dx * cosf(rad) - dy * -sinf(rad) + center.x;
    p->y = dx * -sinf(rad) + dy * cosf(rad) + center.y;
}

void DrawBall(Display *dpy, const Window w, GC gc, Ball *ball) {

	if(ball->r <= 0) return;

	const Vector pos = {ball->center.x-(ball->r/2), ball->center.y-(ball->r/2)};
	const double deg = (180*ball->rad) / PI;

	XSetForeground(dpy, gc, red);
	XFillArc(dpy, w, gc, (int) pos.x, (int) pos.y, (int) ball->r, (int) ball->r, (int) (deg*64.0), 180*64);

	XSetForeground(dpy, gc, gray);

	double round = 180 + deg;
	if(round > 360) round -= 360;
	XFillArc(dpy, w, gc, (int) pos.x, (int) pos.y, (int) ball->r, (int) ball->r, (int) (round*64.0), 180*64);

	XSetForeground(dpy, gc, black);

	Vector vpoint[4];
	vpoint[0] = (Vector){pos.x, pos.y+(ball->r/2)-(ball->r/30)};
	vpoint[1] = (Vector){pos.x+ball->r, vpoint[0].y};
	vpoint[2] = (Vector){vpoint[1].x, vpoint[0].y+(ball->r/15)};
	vpoint[3] = (Vector){vpoint[0].x, vpoint[2].y};

	for(int i = 0; i < 4; i++){
		rotate_point(&vpoint[i], ball->center, ball->rad);
	}

	XPoint point[4];
	for(int i = 0; i < 4; i++){
		point[i].x = (short) vpoint[i].x;
		point[i].y = (short) vpoint[i].y;
	}

	XFillPolygon(dpy, w, gc, point, 4, Convex, CoordModeOrigin);

	XSetForeground(dpy, gc, white);// 中心内側円
	XFillArc(dpy, w, gc, (int) (pos.x+(ball->r* 5/12)), (int) (pos.y+(ball->r* 5/12)), (int) ((ball->r/5)-(ball->r/30)), (int) ((ball->r/5)-(ball->r/30)), 0, 360*64);

	XSetForeground(dpy, gc, black);// 外接正角形
	XDrawRectangle(dpy, w, gc, (int) pos.x, (int) pos.y, (int) ball->r, (int) ball->r);
	// XDrawLine(dpy, w, gc, pos.x+(ball->r/2), pos.y, pos.x+(ball->r/2), pos.y+ball->r);

	if(ball->rad > 2*PI) ball->rad -= (float) (2*PI);

}

void DrawWall(Display *dpy, const Window w, GC gc, Wall *wall) {
	if(wall->pos1.x == -1) return;
	XSetForeground(dpy, gc, black);
	XDrawLine(dpy, w, gc, (int) wall->pos1.x, (int) wall->pos1.y, (int) wall->pos2.x, (int) wall->pos2.y);
}

void DrawWalls(Display *dpy, const Window w, GC gc) {
	for(int i = 0; i < WALL_MAX; ++i)
		DrawWall(dpy, w, gc, placed_walls+i);
}

void DrawBalls(Display *dpy, const Window w, GC gc) {
	for(int i = 0; i < MAX; ++i)
		DrawBall(dpy, w, gc, placed_balls+i);
}

float dot(const Vector a, const Vector b) {
	return a.x*b.x + a.y+b.y;
}

void reflect(const Ball before, Ball *after) {
	for(int i = 0;i < WALL_MAX; ++i) {
		const Wall w = placed_walls[i];
		if(w.pos1.x == -1) continue;
		const float dx = w.pos2.x - w.pos1.x;
		const float dy = w.pos2.y - w.pos1.y;
		float a = 0; // x = c
		if ((int) dx != 0) {
			a = dy / dx;// y = ax+c
		} // ax - y + c = 0
		// const float c = w.pos1.y - a*w.pos1.x;
		const float under = sqrtf(powf(a, 2) + 1);
		const float k = under*after->r/2;
		/*
		 * todo: 法線ベクトルの向きの考慮
		 * 壁の大きさball.r/2の法線ベクトルを円の中心の位置ベクトルに足して、接点の位置ベクトルを出す。
		 */
		const Vector p1 = {before.center.x + a*k, before.center.y - k}; // 壁との接点の位置ベクトル
		const Vector p2 = {after->center.x + a*k, after->center.y - k};
		// printf("%f, %f\n", p1.y, p2.y);
		const float m = dx*(p1.y - w.pos1.y) - dy*(p1.x - w.pos1.x);
		const float n = dx*(p2.y - w.pos1.y) - dy*(p2.x - w.pos1.x);
		if(m*n < 0) {
			printf("reflect\n");
			if(m > 0) { // 床
				// printf("called\n");
			} else if (m < 0){
				// after->center.y = p2.y + after->r/2;
			}
			// after->v.y *= (float) -e;
		}

		// const float d_b = (float) (fabsf(a*before.center.x - before.center.y + c) / under);
		// const float d_a = (float) (fabsf(a*after->center.x - after->center.y + c) / under);

		// const float d = (float)(fabsf(vec.y * ball->center.x - vec.x * ball->center.y + w.pos2.x * w.pos1.y - w.pos1.x * w.pos2.y)/(sqrt(pow(vec.x, 2) + pow(vec.y, 2))));
		// printf("%4.2f, %4.2f, %4.2f\n", a, c, d);
		// if(d <= ball->r) {
		// 	// printf("reflect\n");
		// 	const float y = a*ball->center.x + c;
		// 	if(y > ball->center.y) { // 床
		// 		// printf("called\n");
		// 		printf("%f, %f, %f\n", y, ball->center.y, d);
		// 		// ball->center.y = ball->center.y - (ball->r/2 - (y - ball->center.y));
		// 		// ball->v.y *= (float) -e;
		// 	} else if (y < ball->center.y){
		// 		// ball->center.y = y + ball->r/2;
		// 		// ball->v.y *= (float) -e;
		// 	}
		//
		// 	// if(ball->center.y + ball->r/2 >= y[0] && ball->center.y - ball->r/2 < y[1]) {
		// 	// 	ball->center.y = y[0] - ball->r/2;
		// 	// 	ball->v.y *= (float) -e;
		// 	// }
		// 	//
		// 	// if(ball->center.y + ball->r/2 < y[0] && ball->center.y - ball->r/2 >= y[1]) {
		// 	// 	ball->center.y = y[1] + ball->r/2;
		// 	// 	ball->v.y *= (float) -e;
		// 	// }
		//
		// }
	}
}

int main(int argc, char **argv) {

	for (int i = 0; i < MAX; i++) {
		placed_balls[i] = (Ball) {(Vector) {0,0}, (Vector) {0,0}, (Vector) {0,0}, 0, 0,  0, 0};
	}

	for (int i = 0; i < WALL_MAX; i++) {
		placed_walls[i] = (Wall) {(Vector) {-1, -1}, (Vector) {-1, -1}};
	}

	Display *dpy = XOpenDisplay("");
	const int screen = DefaultScreen(dpy);
	white = WhitePixel(dpy, screen);
	black = BlackPixel(dpy, screen);
	const Window root = DefaultRootWindow(dpy);
	const Window w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HEIGHT, BORDER, black, white);
	XMapWindow(dpy, w);
	GC gc = XCreateGC(dpy, w, 0, NULL);

	const Colormap cmap = DefaultColormap(dpy, 0);       /* カラーマップ構造体 */
	XColor c0, red_, gray_, green_, blue_;             /* カラー構造体 */
	XAllocNamedColor(dpy, cmap, "red", &red_, &c0); /* COLOUR.pixel */
	XAllocNamedColor(dpy, cmap, "gray", &gray_, &c0);
	XAllocNamedColor(dpy, cmap, "Light green", &green_, &c0);
	XAllocNamedColor(dpy, cmap, "blue", &blue_, &c0);

	red = red_.pixel;
	gray = gray_.pixel;

	const Window exit = XCreateSimpleWindow(dpy, w, 10, 10, 50, 30, BORDER, black, white); // exit window
	XMapSubwindows(dpy, w);

	XSelectInput(dpy, w, PointerMotionMask | ButtonPressMask | ButtonReleaseMask);
	XSelectInput(dpy, exit, ButtonPressMask);

	XEvent event;

	Vector start = {-1, -1};
	Vector mouse = {-1, -1};

	int isWriting = 0;

	addWall((Vector) {0, 0}, (Vector) {WIDTH, 0});			// 上
	addWall((Vector) {0, 0}, (Vector) {0, HEIGHT});			// 左
	addWall((Vector) {WIDTH, 0}, (Vector) {WIDTH, HEIGHT});     // 右
	addWall((Vector) {0, HEIGHT}, (Vector) {WIDTH, HEIGHT});// 下

	while(1){

		// XDrawString(dpy, exit, gc, 4, 10,"Exit", 4);

		if(XEventsQueued(dpy, QueuedAfterReading)){
			XNextEvent(dpy, &event);
			switch(event.type){
				case MotionNotify:
					if(isWriting == 1){
						mouse.x = (float) event.xmotion.x;
						mouse.y = (float) event.xmotion.y;
						rotate_point(&mouse, start, PI);
						// if(sqrt(pow(mouse.x - start.x, 2) + pow(mouse.y - start.y, 2)) >= 100) {
						// 	continue;
						// }
					}
				break;
				case ButtonPress:
					if(event.xany.window == exit) return 0;
					if(event.xany.window == w) {
						switch(event.xbutton.button) {
							case MOUSE_LEFT:
								start.x = (float) event.xbutton.x;
								start.y = (float) event.xbutton.y;
								const float r = 50; // 半径
								const float mass = 5;
								addBall(start, (Vector) {0, 0}, (Vector) {0, 0}, 0, r, 0, mass);
								isWriting = 1;
							break;
							case MOUSE_RIGHT:
								// start.x = (float) event.xbutton.x;
								// start.y = (float) event.xbutton.y;
							break;
							default:
								continue;
						}
					}
				break;
				case ButtonRelease:
					if(event.xany.window == w) {
						isWriting = 0;
						Vector end = {(float) event.xbutton.x, (float) event.xbutton.y};
						rotate_point(&end, start, PI);
						placed_balls[last].v.x = (end.x - start.x) / placed_balls[last].mass;
						placed_balls[last].v.y = (end.y - start.y) / placed_balls[last].mass;
						placed_balls[last].r_speed = placed_balls[last].v.x / 80; // RANDOM 100
						placed_balls[last].a.y = g;
						mouse.x = -1;
					}
				break;
				default:
					continue;
			}
		} else {

			usleep(5000);
			XClearWindow(dpy, w);

			for(int i = 0; i < MAX; ++i) {

				Ball *ball = placed_balls+i;
				if(ball->r <= 0) continue;
				const Ball before = *ball;
				// printf("%d, ", (int)ball->v.y);

				ball->v.x += ball->a.x * (float)t;
				ball->v.y += ball->a.y * (float)t;
				ball->center.x += ball->v.x * (float)t;
				ball->center.y += ball->v.y * (float)t;
				ball->rad -= ball->r_speed * (float)t;

				reflect(before, ball);
				// if((*ball)) {
				// 	printf("called\n");
				// }
				// if(ball->center.x - ball->r/2 < 0 && ball->v.x < 0) { // 左端
				// 	ball->v.x *= (float) -e;
				// 	ball->r_speed *= (float) -e;
				// }
				// if(ball->center.x + ball->r/2 > WIDTH && ball->v.x > 0) { // 右端
				// 	ball->v.x *= (float) -e;
				// 	ball->r_speed *= (float) -e;
				// }
				// if(ball->center.y - ball->r/2 < 0 && ball->v.y < 0) { // 上端
				// 	ball->v.y *= (float) -e;
				// }
				// if(ball->center.y + ball->r/2 > HEIGHT && ball->v.y > 0) { // 下端
				// 	ball->center.y = HEIGHT - ball->r/2;
				// 	ball->v.y *= (float) -e;
				// }

				// printf("%d\n", (int)ball->v.y);

			}

			DrawBalls(dpy, w, gc);
			DrawWalls(dpy, w, gc);

			if(isWriting == 1 && mouse.x != -1) {
				XSetForeground(dpy, gc, black);
				XDrawLine(dpy, w, gc, (int) start.x, (int) start.y, (int) mouse.x, (int) mouse.y);
			}

			XFlush(dpy);

		}
	}
}