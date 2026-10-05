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
