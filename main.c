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

#define MAX 5
#define WALL_MAX 10

unsigned long white, black, red, gray;

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
	float l;
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
	const Wall wall = {pos1, pos2, a, b, c, n, l};
	printf("a = %f, b = %f, c = %f, n = (%f, %f)\n", wall.a, wall.b, wall.c, wall.n.x, wall.n.y);
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
	vpoint[0] = (Vector){pos.x, pos.y+(ball->r)-(ball->r/15)};
	vpoint[1] = (Vector){pos.x+ball->r*2, vpoint[0].y};
	vpoint[2] = (Vector){vpoint[1].x, vpoint[0].y+(ball->r*2/15)};
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

/**
 * ボールを壁で反射させる関数
 * @param ball 反射させるボールオブジェクトへのポインタ(速度が更新される)
 * @param wall_n 壁の法線ベクトル(正規化済み)
 * @param coef 反発係数(0.0〜1.0)
 */
void reflect_ball(Ball* ball, const Vector wall_n, const float coef) {
	// 法線方向と接線方向に速度を分解
	const float v_dot_n = dot(ball->v, wall_n);
	
	// 法線方向の成分
	const Vector v_normal = {
		v_dot_n * wall_n.x,
		v_dot_n * wall_n.y
	};
	
	// 接線方向の成分
	const Vector v_tangent = {
		ball->v.x - v_normal.x,
		ball->v.y - v_normal.y
	};
	
	// 反射後の速度ベクトル: 法線方向は反発係数を適用し反転、接線方向はそのまま
	ball->v.x = v_tangent.x - coef * v_normal.x;
	ball->v.y = v_tangent.y - coef * v_normal.y;
}

void reflect(const Ball before, Ball *after) {
	for(int i = 0; i < WALL_MAX; ++i) {
		const Wall w = placed_walls[i];
		if(w.pos1.x == -1) continue;
		
		// 壁との衝突判定
		const float dx = w.pos2.x - w.pos1.x;
		const float dy = w.pos2.y - w.pos1.y;
		
		// 法線ベクトルの方向を決定
		Vector n = w.n;
		if(w.b != 0) { // x = c ではない通常の壁
			const float y = w.a * before.center.x + w.c;
			if(y > before.center.y) n = (Vector) {-n.x, -n.y};
		}
		
		// ボールの表面の点を計算
		const Vector p1 = {before.center.x + n.x * after->r, before.center.y + n.y * after->r};
		const Vector p2 = {after->center.x + n.x * after->r, after->center.y + n.y * after->r};
		
		// 壁を貫通したかチェック (外積を使用)
		const float l = dx * (p1.y - w.pos1.y) - dy * (p1.x - w.pos1.x);
		const float m = dx * (p2.y - w.pos1.y) - dy * (p2.x - w.pos1.x);
		
		if(l * m < 0) { // 貫通した
			// 壁との最短距離を計算
			const float d = fabsf(w.a * after->center.x - w.b * after->center.y + w.c) / sqrtf(w.a * w.a + w.b * w.b);
			
			// ボールが壁に食い込んでいる場合、位置を補正
			if(d < after->r) {
				const float penetration = after->r - d;
				after->center.x += w.n.x * penetration * (l > 0 ? 1 : -1);
				after->center.y += w.n.y * penetration * (l > 0 ? 1 : -1);
			}
			
			// 反射処理を実行
			reflect_ball(after, n, e);
		}
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
	// addWall((Vector) {0, 0}, (Vector) {0, HEIGHT});			// 左
	// addWall((Vector) {WIDTH, 0}, (Vector) {WIDTH, HEIGHT}); // 右
	addWall((Vector) {0, HEIGHT}, (Vector) {WIDTH, HEIGHT});// 下

	addWall((Vector) {0, HEIGHT/2.0}, (Vector) {WIDTH, HEIGHT/2.0});
	// addWall((Vector) {0,  0}, (Vector) {WIDTH, HEIGHT});

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
						// TODO: 最大の矢印を実装
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

				ball->v.x += ball->a.x * (float)t;
				ball->v.y += ball->a.y * (float)t;
				ball->center.x += ball->v.x * (float)t;
				ball->center.y += ball->v.y * (float)t;
				ball->rad -= ball->r_speed * (float)t;
				while(ball->rad > 2*PI) ball->rad -= (float) (2*PI);
				reflect(before, ball);
				ball->r_speed = ball->v.x / 80;

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