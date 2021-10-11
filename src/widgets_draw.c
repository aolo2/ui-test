static void
draw_textview(u32 *vram, int width)
{
    draw_rectf(vram, width, textview_x0, textview_y0, textview_fit, 200, textview_bg_color);
    struct v2 box = render_utf_string(global_font, 22, vram, width, textview_text, textview_fit, textview_x0, textview_y0, textview_text_color);
    draw_rect(vram, width, textview_x0, textview_y0, textview_fit, box.y, 0xffFFbb00);
    draw_rect(vram, width, textview_x0, textview_y0, box.x, box.y, 0xffbb0000);
    
    
    wchar_t clicks_as_wstring[32] = { 0 };
    swprintf(clicks_as_wstring, 15, L"%d", textview_clicks);
    render_utf_string(global_font, 22, vram, width, clicks_as_wstring, 100, textview_x0, textview_y0 + box.y + 20, 0xffffff);
}

static void
draw_button(u32 *vram, int width)
{
    int fill = 0x999999;
    int stroke = 0x666666;
    
    if (button_primed) {
        fill = 0x6666ff;
        if (button_down) {
            fill = 0xff6666;
        }
    }
    
    draw_rectf(vram, width, button_x0, button_y0, button_w, button_h, fill);
    draw_rect(vram, width, button_x0, button_y0, button_w, button_h, stroke);
    int text_width = get_string_width(&global_font, button_text_size, button_text, wcslen(button_text));
    render_utf_string(global_font, button_text_size, vram, width, button_text, button_w - 4, button_x0 + (button_w - text_width) / 2, button_y0 + (button_h - button_text_size) / 2, button_text_color);
}

static void
draw_checkbox(u32 *vram, int width)
{
    wchar_t *text = L"Check me, Senpai!";
    u32 fill = 0x999999;
    u32 stroke = checkbox_stroke;
    
    if (checkbox_primed) {
        stroke = 0x6666ff;
        if (checkbox_superprimed) {
            fill = 0x666666;
        }
    }
    
    draw_rectf(vram, width, checkbox_x0 + checkbox_side, checkbox_y0, checkbox_fit, checkbox_side, checkbox_bgcolor);
    draw_rectf(vram, width, checkbox_x0, checkbox_y0, checkbox_side, checkbox_side, fill);
    
    if (checkbox_checked) {
        text = L"Thank you uWu *drools*";
        draw_rectf(vram, width, checkbox_x0 + checkbox_side / 4, checkbox_y0 + checkbox_side / 4, checkbox_side / 2 + 1, checkbox_side / 2  + 1, 0xffffff);
    }
    
    draw_rect(vram, width, checkbox_x0, checkbox_y0, checkbox_side, checkbox_side, stroke);
    render_utf_string(global_font, checkbox_font_size, vram, width, text, checkbox_fit, checkbox_x0 + checkbox_side + 5, checkbox_y0 + (checkbox_side - checkbox_font_size) / 2, checkbox_text_color);
}

static void
draw_editview(u32 *vram, int width)
{
    u32 frame = 0xffffff;
    
    if (editview_focused) {
        frame = 0xff0000;
    }
    
    draw_rectf(vram, width, editview_x0, editview_y0, editview_w, editview_h, 0xeeeeee);
    draw_rect(vram, width, editview_x0, editview_y0, editview_w, editview_h, frame);
    
    if (editview_selection_active && (editview_cursor_position != editview_selection_to)) {
        int selection_start = get_string_width(&global_font, editview_font_size, editview_text, editview_cursor_position);
        int selection_end = get_string_width(&global_font, editview_font_size, editview_text, editview_selection_to);
        
        int selection_width = selection_end - selection_start;
        if (selection_width < 0) {
            int tmp = selection_end;
            selection_width = -selection_width;
            selection_end = selection_start;
            selection_start = tmp;
        }
        
        draw_rectf(vram, width, editview_x0 + editview_padding + selection_start, editview_y0 + 5, selection_width, editview_h - 10, editview_selection_fill);
    }
    
    render_utf_string(global_font, editview_font_size, vram, width, editview_text, editview_w - 20, editview_x0 + editview_padding, editview_y0 + (editview_h - editview_font_size) / 2, editview_text_color);
    
    if (editview_focused) {
        int cursor_x = get_string_width(&global_font, editview_font_size, editview_text, editview_cursor_position);
        draw_rectf(vram, width, editview_x0 + editview_padding + cursor_x, editview_y0 + 5, 1, editview_h - 10, editview_text_color);
    }
}

static void
draw_scrollview(u32 *vram, int width)
{
    draw_rectf(vram, width, scrollview_x0, scrollview_y0, scrollview_w, scrollview_h, 0x555555);
    draw_rectf(vram, width, scrollview_x0 + scrollview_w - scrollview_handle_width, scrollview_y0, scrollview_handle_width, scrollview_h, 0x222222);
    
    if (scrollview_desired_height > scrollview_h) {
        u32 handle_fill = 0x888888;
        
        if (scrollview_handle_primed) {
            handle_fill = 0xaaaaaa;
        }
        
        if (scrollview_handle_down) {
            handle_fill = 0x666666;
        }
        
        draw_rectf(vram, width, scrollview_x0 + scrollview_w - scrollview_handle_width, scrollview_y0 + scrollview_scrollbar_offset, scrollview_handle_width, scrollview_scrollbar_height, handle_fill);
    }
    
    int item_from = scrollview_margin;
    for (int i = 0; i < scrollview_itemcount; ++i) {
        int item_height = scrollview_items[i];
        int item_to = item_from + item_height;
        
        if ((scrollview_offset <= item_from && item_from <= scrollview_offset + scrollview_h)
            ||
            (scrollview_offset <= item_to && item_to <= scrollview_offset + scrollview_h)) {
            int adjusted_from = item_from - scrollview_offset;
            int adjusted_to = item_to - scrollview_offset;
            
            u32 r = ((i + 123) * 1236) % 256;
            u32 g = ((i + 321) * 6981) % 256;
            u32 b = ((i + 666) *   87) % 256;
            
            u32 color = (r << 16) | (g << 8) | b;
            
            draw_rectf_clipped(vram, width, scrollview_x0, scrollview_y0, scrollview_w, scrollview_h, scrollview_x0 + scrollview_margin, scrollview_y0 + adjusted_from, scrollview_w - scrollview_handle_width - scrollview_margin * 2, item_height, color);
        }
        
        item_from = item_to + scrollview_margin;
    }
}

static void
draw_select()
{
}

static void
draw_radio()
{
}

static void
draw_progress()
{
}

static void
draw_blended_rectf(u32 *vram, int width, int x0, int y0, int w, int h, u32 color, f32 alpha)
{
    for (int y = y0; y < y0 + h; ++y) {
        for (int x = x0; x < x0 + w; ++x) {
            u32 bg = vram[y * width + x];
            u32 fg = color;
            
            u8 bg_r = (bg & 0xff0000) >> 16;
            u8 bg_g = (bg & 0x00ff00) >> 8;
            u8 bg_b = bg & 0xff;
            
            u8 fg_r = (fg & 0xff0000) >> 16;
            u8 fg_g = (fg & 0x00ff00) >> 8;
            u8 fg_b = fg & 0xff;
            
            u8 blended_r = (1.0f - alpha) * bg_r + alpha * fg_r;
            u8 blended_g = (1.0f - alpha) * bg_g + alpha * fg_g;
            u8 blended_b = (1.0f - alpha) * bg_b + alpha * fg_b;
            
            u32 blended = 0xff000000 | (((u32) blended_r) << 16) | (((u32) blended_g) << 8) | blended_b;
            
            vram[y * width + x] = blended;
        }
    }
}

static void
draw_alpha_test(u32 *vram, int width)
{
    draw_blended_rectf(vram, width, 50, 50, 1000, 1000, 0x00ff00, 0.2f);
    draw_blended_rectf(vram, width, 100, 100, 200, 100, 0xffd700, 0.5f);
    draw_blended_rectf(vram, width, 150, 120, 500, 200, 0xff0000, 0.5f);
}
