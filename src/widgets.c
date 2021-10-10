#include "common.h"

#include "../../ttf-test/ttf.h"
#include "../../ttf-test/ttf_engine.c"
#include "../../ttf-test/ttf_parse.c"
#include "../../ttf-test/ttf_rasterize.c"

static struct ttf_font global_font;
static int number_of_clicks;

static struct ui_command command_buffer[128];
static struct ui_rectange dirty[128];
static int command_count;
static int ndirty;

static int scrollview_focused;
static int scrollview_offset;

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_halftransitions;
static int mouse_scrolled_up;
static int mouse_scrolled_down;
static int mouse_scroll_speed = 30;
static struct keyboard_input type_buffer[256]; // TODO: @overflow
static int type_buffer_size;
static int lctrl_down;
static int rctrl_down;
static int lalt_down;
static int ralt_down;

static int scrolling_threshold = 5;

static Display         *display;
static Window          window;
static GC              default_gc;
static XImage*         xwindow_buffer;
static Window          root_window;
static XShmSegmentInfo shminfo;
static int             shm_ready_event;

static int pending_shm;

static int happened_textview;
static int happened_button;
static int happened_checkbox;
static int happened_editview;
static int happened_scrollview;
static int happened_select;
static int happened_radio;
static int happened_progress;

#include "rendering_primitives.c"
#include "widgets_update.c"
#include "widgets_draw.c"

static void *
create_window(int width, int height)
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
    long event_mask = SubstructureNotifyMask | ExposureMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask | ButtonMotionMask | KeymapStateMask | FocusChangeMask;
    
    window = XCreateWindow(display, root_window, 0, 0, width, height, 0, visinfo.depth, InputOutput, visinfo.visual, attribute_mask, &window_attr);
    XGrabPointer(display, window, False, ButtonPressMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    XSelectInput(display, window, event_mask);
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
    shm_ready_event = XShmGetEventBase(display) + ShmCompletion;
    
    return(result);
}

static void
present(int width, int height)
{
    XShmPutImage(display, window, default_gc, xwindow_buffer, 0, 0, 0, 0, width, height, True);
    ++pending_shm;
}

static void
update_widgets(int width, int height)
{
    happened_button = update_button(width, height);
    happened_textview = update_textview(width);
    happened_checkbox = update_checkbox(width, height);
    happened_editview = update_editview(height);
    happened_scrollview = update_scrollview();
    happened_select = update_select();
    happened_radio = update_radio();
    happened_progress = update_progress();
}

static void
draw_widgets(u32 *vram, int width, int height)
{
    (void) height;
    
    static int first_draw = 1;
    
    if (first_draw) {
        first_draw = 0;
        draw_textview(vram, width);
        draw_button(vram, width);
        draw_checkbox(vram, width);
        draw_editview(vram, width);
        draw_scrollview(vram, width);
        draw_select();
        draw_radio();
        draw_progress();
    }
    
    if (happened_textview)   draw_textview(vram, width);
    if (happened_button)     draw_button(vram, width);
    if (happened_checkbox)   draw_checkbox(vram, width);
    if (happened_editview)   draw_editview(vram, width);
    if (happened_scrollview) draw_scrollview(vram, width);
    if (happened_select)     draw_select();
    if (happened_radio)      draw_radio();
    if (happened_progress)   draw_progress();
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s width height font\n", argv[0]);
        return(1);
    }
    
    int width = strtod(argv[1], 0);
    int height = strtod(argv[2], 0);
    
    global_font = parse_ttf_file(argv[3]);
    
    u32 *vram = create_window(width, height);
    memset(vram, 0xff333333, width * height * sizeof(u32));
    
    XEvent ev;
    struct timespec ts;
    
    for (;;) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        u64 frame_start_usec = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
        
        XPeekEvent(display, &ev); // blocks if no events
        
        //printf("events\n");
        
        mouse_scrolled_up = 0;
        mouse_scrolled_down = 0;
        mouse_halftransitions = 0;
        type_buffer_size = 0;
        
        int only_shm_event = 1;
        
        while (pending_shm > 0 || XPending(display)) {
            XNextEvent(display, &ev);
            
            if (ev.type != shm_ready_event) {
                only_shm_event = 0;
            }
            
            if (ev.type == shm_ready_event) {
                --pending_shm;
            } else if (ev.type == MotionNotify) {
                XMotionEvent motion = ev.xmotion;
                mouse_x = motion.x;
                mouse_y = motion.y;
            } else if (ev.type == ButtonPress) {
                XButtonEvent press = ev.xbutton;
                if (press.button == Button1) {
                    if (!mouse_down) {
                        ++mouse_halftransitions;
                    }
                    mouse_down = 1;
                } else if (press.button == Button4) {
                    ++mouse_scrolled_up;
                } else if (press.button == Button5) {
                    ++mouse_scrolled_down;
                }
            } else if (ev.type == ButtonRelease) {
                XButtonEvent release = ev.xbutton;
                if (release.button == 1) {
                    if (mouse_down) {
                        ++mouse_halftransitions;
                    }
                    mouse_down = 0;
                }
            } else if (ev.type == KeyPress) {
                XKeyEvent key = ev.xkey;
                
                int shift_pressed = key.state & ShiftMask;
                int ctrl_pressed = key.state & ControlMask;
                int caps_on = key.state & LockMask;
                int symbol = XLookupKeysym(&key, (shift_pressed ^ caps_on) ? 1 : 0);
                
                type_buffer[type_buffer_size].symbol = symbol;
                type_buffer[type_buffer_size].shift = shift_pressed;
                type_buffer[type_buffer_size].ctrl = ctrl_pressed;
                
                ++type_buffer_size;
            }
        }
        
        if (!only_shm_event) {
            update_widgets(width, height);
            draw_widgets(vram, width, height);
            present(width, height);
        }
        
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        u64 frame_end_usec = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
        u64 frametime_usec = frame_end_usec - frame_start_usec;
        
        if (frametime_usec < 16666) {
            usleep(16666 - frametime_usec);
        }
    }
    
    return(0);
}
