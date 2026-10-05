#include "custom_math.h"
#include <math.h>

double fov_factor(double fov_in_deg, double width) {
	return width / (2.0 * tan((fov_in_deg * M_PI) / 360.0));
}

Vector3_t move_to_xyz(Vector3_t vertex, Vector3_t to) {
	double x = vertex.x - to.x;
	double y = vertex.y - to.y;
	double z = vertex.z - to.z;
	Vector3_t result = {x,y,z};
	return result;
}

Vector3_t euler_rotate(Vector3_t rot, Vector3_t vertex) {
	// Yaw
	double cos_y = cos(-rot.x);
	double sin_y = sin(-rot.x);
	double x1 = vertex.x * cos_y - vertex.z * sin_y;
	double z1 = vertex.x * sin_y + vertex.z * cos_y;

	// Pitch
	double cos_x = cos(-rot.y);
	double sin_x = sin(-rot.y);
	double y2 = vertex.y * cos_x + z1 * sin_x;
	double z2 = -vertex.y * sin_x + z1 * cos_x;

	// Roll
	double cos_z = cos(-rot.z);
	double sin_z = sin(-rot.z);
	double x3 = x1 * cos_z - y2 * sin_z;
	double y3 = x1 * sin_z + y2 * cos_z;

	Vector3_t result = {x3, y3, z2};
	return result;
}

Vector2_t xyz_to_xy(Vector3_t vertex, double fov_factor,
		Vector2_t screen_dim) {
	Vector2_t result;

	if (vertex.z <= 0.0)
		vertex.z = 0.001;

	result.x = round(screen_dim.x / 2.0 + vertex.x / vertex.z * fov_factor);
	result.y = round(screen_dim.y / 2.0 - vertex.y / vertex.z * fov_factor);

	return result;
}
