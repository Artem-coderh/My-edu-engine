#include "custom_math.h"
#include "render.h"
#include <stdio.h>

#define H 256
#define W 256
char screen[H * W];

void drawpixel(Vector2_t a) {
	if ((int)a.x >= 0 && (int)a.x < W && (int)a.y >= 0 && (int)a.y < H) {
		screen[(int)a.y * W + (int)a.x] = 0xff;
	}
}

void print_fbdata() {
	for (int i = 0; i < sizeof(screen); i++) {
		printf("%c", screen[i]);
	}
}

void Render_frame(Vector3_t *vertices, int vertices_count,
		Vector3_t camera_pos, Vector3_t camera_rot) {
	Vector2_t t[vertices_count];
	const Vector2_t screen_dim = {H, W};
	const double fov = fov_factor(75.0, 256.0);

	for (int i = 0; i < vertices_count; i++) {
		Vector3_t t_2 = move_to_xyz(vertices[i], camera_pos);
		t_2 = euler_rotate(camera_rot, t_2);
		t[i] = xyz_to_xy(t_2, fov, screen_dim);
	}

	render_wireframe(t, vertices_count, drawpixel);
}
