static void
draw_rectf_clipped(u32 *vram, int pitch, int cx0, int cy0, int cw, int ch, int x0, int y0, int w, int h, u32 fill)
{
    for (int y = y0; y < y0 + h; ++y) {
        for (int x = x0; x < x0 + w; ++x) {
            if ((cx0 <= x && x <= cx0 + cw) && (cy0 <= y && y <= cy0 + ch)) {
                vram[y * pitch + x] = fill;
            }
        }
    }
}


static void
draw_rectf(u32 *vram, int pitch, int x0, int y0, int w, int h, u32 fill)
{
    for (int y = y0; y < y0 + h; ++y) {
        for (int x = x0; x < x0 + w; ++x) {
            vram[y * pitch + x] = fill;
        }
    }
}

static void
draw_rect(u32 *vram, int pitch, int x0, int y0, int w, int h, u32 stroke)
{
    for (int x = x0; x <= x0 + w; ++x) vram[y0 * pitch + x]       = stroke;
    for (int x = x0; x <= x0 + w; ++x) vram[(y0 + h) * pitch + x] = stroke;
    for (int y = y0; y <= y0 + h; ++y) vram[y * pitch + x0]       = stroke;
    for (int y = y0; y <= y0 + h; ++y) vram[y * pitch + x0 + w]   = stroke;
}
