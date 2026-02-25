// ----- Parameters -----
#define BORDER 3 // ウィンドウの枠の太さ [px]
#define WIDTH  700 // ウィンドウの幅 [px]
#define HEIGHT 700 // ウィンドウの高さ [px]
#define g 9.80665 // 重力加速度 [px/s^2]
#define e 1.1 // 反発係数
#define t 0.1 // 1ループでの経過時間 [s]
#define MASS 5 // ボールの質量
#define SLEEP_TIME 5000 // [μs]
#define R_C 80 // 各速度の割合 (角速度 = v / R_C)
#define R 30 // ボールの半径 [px]
#define BALL_MAX 10 // ボールの最大数
#define WALL_MAX 10 // 壁の最大数
#define MAX_SPEED 300 // 最大速度 [px/s]
// ----------------------

#include <stdio.h>
#include <math.h>     // cc -lm
#include <X11/Xlib.h> // cc -l X11
#include <unistd.h>
#include <stdbool.h>

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

#define red 0xFF0000
#define gray 0xBEBEBE
#define white 0xFFFFFF
#define black 0x000000

typedef struct {
	float x;
	float y;
} Vector;

Vector sub(const Vector a, const Vector b) {
	return (Vector) {a.x-b.x, a.y-b.y};
}

Vector add(const Vector a, const Vector b) {
	return (Vector) {a.x+b.x, a.y+b.y};
}

Vector mul(const Vector a, const float k) {
	return (Vector) {a.x * k, a.y * k};
}

float length(const Vector v) {
	return sqrtf(powf(v.x, 2) + powf(v.y, 2));
}

float dot(const Vector a, const Vector b) {
	return a.x * b.x + a.y * b.y;
}

float cross(const Vector a, const Vector b) {
	return a.x * b.y - a.y * b.x;
}

Vector normalize(const Vector a) {
	const float l = length(a);
	if(l == 0) return (Vector) {0, 0};
	return (Vector){a.x / l, a.y / l};
}

typedef struct {
	Vector pos1;
	Vector pos2;
	Vector d;
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

int ball_last = 0;
Ball placed_balls[BALL_MAX];

int wall_last = 0;
Wall placed_walls[WALL_MAX];

Ball* addBall(const Vector center, const Vector v, const Vector a, const float r_speed, const float r, const float rad, const float mass) {
	const Ball ball = {center, v, a, r_speed, r, rad, mass};
	if(++ball_last >= BALL_MAX) ball_last = 0;
	placed_balls[ball_last] = ball;
	return placed_balls + ball_last;
}

Wall* addWall(Vector pos1, Vector pos2) {
	if(pos1.x > pos2.x) {
		const Vector temp = pos1;
		pos1 = pos2;
		pos2 = temp;
	}
	const Vector d = sub(pos2, pos1);
	const Wall w = {pos1, pos2, d};
	printf("Wall generated:\n\td(%f, %f)\n", d.x, d.y);
	if(++wall_last >= WALL_MAX) wall_last = 0;
	placed_walls[wall_last] = w;
	return placed_walls + wall_last;
}

void rotate_point(Vector* p, const Vector center, const float rad) {
	const float dx = p->x - center.x,
				dy = p->y - center.y;
	p->x = dx * cosf(rad) - dy * -sinf(rad) + center.x;
	p->y = dx * -sinf(rad) + dy * cosf(rad) + center.y;
}

void DrawBall(Display* dpy, const Window w, GC gc, Ball* ball) {
	if(ball->r <= 0) return;
	const float dr = ball->r * 2; // 直径
	const Vector pos = {ball->center.x - (ball->r), ball->center.y - (ball->r)};
	float deg = (float)((180 * ball->rad) / PI);
	while(deg > 360) deg -= 360; while(deg < 0) deg += 360;

	XSetForeground(dpy, gc, red);
	XFillArc(dpy, w, gc, (int)pos.x, (int)pos.y, (int)dr, (int)dr, (int)(deg * 64.0), 180 * 64);

	XSetForeground(dpy, gc, gray);

	float round = 180 + deg;
	while(round > 360) round -= 360; while(round < 0) round += 360;
	XFillArc(dpy, w, gc, (int)pos.x, (int)pos.y, (int)dr, (int)dr, (int)(round * 64.0), 180 * 64);

	XSetForeground(dpy, gc, black);

	Vector vpoint[4];
	vpoint[0] = (Vector){pos.x, pos.y + (ball->r) - (ball->r / 15)};
	vpoint[1] = (Vector){pos.x + dr, vpoint[0].y};
	vpoint[2] = (Vector){vpoint[1].x, vpoint[0].y + (dr / 15)};
	vpoint[3] = (Vector){vpoint[0].x, vpoint[2].y};

	for(int i = 0; i < 4; i++) {
		rotate_point(&vpoint[i], ball->center, ball->rad);
	}

	XPoint point[4];
	for(int i = 0; i < 4; i++) {
		point[i].x = (short)vpoint[i].x;
		point[i].y = (short)vpoint[i].y;
	}

	XFillPolygon(dpy, w, gc, point, 4, Convex, CoordModeOrigin);

	XSetForeground(dpy, gc, black); // 中心外側円
	XFillArc(dpy, w, gc, (int)(pos.x + (ball->r) - (ball->r / 5)), (int)(pos.y + (ball->r) - (ball->r / 5)),
	         (int)dr / 5, (int)dr / 5, 0, 360 * 64);

	XSetForeground(dpy, gc, white); // 中心内側円
	XFillArc(dpy, w, gc, (int)(pos.x + (ball->r * 5 / 6)), (int)(pos.y + (ball->r * 5 / 6)),
	         (int)((dr / 5) - (ball->r / 15)), (int)((dr / 5) - (ball->r / 15)), 0, 360 * 64);
	// XFillArc(dpy, w, gc, (int) (pos.x+(ball->r)-(ball->r/5)+(ball->r/30)), (int) (pos.y+(ball->r)-(ball->r/5)+(ball->r/30)), (int) ((dr/5)-(ball->r/15)), (int) ((dr/5)-(ball->r/15)), 0, 360*64);

	// XSetForeground(dpy, gc, black); // 外接正角形
	// XDrawRectangle(dpy, w, gc, (int)pos.x, (int)pos.y, (int)dr, (int)dr);

	while(ball->rad > 2 * PI) ball->rad -= (float)(2 * PI); while(ball->rad < 0) ball->rad += (float)(2 * PI);
}

void DrawWall(Display* dpy, const Window w, GC gc, const Wall* wall) {
	if(wall->pos1.x == -1) return;
	XSetForeground(dpy, gc, black);
	XDrawLine(dpy, w, gc, (int)wall->pos1.x, (int)wall->pos1.y, (int)wall->pos2.x, (int)wall->pos2.y);
}

void DrawWalls(Display* dpy, const Window w, GC gc) {
	for(int i = 0; i < WALL_MAX; ++i)
		DrawWall(dpy, w, gc, placed_walls + i);
}

void DrawBalls(Display* dpy, const Window w, GC gc) {
	for(int i = 0; i < BALL_MAX; ++i)
		DrawBall(dpy, w, gc, placed_balls + i);
}

Vector closest(const Vector A, const Vector B, const Vector P) {
	const Vector AB = sub(B, A);
	const Vector AP = sub(P, A);
	const Vector BP = sub(P, B);
	Vector c;
	if(dot(AB, AP) < 0) {
		c = A;
	} else if(dot(BP, mul(AB, -1)) < 0) {
		c = B;
	} else {
		const float ab_ = length(AB);
		const float norm = dot(AP, AB) / ab_;
		c =  add(A, mul(mul(AB, norm), 1 / ab_));
	}
	return c;
}

void reflect(const Ball before, Ball* after) {
	for(int i = 0; i < WALL_MAX; ++i) {
		const Wall w = placed_walls[i];
		if(w.pos1.x == -1) continue; // 設置されていない壁オブジェクト

		const Vector p1 = closest(w.pos1, w.pos2, before.center);
		const Vector p2 = closest(w.pos1, w.pos2, after->center);
		const Vector d_after = sub(after->center, p2);
		const Vector n = normalize(d_after);
		const float dist_after = length(d_after);
		if(dist_after > after->r) {
			const float l = cross(w.d, sub(before.center, w.pos1)),
						m = cross(w.d, sub(after->center, w.pos1));
			if(l * m >= 0) continue;
		}
		if(length(n) == 0) {
			// 中心がちょうど最近点にある等で法線が作れない場合、壁の法線っぽいものを作る
			// const Vector AB = sub(w.pos2, w.pos1);
			// n = normalize((Vector){-AB.y, AB.x});
			// if(n.x == 0.0f && n.y == 0.0f) continue;
			printf("called\n");
			continue;
		}

		const float error = after->r - dist_after;
		// if(error > 0) {
			after->center = add(after->center, mul(n, error));
		// }

		// 反射: 壁へ向かっているときだけ
		const float vn = dot(after->v, n);
		if(vn < 0) {
			after->v.x -= (float)(1.0f + e) * vn * n.x;
			after->v.y -= (float)(1.0f + e) * vn * n.y;
		}
	}
}

int main(int argc, char** argv) {
	for(int i = 0; i < BALL_MAX; i++) {
		placed_balls[i] = (Ball){
			(Vector){-1, -1},
			(Vector){-1, 1},
			(Vector){-1, -1},
			-1, -1, -1, -1
		};
	} // init

	for(int i = 0; i < WALL_MAX; i++) {
		placed_walls[i] = (Wall){
			(Vector){-1, -1},
			(Vector){-1, -1}
		};
	} // init

	Display* dpy = XOpenDisplay("");
	const Window root = DefaultRootWindow(dpy),
	             w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HEIGHT, BORDER, black, white);
	XMapWindow(dpy, w);
	GC gc = XCreateGC(dpy, w, 0, NULL);

	const Window exit = XCreateSimpleWindow(dpy, w, 10, 10, 50, 30, BORDER, black, gray); // exit window
	XMapSubwindows(dpy, w);

	XSelectInput(dpy, w, PointerMotionMask | ButtonPressMask | ButtonReleaseMask);
	XSelectInput(dpy, exit, ButtonPressMask);

	XEvent event;

	Vector start = {-1, -1},
	       mouse = {-1, -1};

	bool isWritingArrow = false,
	     isWritingWall = false;

	addWall((Vector){0, 0}, (Vector){WIDTH, 0}); // 上
	// addWall((Vector){0, 0}, (Vector){0, HEIGHT}); // 左
	// addWall((Vector){WIDTH, 0}, (Vector){WIDTH, HEIGHT}); // 右
	addWall((Vector){0, HEIGHT}, (Vector){WIDTH, HEIGHT}); // 下

	// addWall((Vector) {0, HEIGHT/2.0}, (Vector) {WIDTH, HEIGHT/2.0});
	// addWall((Vector) {0,  0}, (Vector) {WIDTH, HEIGHT});

	while(1) {
		if(XEventsQueued(dpy, QueuedAfterReading)) {
			XNextEvent(dpy, &event);
			switch(event.type) {
				case MotionNotify:
					if(isWritingArrow) {
						mouse.x = (float)event.xmotion.x;
						mouse.y = (float)event.xmotion.y;
						rotate_point(&mouse, start, PI);
					}
					if(isWritingWall) {
						mouse.x = (float)event.xmotion.x;
						mouse.y = (float)event.xmotion.y;
					}
					break;
				case ButtonPress:
					if(event.xany.window == exit) return 0;
					if(event.xany.window == w) {
						switch(event.xbutton.button) {
							case MOUSE_LEFT:
								if(isWritingWall) continue;
								start.x = (float)event.xbutton.x;
								start.y = (float)event.xbutton.y;
								mouse.x = start.x;
								mouse.y = start.y;
								addBall(start, (Vector){0, 0}, (Vector){0, 0}, 0, R, 0, MASS);
								isWritingArrow = true;
								break;
							case MOUSE_RIGHT:
								if(isWritingArrow) continue;
								start.x = (float) event.xbutton.x;
								start.y = (float) event.xbutton.y;
								isWritingWall = true;
								break;
							default:
								continue;
						}
					}
					break;
				case ButtonRelease:
					if(event.xany.window == w) {
						switch(event.xbutton.button) {
							case MOUSE_LEFT:
								if(isWritingArrow) {
									isWritingArrow = false;
									Vector end = {(float)event.xbutton.x, (float)event.xbutton.y};
									rotate_point(&end, start, PI);
									placed_balls[ball_last].v.x = (end.x - start.x) / placed_balls[ball_last].mass;
									placed_balls[ball_last].v.y = (end.y - start.y) / placed_balls[ball_last].mass;
									placed_balls[ball_last].r_speed = placed_balls[ball_last].v.x / R_C;
									placed_balls[ball_last].a.y = g;
									mouse.x = -1;
								}
								break;
							case MOUSE_RIGHT:
								if(isWritingWall) {
									isWritingWall = false;
									addWall(start, (Vector){(float) event.xbutton.x, (float) event.xbutton.y});
								}
								break;
							case MOUSE_MIDDLE:
								for(int i = 0; i < BALL_MAX; i++) {
									placed_balls[i] = (Ball){
										(Vector){-1, -1},
										(Vector){-1, 1},
										(Vector){-1, -1},
										-1, -1, -1, -1
									};
								} // init
								for(int i = 0; i < WALL_MAX; i++) {
									placed_walls[i] = (Wall){
										(Vector){-1, -1},
										(Vector){-1, -1}
									};
								} // init
								break;
							default:
								continue;
						}
					}
					break;
				default:
					continue;
			}
		} else {
			usleep(SLEEP_TIME);
			XClearWindow(dpy, w);

			for(int i = 0; i < BALL_MAX; ++i) {
				Ball* ball = placed_balls + i;
				if(ball->r <= 0) continue;
				const Ball before = *ball;

				ball->v.x += ball->a.x * (float)t;
				ball->v.y += ball->a.y * (float)t;
				ball->center.x += ball->v.x * (float)t;
				ball->center.y += ball->v.y * (float)t;
				ball->rad -= ball->r_speed * (float)t;
				while(ball->rad > 2 * PI) ball->rad -= (float)(2 * PI);
				while(ball->rad < 0) ball->rad += (float)(2 * PI);
				reflect(before, ball);
				if(isfinite(ball->center.x) == false || isfinite(ball->center.y) == false) {
					placed_balls[i] = (Ball){
						// ボールを削除
						(Vector){-1, -1},
						(Vector){-1, 1},
						(Vector){-1, -1},
						-1, -1, -1, -1
					};
					continue;
				}

				if(ball->center.x < 0 || ball->center.y < 0) {
					placed_balls[i] = (Ball){
						(Vector){-1, -1},
						(Vector){-1, 1},
						(Vector){-1, -1},
						-1, -1, -1, -1
					}; // ボールを削除
					continue;
				}

				if(ball->v.x > MAX_SPEED) {
					ball->v.x = MAX_SPEED;
				} else if(ball->v.y > MAX_SPEED) {
					ball->v.y = MAX_SPEED;
				}

				if(ball->v.x < -MAX_SPEED) {
					ball->v.x = -MAX_SPEED;
				} else if(ball->v.y < -MAX_SPEED) {
					ball->v.y = -MAX_SPEED;
				}

				ball->r_speed = ball->v.x / R_C;

			}

			DrawBalls(dpy, w, gc);
			DrawWalls(dpy, w, gc);

			if((isWritingArrow || isWritingWall) && mouse.x != -1) {
				XSetForeground(dpy, gc, black);
				XDrawLine(dpy, w, gc, (int)start.x, (int)start.y, (int)mouse.x, (int)mouse.y);
			}

			XFlush(dpy);
		}
	}
}