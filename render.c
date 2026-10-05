#include <math.h>
#include <stdlib.h>
#include "texture.h"
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
		drawpixel(current_pixel, 0xff);

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

void fill_uvs(Vertex_t *vertexes, int vertex_count, UV_t *uvs) {
	for (int i = 0; i < vertex_count; i++) {
		vertexes[i].uv = uvs[i];
	}
	return;
}

void render_wireframe(Vertex_t *vertexes, int count, drawpixel_t drawpixel) {
	for (int i = 0; i < count; i = i + 3) {
		drawline(vertexes[0 + i].pos, vertexes[1 + i].pos, drawpixel);
		drawline(vertexes[1 + i].pos, vertexes[2 + i].pos, drawpixel);
		drawline(vertexes[2 + i].pos, vertexes[0 + i].pos, drawpixel);
	}
	return;
}

UV_t interpolate_uv(Vertex_t v1, Vertex_t v2, double y) {
	if (v2.pos.y == v1.pos.y) return v1.uv;
	double t = (y - v1.pos.y) / (v2.pos.y - v1.pos.y);
	return (UV_t){
		v1.uv.u + (v2.uv.u - v1.uv.u) * t,
		v1.uv.v + (v2.uv.v - v1.uv.v) * t
	};
}

void scanline_raster(Vertex_t p1, Vertex_t p2, Vertex_t p3,
		subfunc_t subfunc, drawpixel_t drawpixel) {
	Vertex_t tmp;
	if (p1.pos.y > p2.pos.y) { tmp = p1; p1 = p2; p2 = tmp; }
	if (p1.pos.y > p3.pos.y) { tmp = p1; p1 = p3; p3 = tmp; }
	if (p2.pos.y > p3.pos.y) { tmp = p2; p2 = p3; p3 = tmp; }

	int y1 = (int)round(p1.pos.y);
	int y3 = (int)round(p3.pos.y);

	for (int y = y1; y < y3; y++) {
		Vertex_t side_v = (y < round(p2.pos.y)) ? p2 : p3;
		Vertex_t start_v = (y < round(p2.pos.y)) ? p1 : p2;

		double t_short = (start_v.pos.y == side_v.pos.y) ? 0 : (y - start_v.pos.y) / (side_v.pos.y - start_v.pos.y);
		double x_short = start_v.pos.x + (side_v.pos.x - start_v.pos.x) * t_short;
		UV_t uv_short = interpolate_uv(start_v, side_v, y);

		double t_long = (y - p1.pos.y) / (p3.pos.y - p1.pos.y);
		double x_long = p1.pos.x + (p3.pos.x - p1.pos.x) * t_long;
		UV_t uv_long = interpolate_uv(p1, p3, y);

		double x_left = x_short;  UV_t uv_left = uv_short;
		double x_right = x_long; UV_t uv_right = uv_long;

		if (x_left > x_right) {
			double tx = x_left; x_left = x_right; x_right = tx;
			UV_t tuv = uv_left; uv_left = uv_right; uv_right = tuv;
		}

		int start_x = (int)round(x_left);
		int end_x = (int)round(x_right);

		for (int x = start_x; x < end_x; x++) {
			double factor = (start_x == end_x) ? 0 : (double)(x - start_x) / (end_x - start_x);

			UV_t pixel_uv = {
				uv_left.u + (uv_right.u - uv_left.u) * factor,
				uv_left.v + (uv_right.v - uv_left.v) * factor
			};

			subfunc((Vector2_t){x, y}, pixel_uv, drawpixel);
		}
	}
}


void render_solid(Vertex_t *vertexes, int count, drawpixel_t drawpixel) {
	void cb_filler(Vector2_t pixel, UV_t uv, drawpixel_t drawpixel) {
		drawpixel(pixel, 0xff);
		return;
	}

	for (int i = 0; i < count; i = i + 3) {
		scanline_raster(vertexes[0 + i], vertexes[1 + i], vertexes[2 + i],
				cb_filler, drawpixel);
	}

}

void render_solid_texture(Vertex_t *vertexes, int count, drawpixel_t drawpixel) {
	void cb_filler(Vector2_t pixel, UV_t uv, drawpixel_t drawpixel) {
		int tx = (int)(uv.u * 63);
		int ty = (int)(uv.v * 63);
		unsigned char color = texture[ty * 64 + tx];
		drawpixel(pixel, color);
		return;
	}

	for (int i = 0; i < count; i = i + 3) {
		scanline_raster(vertexes[0 + i], vertexes[1 + i], vertexes[2 + i],
				cb_filler, drawpixel);
	}

}
