typedef struct {
	double x,y,z;
} Vector3_t;

typedef struct {
	double x,y;
} Vector2_t;

typedef struct {
	double u, v;
} UV_t;

typedef struct {
	Vector2_t pos;
	UV_t uv;
} Vertex_t;

double fov_factor(double fov_in_deg, double width);
Vector3_t move_to_xyz(Vector3_t vertex, Vector3_t to);
Vector3_t euler_rotate(Vector3_t rot, Vector3_t vertex);
Vertex_t xyz_to_xy(Vector3_t vertex, double fov_factor,
		Vector2_t screen_dim);
