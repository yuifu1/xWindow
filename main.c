// ----- Parameters -----
#define BORDER 3 // ウィンドウの枠の太さ [px]
#define WIDTH  700 // ウィンドウの幅 [px]
#define HEIGHT 700 // ウィンドウの高さ [px]
#define g 9.80665 // 重力加速度 [px/s^2]
#define e 0.88 // 反発係数
#define t 0.15 // 1ループでの経過時間 [s]
#define SLEEP_TIME 8000 // [μs]
#define R_C 80 // 各速度の割合 (角速度 = v / R_C)
#define R 30 // ボールの半径 [px]
#define BALL_MAX 10 // ボールの最大数
#define WALL_MAX 10 // 壁の最大数
#define MAX_SPEED 1000 // 最大速度 [px/s]
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

float length(const Vector v) {
	return sqrtf(powf(v.x, 2) + powf(v.y, 2));
}

float dot(const Vector a, const Vector b) {
	return a.x * b.x + a.y * b.y;
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

	const float dx = pos2.x - pos1.x,
				dy = pos2.y - pos1.y;
	float a, b; // ax - y + c = 0
	if(dx < 0 || 0 < dx) {
		// dx != 0
		a = dy / dx; // y = ax+c
		b = -1;
	} else {
		// x = c
		a = 1;
		b = 0;
	}
	const float c = pos1.y - a * pos1.x;
	if(!(dy < 0 || 0 < dy)) {
		// dy == 0 => y + c = 0
		a = 0;
		b = 1;
	}

	Vector n = (Vector) {dy, -dx};
	const float l = length(n);
	n = (Vector){n.x / l, n.y / l}; // 点は考慮しない
	const float min_x = pos1.x < pos2.x ? pos1.x : pos2.x,
				max_x = pos1.x > pos2.x ? pos1.x : pos2.x,
				min_y = pos1.y < pos2.y ? pos1.y : pos2.y,
				max_y = pos1.y > pos2.y ? pos1.y : pos2.y;
	const Wall w = {pos1, pos2, a, b, c, n, min_x, max_x, min_y, max_y};
	printf("Wall generated:\n\ta = %f, b = %f, c = %f, n = (%f, %f)\n", a, b, c, n.x, n.y);
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

void reflect(const Ball before, Ball* after) {
	for(int i = 0; i < WALL_MAX; ++i) {
		const Wall w = placed_walls[i];
		if(w.pos1.x == -1) continue; // 設置されていない壁オブジェクト

		if(w.b == 0) {
			// x = cの例外処理
			// TODO: 速度早すぎるとはみ出る
			// 線分の範囲内か
			if(after->center.y < w.minY - after->r || after->center.y > w.maxY + after->r)
				continue;

			if(w.pos1.x < before.center.x) {
				// | <- o
				const Vector n = {-w.n.x, -w.n.y};
				const Vector p1 = {before.center.x + n.x * before.r, before.center.y},
				             p2 = {after->center.x + n.x * after->r, after->center.y};
				if(p2.x <= w.pos1.x && w.pos1.x < p1.x) {
					after->center.x = w.pos1.x + after->r;
					after->v.x *= (float)-e;
				}
			} else if(w.pos1.x > before.center.x) {
				// o -> |
				const Vector p1 = {before.center.x + w.n.x * before.r, before.center.y},
				             p2 = {after->center.x + w.n.x * after->r, after->center.y};
				if(p1.x <= w.pos1.x && w.pos1.x < p2.x) {
					// pos1.x = pos2.x
					after->center.x = w.pos1.x - after->r;
					after->v.x *= (float)-e;
				}
			}
		} else {
			const float y = w.a * before.center.x + w.c;
			const Vector n = (y > before.center.y) ? (Vector){-w.n.x, -w.n.y} : w.n; // 法線ベクトルの向きの設定
			/*
			 * 大きさball.rの法線ベクトルを円の中心の位置ベクトルに足して、最近点の位置ベクトルを出す。
			 */
			const Vector p1 = {before.center.x + n.x * before.r, before.center.y + n.y * before.r},
			             p2 = {after->center.x + n.x * after->r, after->center.y + n.y * after->r};
			const float dx = w.pos2.x - w.pos1.x,
			            dy = w.pos2.y - w.pos1.y;
			const float l = dx * (before.center.y - w.pos1.y) - dy * (before.center.x - w.pos1.x),
			            m = dx * (p2.y - w.pos1.y) - dy * (p2.x - w.pos1.x);

			if(l * m < 0) {
				// 貫通したか

				const float d = fabsf(w.a * before.center.x + w.b * before.center.y + w.c) / sqrtf(
					powf(w.a, 2) + powf(w.b, 2));
				const float p_x = before.center.x + n.x * d, // 接点のx座標
							p_y = w.a * p_x + w.c;			 // 接点のy座標

				// 線分の範囲内か

				if(p_x < w.minX - before.r || p_x > w.maxX + before.r ||
					p_y < w.minY - before.r || p_y > w.maxY + before.r) {
					continue; // 範囲外
				}

				// printf("thru %f (%f, %f) %f (%f, %f)\n", n.y, before.center.x, before.center.y, d, p_x, p_y);

				if(y < before.center.y) { // ボールの上に壁
					// printf("downer (%f, %f) (%f, %f), (%f, %f)\n", n.x, n.y, p1.x, p1.y, p2.x, p2.y);
					if(p1.y > p_y && p_y > p2.y) {
						after->center.y = p_y + after->r * (-n.y);
						after->v.y *= (float)-e;
					}
					if((p1.x < p_x && p_x < p2.x) || (p1.x > p_x && p_x > p2.x)) {
						after->center.x = p_x + after->r * (-n.x);
						after->v.x *= (float)-e;
					}
				} else if(y > before.center.y) { // ボールの下に壁
					// printf("upper (%f, %f) (%f, %f), (%f, %f)\n", p_x, p_y, p1.x, p1.y, p2.x, p2.y);
					if(before.center.y < p_y && p_y < p2.y) {
						after->center.y = p_y + after->r * (-n.y);
					}
					if((p1.x < p_x && p_x < p2.x) || (p1.x > p_x && p_x > p2.x)) {
						after->center.x = p_x + after->r * (-n.x);
					}

					const float vn = dot(after->v, n); // ??
					after->v.x -= (float)(1 + e) * vn * n.x;
					after->v.y -= (float)(1 + e) * vn * n.y;

				}
			}
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
	addWall((Vector){0, 0}, (Vector){0, HEIGHT}); // 左
	addWall((Vector){WIDTH, 0}, (Vector){WIDTH, HEIGHT}); // 右
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
								const float mass = 5;
								addBall(start, (Vector){0, 0}, (Vector){0, 0}, 0, R, 0, mass);
								isWritingArrow = true;
								break;
							case MOUSE_RIGHT:
								if(isWritingArrow) continue;
								start.x = (float)event.xbutton.x;
								start.y = (float)event.xbutton.y;
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
									addWall(start, (Vector){(float)event.xbutton.x, (float)event.xbutton.y});
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