typedef void (*drawpixel_t)(Vector2_t);
void drawline(Vector2_t a, Vector2_t b, drawpixel_t drawpixel);
void render_wireframe(Vector2_t *vertexes, int count, drawpixel_t drawpixel);
