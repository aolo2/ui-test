#include "common.h"

#define COMMAND_BUFFER_SIZE 1024
#define MAX_DIRTY_RECTANGLES 128

static struct ui_rectange dirty[MAX_DIRTY_RECTANGLES];
static struct ui_drawcall command_buffer[COMMAND_BUFFER_SIZE];
static int ndrawcalls;
static int ndirty;
static int pending_shm;
static int shm_ready_event;

static Display         *display;
static Window          window;
static GC              default_gc;
static XImage*         xwindow_buffer;
static Window          root_window;
static XShmSegmentInfo shminfo;

#include "draw.c"

static void *
open_xlib_window(int width, int height)
{
    void *result = 0;
    
    XVisualInfo visinfo = { 0 };
    XSetWindowAttributes window_attr = { 0 };
    
    display = XOpenDisplay(0);
    root_window = DefaultRootWindow(display);
    
    int default_screen = DefaultScreen(display);
    int screen_bit_depth = 24;
    
    XMatchVisualInfo(display, default_screen, screen_bit_depth, TrueColor, &visinfo);
    
    window_attr.bit_gravity = StaticGravity;
    window_attr.background_pixel = 0;
    window_attr.colormap = XCreateColormap(display, root_window, visinfo.visual, AllocNone);
    
    unsigned long attribute_mask = CWBitGravity | CWBackPixel | CWColormap | CWEventMask;
    
    window = XCreateWindow(display, root_window, 0, 0, width, height, 0, visinfo.depth, InputOutput, visinfo.visual, attribute_mask, &window_attr);
    XGrabPointer(display, window, False, ButtonPressMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    XSelectInput(display, window, SubstructureNotifyMask | ExposureMask | PointerMotionMask 
                 | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask
                 | EnterWindowMask | LeaveWindowMask | ButtonMotionMask | KeymapStateMask | FocusChangeMask);
    XStoreName(display, window, "ui");
    XMapWindow(display, window);
    XFlush(display);
    
    Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &WM_DELETE_WINDOW, 1);
    
    xwindow_buffer = XShmCreateImage(display, visinfo.visual, visinfo.depth, ZPixmap, NULL, &shminfo, width, height);
    shminfo.shmid = shmget(IPC_PRIVATE, width * height * sizeof(u32), IPC_CREAT | 0777);
    shminfo.shmaddr = result;
    shminfo.readOnly = False;
    result = shmat(shminfo.shmid, 0, 0);
    XShmAttach(display, &shminfo);
    
    default_gc = DefaultGC(display, default_screen);
    
    return(result);
}

static int
rectangles_overlap(struct ui_rectange a, struct ui_rectange b)
{
    int result = (a.x <= b.x + b.width) && (b.x <= a.x + a.width) && (a.y <= b.y + b.height) && (b.y <= a.y + a.height);
    return(result);
}

static struct ui_rectange
merge_overlapping_rectangles(struct ui_rectange a, struct ui_rectange b)
{
    struct ui_rectange result;
    
    int a_xend = a.x + a.width;
    int b_xend = b.x + b.width;
    
    int a_yend = a.y + a.height;
    int b_yend = b.y + b.height;
    
    result.x = (a.x < b.x ? a.x : b.x);
    result.y = (a.y < b.y ? a.y : b.y);
    result.width = (a_xend > b_xend ? a_xend : b_xend) - result.x;
    result.height = (a_yend > b_yend ? a_yend : b_yend) - result.y;
    
    return(result);
}

static void
add_draw_command(struct ui_drawcall command)
{
    if (ndrawcalls == 1024) {
        __builtin_trap();
    }
    
    command_buffer[ndrawcalls++] = command;
}

static void
redraw(struct ui_rectange rect, struct ui_context *ui)
{
    if (ndirty == 128) {
        __builtin_trap();
    }
    
    int merged = 0;
    
#if 0
    struct ui_drawcall command = { 0 };
    command.rect = rect;
    command.fill = 0xFF00FF00;
    command.stroke = 0xFFFF0000;
    
    add_draw_command(command);
#endif
    
    for (int i = 0; i < ndirty; ++i) {
        struct ui_rectange other = dirty[i];
        if (rectangles_overlap(rect, other)) {
            struct ui_rectange merged_rect = merge_overlapping_rectangles(rect, other);
            dirty[i] = merged_rect;
            merged = 1;
            break;
        }
    }
    
    if (!merged) {
        dirty[ndirty++] = rect;
    }
}

static void
commit_redraw(u32 *vram, struct ui_context *ui)
{
    for (int i = 0; i < ndrawcalls; ++i) {
        struct ui_drawcall command = command_buffer[i];
        draw_process_command(vram, ui, command);
    }
    
    ndrawcalls = 0;
}

static void
present(u32 *vram, struct ui_context *ui)
{
    commit_redraw(vram, ui);
    
    for (int i = 0; i < ndirty; ++i) {
        struct ui_rectange region = dirty[i];
        XShmPutImage(display, window, default_gc, xwindow_buffer, 0, 0, region.x, region.y, region.width, region.height, True);
        ++pending_shm;
    }
    
    ndirty = 0;
}

static void
handle_event(XEvent ev, struct ui_context *ui)
{
    if (ev.type == Expose) {
        XExposeEvent event = ev.xexpose;
        struct ui_rectange rect = { event.x, event.y, event.width, event.height };
        redraw(rect, ui);
    } else if (ev.type == shm_ready_event) {
        --pending_shm;
    }
}

static void
event_loop(u32 *vram, struct ui_context *ui)
{
    shm_ready_event = XShmGetEventBase(display) + ShmCompletion;
    
    XEvent ev;
    
    for (;;) {
        XPeekEvent(display, &ev); // NOTE(aolo2): block and wait for event
        
        while (pending_shm > 0 || XPending(display)) {
            XNextEvent(display, &ev);
            if (ev.type == shm_ready_event) {
                --pending_shm;
            } else {
                handle_event(ev, ui);
            }
        }
        
        if (ndirty > 0)  {
            present(vram, ui);
        }
    }
}

static void
handle_event(struct ui_event *event)
{
    // if mouse -> determine what widget handles
    // if keyboard -> check what's focused?
    
    ui_treeview_handle(watch_tree, event);
    ui_scrollview_handle(source_text, event);
}

static struct ui_context *
ui_init(int width, int height)
{
    struct ui_context *ui = ui_create_context(width, height);
    
    struct ui_container *c_root = ui_create_container(ui, 0, 0, 0, DIRECTION_VERTICAL);
    struct ui_container *c_main = ui_create_container(ui, c_root, 0, height - 32, DIRECTION_HORIZONTAL);
    struct ui_container *c_statusbar = ui_create_container(ui, c_root, 0, 0, DIRECTION_HORIZONTAL);
    
    struct ui_container *c_source = ui_create_container(ui, c_main, width / 3 * 2, 0, DIRECTION_NONE);
    struct ui_container *c_watch = ui_create_container(ui, c_main, 0, 0, DIRECTION_NONE);
    
    struct ui_textview *source_text = ui_create_scrollview(ui, c_source);
    struct ui_textview *status_text = ui_create_textview(ui, c_statusbar);
    struct ui_treeview *watch_tree = ui_create_treeview(ui, c_watch);
    
    // also padding
    // content clipped by parent rect, padding just moves and shrinks the rectu
    
    struct ui_editview *text_edit;
    struct ui_button *button;
    struct ui_checkbox *cb;
    struct ui_radio *radio;
    struct ui_tab *tab;
    struct ui_select *select;
    struct ui_image *image;
    struct ui_menu *menu;
    struct ui_progress *progress;
    
    return(ui);
}

int
main(void)
{
    int width = 1920;
    int height = 1080;
    int running = 1;
    
    struct ui_context *ui = ui_init(width, height);
    
    while (running) {
        ui_begin(ui); // NOTE(aolo2): can block
        
        // NOTE(aolo2): event_count can be zero when all events are non-userfacing
        for (int i = 0; i < ui->event_count; ++i) {
            struct ui_event *event = ui->events + i;
            if (event->type == EVENT_EXIT) {
                running = 0;
            } else {
                handle_event(event);
            }
        }
        
        ui_end(ui); // NOTE(aolo2): draw, including handling all expose events
    }
    
    return(0);
}



#if 0
for (int i = 0; i < command_count; ++i) {
    struct ui_command command = command_buffer[i];
    if (command.type == DRAW_RECT) {
        
    } else if (command.type == DRAW_LINE) {
        
    } else if (command.type == DRAW_TEXT) {
        
    }
}

for (int i = 0; i < ndirty; ++i) {
    struct ui_rectange region = dirty[i];
    XShmPutImage(display, window, default_gc, xwindow_buffer, 0, 0, region.x, region.y, region.width, region.height, True);
    ++pending_shm;
}
#endif