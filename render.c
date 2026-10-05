#include <math.h>
#include <stdlib.h>
#include "custom_math.h"
#include "render.h"

void drawline(Vector2_t a, Vector2_t b, drawpixel_t drawpixel) {
	int x1 = (int)a.x;
	int y1 = (int)a.y;
	int x2 = (int)b.x;
	int y2 = (int)b.y;
	int dx = abs(x2 - x1);
	int dy = -abs(y2 - y1);
	int sx = x1 < x2 ? 1 : -1;
	int sy = y1 < y2 ? 1 : -1;
	int err = dx + dy;

	while (1) {
		Vector2_t current_pixel = { (double)x1, (double)y1 };
		drawpixel(current_pixel);

		if (x1 == x2 && y1 == y2)
			break;
		int e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x1 += sx;
		}

		if (e2 <= dx) {
			err += dx;
			y1 += sy;
		}
	}
}

void render_wireframe(Vector2_t *vertexes, int count, drawpixel_t drawpixel) {
	for (int i = 0; i < count; i = i + 3) {
		drawline(vertexes[0 + i], vertexes[1 + i], drawpixel);
		drawline(vertexes[1 + i], vertexes[2 + i], drawpixel);
		drawline(vertexes[2 + i], vertexes[0 + i], drawpixel);
	}
	return;
}

void scanline_raster(Vector2_t p1, Vector2_t p2, Vector2_t p3,
					 rastersubfunc_t subfunc, drawpixel_t drawpixel) {
	Vector2_t tmp;
	if (p1.y > p2.y) { tmp = p1; p1 = p2; p2 = tmp; }
	if (p1.y > p3.y) { tmp = p1; p1 = p3; p3 = tmp; }
	if (p2.y > p3.y) { tmp = p2; p2 = p3; p3 = tmp; }

	int y1 = (int)round(p1.y);
	int y2 = (int)round(p2.y);
	int y3 = (int)round(p3.y);

	if (y1 == y3) return;

	// 2. Отрисовка верхней половины треугольника (от p1.y до p2.y)
	if (y1 != y2) {
		double dx_left = (p2.x - p1.x) / (p2.y - p1.y);
		double dx_right = (p3.x - p1.x) / (p3.y - p1.y);

		for (int y = y1; y < y2; y++) {
			double cur_x_left = p1.x + dx_left * (y - y1);
			double cur_x_right = p1.x + dx_right * (y - y1);

			int start_x = (int)round(cur_x_left);
			int end_x = (int)round(cur_x_right);
			if (start_x > end_x) { int t = start_x; start_x = end_x; end_x = t; }

			for (int x = start_x; x < end_x; x++) {
				subfunc((Vector2_t){x, y}, drawpixel);
			}
		}
	}

	if (y2 != y3) {
		double dx_left = (p3.x - p2.x) / (p3.y - p2.y);
		double dx_right = (p3.x - p1.x) / (p3.y - p1.y);

		for (int y = y2; y < y3; y++) {
			double cur_x_left = p2.x + dx_left * (y - y2);
			double cur_x_right = p1.x + dx_right * (y - y1);
			int start_x = (int)round(cur_x_left);
			int end_x = (int)round(cur_x_right);

			if (start_x > end_x) { int t = start_x; start_x = end_x; end_x = t; }
			for (int x = start_x; x < end_x; x++) {
				subfunc((Vector2_t){x, y}, drawpixel);
			}
		}
	}
}

void render_solid(Vector2_t *vertexes, int count, drawpixel_t drawpixel) {
	void cb_filler(Vector2_t pixel, drawpixel_t drawpixel) {
		drawpixel(pixel);
		return;
	}

	for (int i = 0; i < count; i = i + 3) {
		scanline_raster(vertexes[0 + i], vertexes[1 + i], vertexes[2 + i],
				cb_filler, drawpixel);
	}

}
