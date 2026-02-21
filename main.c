#include <stdio.h>
#include <math.h>     // cc -lm
#include <X11/Xlib.h> // cc -l X11
// #include <X11/Xutil.h>
#include <unistd.h>
#include <stdbool.h>

#define BORDER 3
#define WIDTH  700
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
#define e 0.88 // 反発係数

#define t 0.1
#define SLEEP_TIME 8000 // [μs]

#define R_C 80

#define MAX 5
#define WALL_MAX 10

#define red 0xFF0000
#define gray 0xBEBEBE
#define white 0xFFFFFF
#define black 0x000000

typedef struct {
	float x;
	float y;
} Vector;

float length(const Vector v) {
	return sqrtf(powf(v.x, 2) + powf(v.y, 2));
}

float dot(const Vector a, const Vector b) {
	return a.x*b.x + a.y*b.y;
}

typedef struct {
	Vector pos1;
	Vector pos2;
	float a;
	float b;
	float c; // ax  - y + c = 0
	Vector n; // 単位法線ベクトル
	float minX;
	float maxX;
	float minY;
	float maxY;
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

Wall* addWall(Vector pos1, Vector pos2) {

	if (pos1.x > pos2.x) {
		const Vector temp = pos1;
		pos1 = pos2;
		pos2 = temp;
	}

	const float dx = pos2.x - pos1.x;
	const float dy = pos2.y - pos1.y;
	float a, b; // ax - y + c = 0
	if(dx < 0 || 0 < dx) { // dx != 0
		a = dy / dx;// y = ax+c
		b = -1;
	} else { // x = c
		a = 1;
		b = 0;
	}
	const float c = pos1.y - a*pos1.x;
	if(!(dy < 0 || 0 < dy)) { // dy == 0 => y + c = 0
		a = 0;
		b = 1;
	}

	Vector n = (Vector) {dy, -dx};
	const float l = length(n);
	n = (Vector) {n.x/l, n.y/l}; // 点は考慮しない
	const float min_x = pos1.x < pos2.x ? pos1.x : pos2.x;
	const float max_x = pos1.x > pos2.x ? pos1.x : pos2.x;
	const float min_y = pos1.y < pos2.y ? pos1.y : pos2.y;
	const float max_y = pos1.y > pos2.y ? pos1.y : pos2.y;
	const Wall w = {pos1, pos2, a, b, c, n, min_x, max_x, min_y, max_y};
	printf("Wall generated:\n\ta = %f, b = %f, c = %f, n = (%f, %f)\n", a, b, c, n.x, n.y);
	if (++wall_last >= WALL_MAX) wall_last = 0;
	placed_walls[wall_last] = w;
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
	const float dr = ball->r*2; // 直径
	const Vector pos = {ball->center.x-(ball->r), ball->center.y-(ball->r)};
	float deg = (float) ((180*ball->rad) / PI);
	while(deg > 360) deg -= 360;

	XSetForeground(dpy, gc, red);
	XFillArc(dpy, w, gc, (int) pos.x, (int) pos.y, (int) dr, (int) dr, (int) (deg*64.0), 180*64);

	XSetForeground(dpy, gc, gray);

	float round = 180 + deg;
	while(round > 360) round -= 360;
	XFillArc(dpy, w, gc, (int) pos.x, (int) pos.y, (int) dr, (int) dr, (int) (round*64.0), 180*64);

	XSetForeground(dpy, gc, black);

	Vector vpoint[4];
	vpoint[0] = (Vector) {pos.x, pos.y+(ball->r)-(ball->r/15)};
	vpoint[1] = (Vector) {pos.x+ball->r*2, vpoint[0].y};
	vpoint[2] = (Vector) {vpoint[1].x, vpoint[0].y+(ball->r*2/15)};
	vpoint[3] = (Vector) {vpoint[0].x, vpoint[2].y};

	for(int i = 0; i < 4; i++){
		rotate_point(&vpoint[i], ball->center, ball->rad);
	}

	XPoint point[4];
	for(int i = 0; i < 4; i++){
		point[i].x = (short) vpoint[i].x;
		point[i].y = (short) vpoint[i].y;
	}

	XFillPolygon(dpy, w, gc, point, 4, Convex, CoordModeOrigin);

	XSetForeground(dpy, gc, black);// 中心外側円
	XFillArc(dpy, w, gc, (int) (pos.x+(ball->r)-(ball->r/5)), (int) (pos.y+(ball->r)-(ball->r/5)), (int) dr/5, (int) dr/5, 0, 360*64);

	XSetForeground(dpy, gc, white);// 中心内側円
	XFillArc(dpy, w, gc, (int) (pos.x+(ball->r* 5/6)), (int) (pos.y+(ball->r* 5/6)), (int) ((ball->r*2/5)-(ball->r/15)), (int) ((dr/5)-(ball->r/15)), 0, 360*64);
	// XFillArc(dpy, w, gc, (int) (pos.x+(ball->r)-(ball->r/5)+(ball->r/30)), (int) (pos.y+(ball->r)-(ball->r/5)+(ball->r/30)), (int) ((dr/5)-(ball->r/15)), (int) ((dr/5)-(ball->r/15)), 0, 360*64);

	// XSetForeground(dpy, gc, black);// 外接正角形
	// XDrawRectangle(dpy, w, gc, (int) pos.x, (int) pos.y, (int) dr, (int) dr);

	// XDrawLine(dpy, w, gc, pos.x+(ball->r), pos.y, pos.x+(ball->r), pos.y+dr);

	while(ball->rad > 2*PI) ball->rad -= (float) (2*PI);

}

void DrawWall(Display *dpy, const Window w, GC gc, const Wall *wall) {
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

void reflect(const Ball before, Ball *after) {
	for(int i = 0;i < WALL_MAX; ++i) {
		const Wall w = placed_walls[i];
		if(w.pos1.x == -1) continue;
		Vector n = w.n;

		const float dx = w.pos2.x - w.pos1.x;
		const float dy = w.pos2.y - w.pos1.y;

		if(w.b == 0) { // x = c

			// ★ 線分の範囲内かチェック（Y座標）
			if (after->center.y < w.minY - after->r || after->center.y > w.maxY + after->r) {
				continue; // 線分の範囲外なので反射しない
			}

			if (w.pos1.x < before.center.x) {
				n = (Vector) {-n.x, -n.y};
				const Vector p1 = {before.center.x + n.x*before.r, before.center.y};
				const Vector p2 = {after->center.x + n.x*after->r, after->center.y};
				if(p2.x < w.pos1.x && w.pos1.x < p1.x) {// 左に壁
					after->center.x += p1.x - w.pos1.x + 1;
					after->v.x *= (float)-e;
				}
			} else if(w.pos1.x > before.center.x) {
				const Vector p1 = {before.center.x + n.x*before.r, before.center.y};
				const Vector p2 = {after->center.x + n.x*after->r, after->center.y};
				if (p1.x < w.pos1.x && w.pos1.x < p2.x) { // pos1.x = pos2.x
					// 右に壁
					after->center.x -= p2.x - w.pos1.x  -1;
					after->v.x *= (float)-e;
				}
			}

		} else {
			const float y = w.a*before.center.x + w.c;
			if(y > before.center.y) n = (Vector) {-n.x, -n.y};
			/*
			 * 壁の大きさball.r/2の法線ベクトルを円の中心の位置ベクトルに足して、接点の位置ベクトルを出す。
			 */
			const Vector p1 = {before.center.x + n.x*before.r, before.center.y + n.y*before.r};
			const Vector p2 = {after->center.x + n.x*after->r, after->center.y + n.y*after->r};
			// const float l = dx*(p1.y - w.pos1.y) - dy*(p1.x - w.pos1.x);
			const float l = dx*(before.center.y - w.pos1.y) - dy*(before.center.x - w.pos1.x);
			const float m = dx*(p2.y - w.pos1.y) - dy*(p2.x - w.pos1.x);

			// printf("(%f, %f) (%f, %f) (%f, %f)\n", n.x, n.y, dx, dy, l, m);
			if(l*m < 0) { // 貫通した

				const float d = fabsf(w.a*before.center.x + w.b*before.center.y + w.c) / sqrtf(powf(w.a, 2) + powf(w.b, 2));
				const float p_x = before.center.x + n.x*d; // 接点のx座標
				const float p_y = w.a*p_x + w.c; //	接点のy座標

				// 線分の範囲内か

				if (p_x < w.minX - before.r || p_x > w.maxX + before.r ||
				    p_y < w.minY - before.r || p_y > w.maxY + before.r) {
					continue; // 範囲外
				}

				// printf("thru (%f, %f) %f (%f, %f)\n", before.center.x, before.center.y, d, p_x, p_y);

				if(y < before.center.y) { // ボールの上に壁
					// printf("downer (%f, %f) (%f, %f), (%f, %f)\n", n.x, n.y, p1.x, p1.y, p2.x, p2.y);
					if(p1.y > p_y && p_y > p2.y) {
						after->center.y -= (float) (p1.y - p_y);
						after->v.y *= (float)-e;
					}
					if(p1.x < p_x && p_x < p2.x) {
						after->center.x -= (float) (p_x - p2.x);
						after->v.x *= (float)-e;
					}
				} else if (y > before.center.y){ // ボールの下に壁'
					printf("upper (%f, %f) (%f, %f), (%f, %f)\n", p_x, p_y, p1.x, p1.y, p2.x, p2.y);
					if(p1.y < p_y && p_y < p2.y) {
						after->center.y = (float) (after->center.y - (p_y - p1.y));
						// after->v.y *= (float)-e;
					}
					if(p1.x > p_x && p_x > p2.x) {
						after->center.x -= (float) (p2.x - p_x);
						// after->v.x *= (float)-e;
					}

					const float vn = dot(after->v, n);
					after->v.x -= (1 + e) * vn * n.x;
					after->v.y -= (1 + e) * vn * n.y;

						// 摩擦なし → 回転は適当
						// 必要ならここで角速度処理
				}
			}
		}
	}
}

int main(int argc, char **argv) {

	for (int i = 0; i < MAX; i++) {
		placed_balls[i] = (Ball) {
			(Vector) {0,0},
			(Vector) {0,0},
			(Vector) {0,0},
			0, 0,  0, 0
		};
	}

	for (int i = 0; i < WALL_MAX; i++) {
		placed_walls[i] = (Wall) {
			(Vector) {-1, -1},
			(Vector) {-1, -1}
		};
	}


	Display *dpy = XOpenDisplay("");
	const Window root = DefaultRootWindow(dpy);
	const Window w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HEIGHT, BORDER, black, white);
	XMapWindow(dpy, w);
	GC gc = XCreateGC(dpy, w, 0, NULL);

	const Window exit = XCreateSimpleWindow(dpy, w, 10, 10, 50, 30, BORDER, black, gray); // exit window
	XMapSubwindows(dpy, w);

	XSelectInput(dpy, w, PointerMotionMask | ButtonPressMask | ButtonReleaseMask);
	XSelectInput(dpy, exit, ButtonPressMask);

	XEvent event;

	Vector start = {-1, -1};
	Vector mouse = {-1, -1};

	int isWriting = 0;

	addWall((Vector) {0, 0}, (Vector) {WIDTH, 0});			// 上
	addWall((Vector) {0, 0}, (Vector) {0, HEIGHT});			// 左
	addWall((Vector) {WIDTH, 0}, (Vector) {WIDTH, HEIGHT}); // 右
	addWall((Vector) {0, HEIGHT}, (Vector) {WIDTH, HEIGHT});// 下

	// addWall((Vector) {0, HEIGHT/2.0}, (Vector) {WIDTH, HEIGHT/2.0});
	addWall((Vector) {0,  0}, (Vector) {WIDTH, HEIGHT});

	while(1){

		if(XEventsQueued(dpy, QueuedAfterReading)){
			XNextEvent(dpy, &event);
			switch(event.type){
				case MotionNotify:
					if(isWriting == 1){
						mouse.x = (float) event.xmotion.x;
						mouse.y = (float) event.xmotion.y;
						rotate_point(&mouse, start, PI);
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
						placed_balls[last].r_speed = placed_balls[last].v.x / R_C;
						placed_balls[last].a.y = g;
						mouse.x = -1;
					}
				break;
				default:
					continue;
			}
		} else {

			usleep(SLEEP_TIME);
			XClearWindow(dpy, w);

			for(int i = 0; i < MAX; ++i) {

				Ball *ball = placed_balls+i;
				if(ball->r <= 0) continue;
				const Ball before = *ball;

				ball->v.x += ball->a.x * (float)t;
				ball->v.y += ball->a.y * (float)t;
				ball->center.x += ball->v.x * (float)t;
				ball->center.y += ball->v.y * (float)t;
				ball->rad -= ball->r_speed * (float)t;
				while(ball->rad > 2*PI) ball->rad -= (float) (2*PI);
				reflect(before, ball);
				ball->r_speed = ball->v.x / R_C;

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