#define H 256
#define W 256

typedef void (*drawpixel_t)(Vector2_t, unsigned char);
typedef void (*subfunc_t)(Vector2_t, UV_t, double, drawpixel_t);

void drawline(Vector2_t a, Vector2_t b, drawpixel_t drawpixel);
void render_wireframe(Vertex_t *vertexes, int count, drawpixel_t drawpixel);
void scanline_raster(Vertex_t p1, Vertex_t p2, Vertex_t p3,
		subfunc_t subfunc, drawpixel_t drawpixel);
void render_solid(Vertex_t *vertexes, int count, drawpixel_t drawpixel);
void render_solid_texture(Vertex_t *vertexes, int count, drawpixel_t drawpixel);
void clear_zbuf(void);
void obj2world(int *polys, int polys_count, Vector3_t *vertices, Vector3_t *output);
