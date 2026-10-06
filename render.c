#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include "texture.h"
#include "custom_math.h"
#include "render.h"
#define PERSP
#define TEX_SIZE 64

static float g_zbuf[H * W];

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

void render_wireframe(Vertex_t *vertexes, int count, drawpixel_t drawpixel) {
	for (int i = 0; i < count; i = i + 3) {
		drawline(vertexes[0 + i].pos, vertexes[1 + i].pos, drawpixel);
		drawline(vertexes[1 + i].pos, vertexes[2 + i].pos, drawpixel);
		drawline(vertexes[2 + i].pos, vertexes[0 + i].pos, drawpixel);
	}
	return;
}


#ifdef PERSP
  #define Z_CLEAR   0.0f
  #define Z_PASS(n, old) ((n) > (old))
#else
  #define Z_CLEAR   FLT_MAX
  #define Z_PASS(n, old) ((n) < (old))
#endif

void clear_zbuf(void)
{
    for (int i = 0; i < H * W; i++) g_zbuf[i] = Z_CLEAR;
}

typedef struct { double a0, gx, gy; } Plane_t;

static inline Plane_t make_plane(double a1, double a2, double a3,
                                 double dx2, double dy2,
                                 double dx3, double dy3, double inv_area)
{
    double da2 = a2 - a1, da3 = a3 - a1;
    Plane_t p = { a1,
                  (da2 * dy3 - da3 * dy2) * inv_area,
                  (dx2 * da3 - dx3 * da2) * inv_area };
    return p;
}

void scanline_raster(Vertex_t p1, Vertex_t p2, Vertex_t p3,
                     subfunc_t subfunc, drawpixel_t drawpixel)
{
    Vertex_t a = p1, b = p2, c = p3, t;
    if (a.pos.y > b.pos.y) { t = a; a = b; b = t; }
    if (b.pos.y > c.pos.y) { t = b; b = c; c = t; }
    if (a.pos.y > b.pos.y) { t = a; a = b; b = t; }

    if (c.pos.y <= a.pos.y) return;

    double ax = a.pos.x, ay = a.pos.y;
    double dx2 = b.pos.x - ax, dy2 = b.pos.y - ay;
    double dx3 = c.pos.x - ax, dy3 = c.pos.y - ay;
    double area = dx2 * dy3 - dx3 * dy2;
    if (area == 0.0) return;
    double inv_area = 1.0 / area;

#ifdef PERSP
    double za = 1.0 / a.z, zb = 1.0 / b.z, zc = 1.0 / c.z;
    double ua = a.uv.u * za, ub = b.uv.u * zb, uc = c.uv.u * zc;
    double va = a.uv.v * za, vb = b.uv.v * zb, vc = c.uv.v * zc;
#else
    double za = a.z,    zb = b.z,    zc = c.z;
    double ua = a.uv.u, ub = b.uv.u, uc = c.uv.u;
    double va = a.uv.v, vb = b.uv.v, vc = c.uv.v;
#endif
    Plane_t pz = make_plane(za, zb, zc, dx2, dy2, dx3, dy3, inv_area);
    Plane_t pu = make_plane(ua, ub, uc, dx2, dy2, dx3, dy3, inv_area);
    Plane_t pv = make_plane(va, vb, vc, dx2, dy2, dx3, dy3, inv_area);

    double s_ac = dx3 / dy3;
    double s_ab = (dy2 > 0.0) ? dx2 / dy2 : 0.0;
    double s_bc = (c.pos.y > b.pos.y)
                ? (c.pos.x - b.pos.x) / (c.pos.y - b.pos.y) : 0.0;

    int y_start = (int)ceil(ay - 0.5);
    int y_end   = (int)ceil(c.pos.y - 0.5) - 1;
    if (y_start < 0)     y_start = 0;
    if (y_end   > H - 1) y_end   = H - 1;

    for (int y = y_start; y <= y_end; y++) {
        double yc = y + 0.5;

        double x_long  = ax + (yc - ay) * s_ac;
        double x_short = (yc < b.pos.y) ? ax + (yc - ay) * s_ab
                                        : b.pos.x + (yc - b.pos.y) * s_bc;
        double xl = x_long < x_short ? x_long  : x_short;
        double xr = x_long < x_short ? x_short : x_long;

        int xs = (int)ceil(xl - 0.5);
        int xe = (int)ceil(xr - 0.5) - 1;
        if (xs < 0)     xs = 0;
        if (xe > W - 1) xe = W - 1;
        if (xs > xe) continue;

        double dx = (xs + 0.5) - ax, dy = yc - ay;
        double z = pz.a0 + pz.gx * dx + pz.gy * dy;
        double u = pu.a0 + pu.gx * dx + pu.gy * dy;
        double v = pv.a0 + pv.gx * dx + pv.gy * dy;

        float *zrow = &g_zbuf[y * W];

        for (int x = xs; x <= xe; x++) {
            if (Z_PASS((float)z, zrow[x])) {
                zrow[x] = (float)z;
#ifdef PERSP
                double w = 1.0 / z;
                UV_t uv = { u * w, v * w };
                subfunc((Vector2_t){ x, y }, uv, w, drawpixel);
#else
                UV_t uv = { u, v };
                subfunc((Vector2_t){ x, y }, uv, z, drawpixel);
#endif
            }
            z += pz.gx; u += pu.gx; v += pv.gx;
        }
    }
}


void render_solid(Vertex_t *vertexes, int count, drawpixel_t drawpixel) {
	void cb_filler(Vector2_t pixel, UV_t uv, double z, drawpixel_t drawpixel) {
		drawpixel(pixel, 0xff);
		return;
	}

	for (int i = 0; i < count; i = i + 3) {
		scanline_raster(vertexes[0 + i], vertexes[1 + i], vertexes[2 + i],
				cb_filler, drawpixel);
	}

}

static inline int tex_coord(double t)
{
    int i = (int)(t * TEX_SIZE);
    if (i < 0)             return 0;
    if (i > TEX_SIZE - 1)  return TEX_SIZE - 1;
    return i;
}

static void cb_texture(Vector2_t pixel, UV_t uv, double z, drawpixel_t drawpixel)
{
    (void)z;
    int tx = tex_coord(uv.u);
    int ty = tex_coord(uv.v);
    drawpixel(pixel, texture[ty * TEX_SIZE + tx]);
}

static inline double tri_area2(Vertex_t a, Vertex_t b, Vertex_t c) {
	return    (b.pos.x - a.pos.x) * (c.pos.y - a.pos.y)
		- (c.pos.x - a.pos.x) * (b.pos.y - a.pos.y);
}

void render_solid_texture(Vertex_t *vertexes, int count, drawpixel_t drawpixel)
{
    for (int i = 0; i + 2 < count; i += 3) {
	if (tri_area2(vertexes[i], vertexes[i + 1], vertexes[i + 2]) <= 0)
		continue;
        scanline_raster(vertexes[i], vertexes[i + 1], vertexes[i + 2],
                        cb_texture, drawpixel);
    }
}

void obj2world(int *polys, int polys_count, Vector3_t *vertices, Vector3_t *output) {
	for (int i = 0; i < polys_count; i++) {
		output[i] = vertices[polys[i] - 1];
	}
}
