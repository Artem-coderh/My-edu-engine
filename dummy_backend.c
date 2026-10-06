#include "backend.h"
#include "render.h"
#include <stdio.h>

char screen[H * W];

void drawpixel(Vector2_t a, unsigned char color) {
	if ((int)a.x >= 0 && (int)a.x < W && (int)a.y >= 0 && (int)a.y < H) {
		screen[(int)a.y * W + (int)a.x] = color;
	}
}

void print_fbdata() {
	for (int i = 0; i < sizeof(screen); i++) {
		printf("%c", screen[i]);
	}
}

void Render_frame(Vector3_t *vertices, int vertices_count, UV_t *uvs,
		Vector3_t camera_pos, Vector3_t camera_rot) {
	Vertex_t t[vertices_count];
	const Vector2_t screen_dim = {H, W};
	const double fov = fov_factor(75.0, 256.0);


	clear_zbuf();
	for (int i = 0; i < vertices_count; i++) {
		Vector3_t t_2 = euler_rotate(camera_rot, vertices[i]);
		t_2 = move_to_xyz(t_2, camera_pos);
		t[i] = xyz_to_xy(t_2, fov, screen_dim);
		if (uvs == NULL) {
			t[i].uv = (UV_t){0.0, 0.0};
		} else {
			t[i].uv = uvs[i];
		}
		t[i].z = t_2.z;
	}

	render_solid_shade(t, vertices_count, drawpixel, vertices, (Vector3_t){0.3, 0.6, -0.7});
//	render_solid(t, vertices_count, drawpixel);
//	render_wireframe(t, vertices_count, drawpixel);
}
