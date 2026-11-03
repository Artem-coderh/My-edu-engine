#include <cglm/cglm.h>
#include <cglm/struct/vec2.h>
#include <cglm/struct/vec3.h>
#include <string.h>
#include "dragon.h"
#include "cube.h"
#include "texture.h"

#define TEX_W 64
#define TEX_H 64


#define W 1024
#define H 1024

typedef struct {
	vec3s pos;
	vec2s uv;
} vert;
typedef void (*shader_t)(vec2 xy, vec2 uv, double z);

char g_screen[W * H];
float g_zbuf[W][H];
unsigned char g_shade = 255;

void drawpixel(int x, int y, unsigned char color) {
	if (x >= 0 && x < W && y >= 0 && y < H) {
		g_screen[y * W + x] = color;
	}
}

double fov_factor(double fov, double width) {
	return width / (2.0 * tan(glm_rad(fov)) / 2);
}

vec2s projection(vec3 vertex, double fov_factor,
		vec2 screen_dim) {
	vec2s result;
	result.x = round(screen_dim[0] / 2.0 + vertex[0] / vertex[2] * fov_factor);
	result.y = round(screen_dim[1] / 2.0 - vertex[1] / vertex[2] * fov_factor);
	return result;
}

void drawline(vec2 a, vec2 b, unsigned char color) {
	int x1 = (int)a[0];
	int y1 = (int)a[1];
	int x2 = (int)b[0];
	int y2 = (int)b[1];
	int dx = abs(x2 - x1);
	int dy = -abs(y2 - y1);
	int sx = x1 < x2 ? 1 : -1;
	int sy = y1 < y2 ? 1 : -1;
	int err = dx + dy;

	while (1) {
		drawpixel(x1, y1, color);

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

float SignedTriangleArea(vec3 a, vec3 b, vec3 c) {
	return 0.5 * ((b[0] - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (b[1] - a[1]));
}

void rasterizer(vert a, vert b, vert c, shader_t shader) {
	int min_x = fmin(a.pos.raw[0], fmin(b.pos.raw[0], c.pos.raw[0]));
	int min_y = fmin(a.pos.raw[1], fmin(b.pos.raw[1], c.pos.raw[1]));
	int max_x = fmax(a.pos.raw[0], fmax(b.pos.raw[0], c.pos.raw[0]));
	int max_y = fmax(a.pos.raw[1], fmax(b.pos.raw[1], c.pos.raw[1]));
	float area = SignedTriangleArea(a.pos.raw, b.pos.raw, c.pos.raw), alpha, beta, gamma, inv_z, u, v, side;
	vec2 ab, ac;

	glm_vec2_sub(b.pos.raw, a.pos.raw, ab);
	glm_vec2_sub(c.pos.raw, a.pos.raw, ac);
	side = glm_vec2_cross(ab, ac);
	if (side <= 0.0) return;

	for (int x = min_x; x <= max_x; x++) {
		for (int y = min_y; y <= max_y; y++) {
			alpha = SignedTriangleArea((vec3){x, y, 0}, b.pos.raw, c.pos.raw) / area;
			beta  = SignedTriangleArea((vec3){x, y, 0}, c.pos.raw, a.pos.raw) / area;
			gamma = SignedTriangleArea((vec3){x, y, 0}, a.pos.raw, b.pos.raw) / area;
			if (alpha < 0 || beta < 0 || gamma < 0) continue;

			inv_z = (alpha / a.pos.raw[2]) + (beta / b.pos.raw[2]) + (gamma / c.pos.raw[2]);
			if (inv_z < g_zbuf[x][y]) continue;
			g_zbuf[x][y] = inv_z;

			u = ((alpha * a.uv.raw[0] / a.pos.raw[2])
			   + (beta  * b.uv.raw[0] / b.pos.raw[2])
			   + (gamma * c.uv.raw[0] / c.pos.raw[2])) / inv_z;


			v = ((alpha * a.uv.raw[1] / a.pos.raw[2])
			   + (beta  * b.uv.raw[1] / b.pos.raw[2])
			   + (gamma * c.uv.raw[1] / c.pos.raw[2])) / inv_z;

			shader((vec2){x, y}, (vec2){u, v}, inv_z);
		}
	}
}

double face_light(vec3 p0, vec3 p1, vec3 p2, vec3 light) {
	vec3 e1, e2, n;
	glm_vec3_sub(p1, p0, e1);
	glm_vec3_sub(p2, p0, e2);
	glm_vec3_cross(e1, e2, n);
	glm_vec3_normalize(n);

	double len = glm_vec3_norm(n);
	if (len == 0.0) return 0.0;
	double d = glm_vec3_dot(n, light) / len;
	return d > 0.0 ? d : 0.0;
}

// Render func

void render_solid(vert *vertexes, int count) {
	void cb_filler(vec2 xy, vec2 uv, double z) {
		drawpixel(xy[0], xy[1], 0xff);
	}

	for (int i = 0; i < count; i += 3) {
		rasterizer(vertexes[i], vertexes[i + 1], vertexes[i + 2],
			cb_filler);

	}
	return;
}

void render_solid_shade(vert *vertexes, int count, vec3 *vert3, vec3 light) {
	void cb_shaded(vec2 xy, vec2 uv, double z) {
		drawpixel(xy[0], xy[1], g_shade);
	}

	for (int i = 0; i < count; i += 3) {
		g_shade = 30 + 225 * face_light(vert3[i], vert3[i + 1], vert3[i + 2], light);
		rasterizer(vertexes[i], vertexes[i + 1], vertexes[i + 2],
				cb_shaded);
	}
	return;
}

void render_wireframe(vert *vertexes, int count) {
	for (int i = 0; i < count; i += 3) {
		drawline(vertexes[0 + i].pos.raw, vertexes[1 + i].pos.raw, 0xff);
		drawline(vertexes[1 + i].pos.raw, vertexes[2 + i].pos.raw, 0xff);
		drawline(vertexes[2 + i].pos.raw, vertexes[0 + i].pos.raw, 0xff);
	}
	return;
}

void render_solid_texture(vert *vertexes, int count) {
	void cb_texture(vec2 xy, vec2 uv, double z) {
		int tx, ty;
		tx = uv[0] * TEX_W;
		ty = uv[1] * TEX_H;

		if (tx < 0) tx = 0;
		if (tx > TEX_W - 1) tx = TEX_W - 1;

		if (ty < 0) ty = 0;
		if (ty > TEX_H - 1) ty = TEX_H - 1;

		drawpixel(xy[0], xy[1], texture[ty * TEX_W + tx]);
	}

	for (int i = 0; i < count; i += 3) {
		rasterizer(vertexes[i], vertexes[i + 1], vertexes[i + 2],
			cb_texture);
	}
	return;
}

void RenderModel(vec3 *vertices, int vertices_count, vec2 *uvs) {
	vert *projected_vertices = malloc(vertices_count * sizeof(vert));
	vec2 screen_dim = {W, H};
	vec2s tmp;
	double fov = fov_factor(75.0, W);

	for (int i = 0; i < vertices_count; i++) {
		glm_vec3_rotate(vertices[i], glm_rad(-30), (vec3){0, 1, 0});
//		vertices[i][2] -= -1.3;
		vertices[i][2] -= -2;
//		vertices[i][1] -= 0.2;

		tmp = projection(vertices[i], fov, screen_dim);
		projected_vertices[i].pos = (vec3s){tmp.x, tmp.y, vertices[i][2]};
		if (uvs != NULL)
			projected_vertices[i].uv  = (vec2s){uvs[i][0], uvs[i][1]};
	}
	render_solid_shade(projected_vertices, vertices_count, vertices, (vec3){0.3, 0.6, -0.7});
//	render_solid(projected_vertices, vertices_count);
//	render_solid_texture(projected_vertices, vertices_count);
//	render_wireframe(projected_vertices, vertices_count);
	free(projected_vertices);
}

void obj2world(int *polys, int polys_count, vec3 *vertices, vec3 *output) {
	for (int i = 0; i < polys_count; i++) {
		output[i][0] = vertices[polys[i] - 1][0];
		output[i][1] = vertices[polys[i] - 1][1];
		output[i][2] = vertices[polys[i] - 1][2];
	}
}

int main() {
	vec3 *output = malloc(300010 * sizeof(vec3));
	obj2world(polys, 300000, vertices, output);

	RenderModel(cube_vertices, 36, NULL);
	RenderModel(output, 300000, NULL);

	free(output);
	for (int i = 0; i < sizeof(g_screen); i++) {
		printf("%c", g_screen[i]);
	}

}
