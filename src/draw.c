static void
draw_rectangle(u32 *vram, int width, int height, struct ui_rectange rect, u32 fill, u32 stroke)
{
    // NOTE(aolo2): clip to screen
    if (rect.x >= width) {
        rect.width = 0;
    } else if (rect.x + rect.width >= width) {
        rect.width = width - 1 - rect.x;
    }
    
    if (rect.y >= height) {
        rect.height = 0;
    } else if (rect.y + rect.height >= height) {
        rect.height = height - 1 - rect.y;
    }
    
    if (rect.width == 0 || rect.height == 0) {
        return;
    }
    
    for (int y = 0; y < rect.height; ++y) {
        for (int x = 0; x < rect.width; ++x) {
            vram[(y + rect.y) * width + x + rect.x] = fill;
        }
    }
    
    for (int x = 0; x < rect.width; ++x) {
        vram[rect.y * width + x + rect.x] = stroke;
    }
    
    for (int x = 0; x < rect.width; ++x) {
        vram[(rect.y + rect.height) * width + x + rect.x] = stroke;
    }
    
    for (int y = 0; y < rect.height; ++y) {
        vram[(y + rect.y) * width + rect.x] = stroke;
    }
    
    for (int y = 0; y < rect.height; ++y) {
        vram[(y + rect.y) * width + rect.x + rect.width] = stroke;
    }
}

static void
draw_text()
{
    
}

static void
draw_process_command(u32 *vram, struct ui_context *ui, struct ui_drawcall command)
{
    switch (command.type) {
        case COMMAND_DRAW_RECTANGLE: {
            draw_rectangle(vram, ui, command.rect, command.fill, command.stroke);
            break;
        }
        
        case COMMAND_DRAW_TEXT: {
            draw_text();
        }
    }
}