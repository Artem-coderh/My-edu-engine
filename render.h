typedef void (*drawpixel_t)(Vector2_t);
typedef void (*rastersubfunc_t)(Vector2_t, drawpixel_t);

void drawline(Vector2_t a, Vector2_t b, drawpixel_t drawpixel);
void render_wireframe(Vector2_t *vertexes, int count, drawpixel_t drawpixel);
void scanline_raster(Vector2_t p1, Vector2_t p2, Vector2_t p3,
                rastersubfunc_t subfunc, drawpixel_t drawpixel);
void render_solid(Vector2_t *vertexes, int count, drawpixel_t drawpixel);
