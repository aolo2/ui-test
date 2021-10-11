static int textview_x0;
static int textview_y0;
static int textview_fit;
static u32 textview_bg_color;
static u32 textview_text_color;
static int textview_clicks;
static wchar_t *textview_text =  L"Textview:\nLorem ipsum dolor sit amet\nconsectetur adipiscing elit. Vestibulum...";

static int
update_textview(int width)
{
    textview_x0 = 10;
    textview_y0 = 10;
    textview_fit = width / 4;
    textview_bg_color = 0x555555;
    textview_text_color = 0xee55ee;
    
    int happened = 0;
    
    if (number_of_clicks != textview_clicks) {
        happened = 1;
        textview_clicks = number_of_clicks;
    }
    
    return(happened);
}

static u32 button_fill;
static u32 button_text_color;
static int button_text_size;
static int button_x0;
static int button_y0;
static int button_w;
static int button_h;
static int button_primed;
static int button_down;
static int button_clicked;
static wchar_t *button_text = L"Button";

static int
update_button(int width, int height)
{
    button_fill = 0xFF777777;
    button_text_color = 0x000000;
    button_text_size = 32;
    button_x0 = width / 2;
    button_y0 = height / 2;
    button_w = 120;
    button_h = button_text_size + 8;
    
    int new_button_primed = button_primed;
    int new_button_down = button_down;
    
    if (mouse_down && mouse_halftransitions > 0) {
        if (button_primed) {
            new_button_down = 1;
        }
    }
    
    if (!mouse_down && mouse_halftransitions > 0) {
        if (button_down && button_primed) {
            new_button_down = 0;
            ++number_of_clicks;
        }
    }
    
    if ((button_x0 <= mouse_x && mouse_x <= button_x0 + button_w) && (button_y0 <= mouse_y && mouse_y <= button_y0 + button_h)) {
        new_button_primed = 1;
    } else {
        new_button_primed = 0;
    }
    
    int happened = 0;
    
    if (new_button_primed != button_primed) {
        happened = 1;
        button_primed = new_button_primed;
    }
    
    if (new_button_down != button_down) {
        happened = 1;
        button_down = new_button_down;
    }
    
    return(happened);
}

static u32 checkbox_stroke;
static u32 checkbox_bgcolor;
static u32 checkbox_text_color;
static int checkbox_font_size;
static int checkbox_fit;
static int checkbox_x0;
static int checkbox_y0;
static int checkbox_side;
static int checkbox_primed;
static int checkbox_superprimed;
static int checkbox_checked;

static int
update_checkbox(int width, int height)
{
    checkbox_stroke = 0xffffff;
    checkbox_bgcolor = 0x222222;
    checkbox_font_size = 32;
    checkbox_fit = width / 4;
    checkbox_text_color = 0x00ff00;
    
    checkbox_x0 = 10;
    checkbox_y0 = height / 2;
    checkbox_side = 32;
    
    int new_checkbox_primed = checkbox_primed;
    int new_checkbox_checked = checkbox_checked;
    int new_checkbox_superprimed = checkbox_superprimed;
    
    if (mouse_down && mouse_halftransitions > 0) {
        if (checkbox_primed) {
            new_checkbox_superprimed = 1;
        }
    }
    
    if (!mouse_down && mouse_halftransitions > 0) {
        if (new_checkbox_superprimed && new_checkbox_primed) {
            new_checkbox_checked = 1 - new_checkbox_checked;
        }
        new_checkbox_superprimed = 0;
    }
    
    if ((checkbox_x0 <= mouse_x && mouse_x <= checkbox_x0 + checkbox_side) && (checkbox_y0 <= mouse_y && mouse_y <= checkbox_y0 + checkbox_side)) {
        new_checkbox_primed = 1;
    } else {
        new_checkbox_primed = 0;
    }
    
    int happened = 0;
    
    if (new_checkbox_checked != checkbox_checked) {
        happened = 1;
        checkbox_checked = new_checkbox_checked;
    }
    
    if (new_checkbox_primed != checkbox_primed) {
        happened = 1;
        checkbox_primed = new_checkbox_primed;
    }
    
    if (new_checkbox_superprimed != checkbox_superprimed) {
        happened = 1;
        checkbox_superprimed = new_checkbox_superprimed;
    }
    
    return(happened);
}

static wchar_t editview_text[128] = L"fooo     baar/baz/boo wow123";
static int     editview_text_length;
static int     editview_focused;
static int     editview_cursor_position; // letter BEFORE which the cursor is
static int     editview_selection_active;
static int     editview_selection_to;

static int editview_font_size;
static u32 editview_selection_fill;
static u32 editview_stroke;
static u32 editview_frame; 
static u32 editview_text_color;

static int editview_x0;
static int editview_y0;
static int editview_w; 
static int editview_h;
static int editview_padding;

static int
is_wword(wchar_t c)
{
    int is_latin = (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'));
    int is_cyrillic = ((L'а' <= c && c <= L'я') || (L'А' <= c && c <= L'Я'));
    int is_number = ('0' <= c && c <= '9');
    
    int result = is_latin || is_cyrillic || is_number;
    
    return(result);
}

static void
editview_move_caret(int left, int wordwise, int selection,
                    int *cursor, int *selection_active, int *selection_to)
{
    int new_cursor = *cursor;
    int dir = (left ? -1 : 1);
    
    if (wordwise) {
        // skip till end of word (place cursor before first non-word char)
        
        int len = wcslen(editview_text);
        int p = new_cursor;
        
        if (!left) {
            int started_inside_word = is_wword(editview_text[p]);
            if (!started_inside_word) {
                // skip all whitespace
                while (p < len) {
                    ++p;
                    if (is_wword(editview_text[p])) {
                        break;
                    }
                }
            }
            
            // skip the word
            while (p < len) {
                ++p;
                if (!is_wword(editview_text[p])) {
                    break;
                }
            }
        } else if (p > 0) {
            int started_inside_word = is_wword(editview_text[p - 1]);
            if (!started_inside_word) {
                // skip all whitespace
                while (p > 0) {
                    --p;
                    if (is_wword(editview_text[p])) {
                        break;
                    }
                }
            }
            
            // skip the word
            while (p > 0) {
                --p;
                if (!is_wword(editview_text[p])) {
                    break;
                }
            }
            
            if (p > 0) {
                ++p; // need to place the cursor before the first LETTER INSIDE A WORD
            }
        }
        
        dir = p - new_cursor;
    }
    
    if (selection) {
        if (!(*selection_active)) {
            *selection_to = new_cursor;
            *selection_active = 1;
        }
        new_cursor += dir;
        *cursor = new_cursor;
    } else {
        if (*selection_active) {
            if (left) {
                new_cursor = MIN(*cursor, *selection_to);
            } else {
                new_cursor = MAX(*cursor, *selection_to);
            }
        } else {
            new_cursor += dir;
        }
        
        *selection_active = 0;
        *cursor = new_cursor;
    }
}


static int
editview_is_printable(wchar_t c)
{
    int is_latin = (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'));
    int is_cyrillic = ((L'а' <= c && c <= L'я') || (L'А' <= c && c <= L'Я'));
    int is_number = ('0' <= c && c <= '9');
    int is_shiftnumber = (c == '!' || c == '@' || c == '#' || c == '$' || c == '%' || c == '^' || c == '&' || c == '*' || c == '(' || c == ')' || c == '_' || c == '+' || c == '=' || c == '-');
    int is_misc = (c == '~' || c == '`' || c == ';' || c == ':' || c == '<' || c == '>' || c == '.' || c == ',' || c == '/' || c == '\\' || c == '|' || c == ']' || c == '[' || c == '{' || c == '}'
                   || c == L'№' || c == '?' || c == '\"');
    int is_space = (c == ' ');
    
    return(is_latin || is_cyrillic || is_number || is_shiftnumber || is_misc || is_space);
}

static void
editview_delete_selected(int l, int *cursor, int selection_to)
{
    int new_cursor = *cursor;
    int from = MIN(new_cursor, selection_to);
    int to = MAX(new_cursor, selection_to);
    
    memmove(editview_text + from, editview_text + to, (l - to + 1) * sizeof(wchar_t));
    
    l -= (to - from);
    editview_text[l] = 0;
    
    if (selection_to > new_cursor) {
        new_cursor = selection_to - (to - from);
    } else {
        new_cursor = selection_to;
    }
    
    *cursor = new_cursor;
}


static int
update_editview(int height)
{
    editview_stroke = 0xffffffff;
    editview_frame = editview_stroke;
    editview_font_size = 32;
    editview_text_color = 0x000000;
    editview_selection_fill = 0x8888ff;
    
    editview_x0 = 100;
    editview_y0 = height / 4;
    editview_w = 600;
    editview_h = editview_font_size + 10;
    editview_padding = 10;
    
    editview_text_length = wcslen(editview_text);
    
    int new_editview_focused = editview_focused;
    int new_editview_cursor_position = editview_cursor_position;
    int new_editview_selection_active = editview_selection_active;
    int new_editview_selection_to = editview_selection_to;
    
    int editview_text_changed = 0;
    
    /* focus / unfocus */
    if ((editview_x0 <= mouse_x && mouse_x <= editview_x0 + editview_w) && (editview_y0 <= mouse_y && mouse_y <= editview_y0 + editview_h)) {
        if (mouse_down && mouse_halftransitions > 0) {
            new_editview_focused = 1;
        }
    } else if (mouse_down && mouse_halftransitions > 0) {
        new_editview_focused = 0;
    }
    
    /* keys */
    if (new_editview_focused) {
        for (int k = 0; k < type_buffer_size; ++k) {
            struct keyboard_input key = type_buffer[k];
            
            switch (key.symbol) {
                case XK_Left: {
                    editview_move_caret(1, key.ctrl, key.shift, 
                                        &new_editview_cursor_position, &new_editview_selection_active, &new_editview_selection_to);
                    break;
                }
                
                case XK_Right: {
                    editview_move_caret(0, key.ctrl, key.shift,
                                        &new_editview_cursor_position, &new_editview_selection_active, &new_editview_selection_to);
                    break;
                }
                
                case XK_Home:
                case XK_End: {
                    if (key.shift) {
                        new_editview_selection_active = 1;
                        new_editview_selection_to = new_editview_cursor_position;
                    } else {
                        new_editview_selection_active = 0;
                    }
                    new_editview_cursor_position = (key.symbol == XK_Home) ? 0 : editview_text_length;
                    break;
                }
                
                case XK_Delete:
                case XK_BackSpace: {
                    int l = wcslen(editview_text);
                    
                    if (new_editview_selection_active && new_editview_cursor_position != new_editview_selection_to) {
                        editview_delete_selected(l, &new_editview_cursor_position, new_editview_selection_to);
                        editview_text_changed = 1;
                        new_editview_selection_active = 0;
                    } else {
                        if (key.symbol == XK_Delete) {
                            if (new_editview_cursor_position < l) {
                                memmove(editview_text + new_editview_cursor_position, editview_text + new_editview_cursor_position + 1, (l - new_editview_cursor_position) * sizeof(wchar_t));
                                editview_text_changed = 1;
                            }
                        } else if (key.symbol == XK_BackSpace) {
                            if (new_editview_cursor_position == l && l > 0) {
                                editview_text[l - 1] = 0;
                                editview_text_changed = 1;
                                --new_editview_cursor_position;
                            } else if (new_editview_cursor_position > 0) {
                                memmove(editview_text + new_editview_cursor_position - 1, editview_text + new_editview_cursor_position, (l - new_editview_cursor_position + 1) * sizeof(wchar_t));
                                editview_text_changed = 1;
                                --new_editview_cursor_position;
                            }
                        }
                    }
                    
                    break;
                }
                
                default: {
                    if (key.symbol != NoSymbol) {
                        int l = wcslen(editview_text);
                        wchar_t c = key.symbol;
                        
                        if (editview_is_printable(key.symbol)) {
                            if (new_editview_selection_active && new_editview_cursor_position != new_editview_selection_to) {
                                editview_delete_selected(l, &new_editview_cursor_position, new_editview_selection_to);
                                editview_text_changed = 1;
                            }
                            
                            if (new_editview_cursor_position == l) {
                                editview_text[l] = c;
                                editview_text[l + 1] = 0;
                                editview_text_changed = 1;
                            } else {
                                memmove(editview_text + new_editview_cursor_position + 1,
                                        editview_text + new_editview_cursor_position,
                                        (l - new_editview_cursor_position + 1) * sizeof(wchar_t));
                                editview_text[new_editview_cursor_position] = c;
                                editview_text_changed = 1;
                            }
                            ++new_editview_cursor_position;
                            new_editview_selection_to = new_editview_cursor_position;
                        }
                    }
                }
            }
        }
    }
    
    /* post-process (clamp etc) */
    int new_text_length = wcslen(editview_text);
    if (new_editview_cursor_position < 0) {
        new_editview_cursor_position = 0;
    }
    if (new_editview_cursor_position > new_text_length) {
        new_editview_cursor_position = new_text_length;
    }
    
    int happened = 0;
    
    if (editview_focused != new_editview_focused) {
        happened = 1;
        editview_focused = new_editview_focused;
    }
    
    if (editview_cursor_position != new_editview_cursor_position) {
        happened = 1;
        editview_cursor_position = new_editview_cursor_position;
    }
    
    if (editview_selection_active != new_editview_selection_active) {
        happened = 1;
        editview_selection_active = new_editview_selection_active;
    }
    
    if (editview_selection_to != new_editview_selection_to) {
        happened = 1;
        editview_selection_to = new_editview_selection_to;
    }
    
    if (editview_text_changed) {
        happened = 1;
    }
    
    return(happened);
}

static int scrollview_x0;
static int scrollview_y0;
static int scrollview_w;
static int scrollview_h;
static int scrollview_handle_width;
static int scrollview_items[] = { 10, 50, 20, 12, 15, 15, 15, 100, 100, 100, 15, 15, 15, 15, 100, 100, 100, 100, 5 };
static int scrollview_itemcount;
static int scrollview_scrollbar_width;
static int scrollview_margin;
static int scrollview_desired_height;

static int
update_scrollview(int width)
{
    scrollview_x0 = width / 2 + 100;
    scrollview_y0 = 10;
    scrollview_handle_width = 20;
    scrollview_w = 300;
    scrollview_h = 500;
    
    scrollview_itemcount = sizeof(scrollview_items) / sizeof(scrollview_items[0]);
    scrollview_scrollbar_width = 20;
    scrollview_margin = 5;
    
    scrollview_desired_height = scrollview_margin;
    
    for (int i = 0; i < scrollview_itemcount; ++i) {
        scrollview_desired_height += scrollview_items[i];
        scrollview_desired_height += scrollview_margin;
    }
    
    if ((scrollview_x0 <= mouse_x && mouse_x <= scrollview_x0 + scrollview_w) && (scrollview_y0 <= mouse_y && mouse_y <= scrollview_y0 + scrollview_h)) {
        scrollview_focused = 1;
    } else {
        scrollview_focused = 0;
    }
    
    int new_scrollview_offset = scrollview_offset;
    
    if (scrollview_focused) {
        if (mouse_scrolled_down) {
            new_scrollview_offset += mouse_scroll_speed * mouse_scrolled_down;
        }
        
        if (mouse_scrolled_up) {
            new_scrollview_offset -= mouse_scroll_speed * mouse_scrolled_up;
        }
        
        if (scrollview_offset < 0) {
            new_scrollview_offset = 0;
        }
        
        if (scrollview_desired_height > scrollview_h && new_scrollview_offset > scrollview_desired_height - scrollview_h) {
            new_scrollview_offset = scrollview_desired_height - scrollview_h;
        }
        
        // snapping
        if (new_scrollview_offset < scrolling_threshold) {
            new_scrollview_offset = 0;
        }
        
        if (scrollview_desired_height > scrollview_h && scrollview_desired_height - scrollview_h - new_scrollview_offset < scrolling_threshold) {
            new_scrollview_offset = scrollview_desired_height - scrollview_h;
        }
    }
    
    int happened = 0;
    
    if (new_scrollview_offset != scrollview_offset) {
        happened = 1;
        scrollview_offset = new_scrollview_offset;
    }
    
    return(happened);
}

static int
update_select()
{
    return(0);
}

static int
update_radio()
{
    return(0);
}

static int
update_progress()
{
    return(0);
}