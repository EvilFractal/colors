#include<gtk/gtk.h>
#include<cairo.h>
#include<iostream>
#include<math.h>
#include "colorspaces.h"
#include "geometry.h"

// static GdkSurface* surface = NULL;

// struct ColorTile {
//     GtkWidget* frame;
//     GtkWidget* tile;
//     GdkRGBA* color;
// };

GdkRGBA* CURRENT_COLOR;

static cairo_surface_t* surface=NULL;

float drag_dot_scale=1.0;
float drag_bar_scale=1.0;

class ColorTile;
void update(GtkWidget* widget, gpointer data);

enum {    ACTION_SIGNAL,    LAST_SIGNAL};
static guint my_widget_signals[LAST_SIGNAL] = { 0 };
// std::vector<gpointer> rid(100);

GdkRGBA _rgb_to_gdk_rgba(ColorSpaces::RGB* color) {
    GdkRGBA result; // = g_new(GdkRGBA, 1);
    result.red=color->r;
    result.green=color->g;
    result.blue=color->b;
    result.alpha=color->a;
    return result;
}

ColorSpaces::RGB _gdk_rgba_to_rgb(GdkRGBA* color) {
    ColorSpaces::RGB result;// = g_new(ColorSpaces::RGB, 1);
    result.r=color->red;
    result.g=color->green;
    result.b=color->blue;
    result.a=color->alpha;
    return result;
}

void niffie(std::string message) {
    std::cout<<message<<'\n';
    return;
}

void clear_surface(GdkRGBA* color) {
    cairo_t* cr;
    cr=cairo_create(surface);
    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_paint(cr);
    cairo_destroy(cr);
}

/* Create a new surface of the appropriate size to store our scribbles */
void resize_cb(GtkWidget* widget, int width, int height, gpointer   data) {
    if (surface) {
        cairo_surface_destroy(surface);
        // surface=NULL;
    }
    if (gtk_native_get_surface(gtk_widget_get_native(widget))) {
        GdkRGBA* color=g_new(GdkRGBA, 1);
        *color=*CURRENT_COLOR;
        surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, gtk_widget_get_width(widget), gtk_widget_get_height(widget));
        std::cout<<gtk_widget_get_width(widget)<<' '<<gtk_widget_get_height(widget)<<'\n';
        /* Initialize the surface to color */
        clear_surface(color);
        g_free(color);
    }
}

void paint_tile(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
    cairo_set_source_surface(cr, surface, 0, 0);
    GdkRGBA* color=((GdkRGBA*)data);
    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_paint(cr);
}

static void on_closure_notify(gpointer data, GClosure *closure){
    // g_free(data);
}
static void on_destroy_notify(gpointer data){
    g_free(data);
}

class ColorTile {
public:
    GtkWidget* frame;
    GtkWidget* tile;
    GdkRGBA* color;

    static ColorTile * ColorTilenew(GtkWidget* parent, GdkRGBA* def_color,
        int width=50, int height=50,
        int grid_column=0, int grid_row=0, int grid_vert_span=1, int grid_hz_span=1) {
        ColorTile * tile = g_new(ColorTile, 1);
        tile->color= CURRENT_COLOR;
        tile->frame=gtk_frame_new(NULL);
        tile->tile=gtk_drawing_area_new();
        gtk_widget_set_size_request(GTK_WIDGET(tile->tile), width, height);
        gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(tile->tile), width);
        gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(tile->tile), height);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(tile->tile), paint_tile, CURRENT_COLOR, (GDestroyNotify)on_destroy_notify);
        g_signal_connect_after(GTK_DRAWING_AREA(tile->tile), "resize", G_CALLBACK(resize_cb), NULL);
        g_signal_connect_data(GTK_GRID(parent), "state-flags-changed", G_CALLBACK(update), tile->tile, on_closure_notify, G_CONNECT_SWAPPED);
        gtk_frame_set_child(GTK_FRAME(tile->frame), tile->tile);
        if (GTK_IS_GRID(parent)) {
            gtk_grid_attach(GTK_GRID(parent), tile->frame, grid_column, grid_row, grid_hz_span, grid_vert_span);
        } else if (GTK_IS_FRAME(parent)) {
            gtk_frame_set_child(GTK_FRAME(parent), tile->frame);
        } else if (GTK_IS_WINDOW(parent)) {
            gtk_window_set_child(GTK_WINDOW(parent), tile->frame);
        } else {
            niffie("ERROR - could not attach the tile: unsupported parent type");
        }
        return tile;
    }

    
};

void update(GtkWidget* widget, gpointer data){
        // niffie("works");
        gtk_widget_queue_draw(widget);
    }

void free_tile(ColorTile* obj) {
    g_free(obj);
}
float current_hue=0.0;
// bool hsl_dragged_farright=false;

// void paint_hsl_area(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
//     niffie("kk");
//     GdkRGBA* color=g_new(GdkRGBA, 1);
//     *color=*CURRENT_COLOR;
//     cairo_set_source_surface(cr, surface, 0, 0);
//     cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1);
//     cairo_paint(cr);
//     // gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
//     niffie(std::to_string(cairo_get_reference_count(cr)));
//     cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
//     cairo_rectangle(cr, 30, 30, 50, 50);
//     cairo_stroke(cr);
//     std::cout<<"paint ";
//     int stride, width_, height_;
//     width_=gtk_widget_get_width(GTK_WIDGET(drawing_area))*9/10;
//     height_=gtk_widget_get_height(GTK_WIDGET(drawing_area))/2;
//     stride=cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width_);
//     unsigned int* buffer=g_new(unsigned int, (height_*stride)/4);
//     unsigned int* pixel;
//     int hue, sat, light;
//     std::cout<<"painter ";
//     niffie("ooop");
//     ColorSpaces::HSL* hsl_color=g_new(ColorSpaces::HSL, 1);
//     ColorSpaces::RGB t=(_gdk_rgba_to_rgb(color));
//     *hsl_color=Converter::rgb_to_hsl(&t);
//     ColorSpaces::HSL* hslpix=g_new(ColorSpaces::HSL, 1);
//     if (hsl_color->s == 0.0 or hsl_dragged_farright) {
//         hsl_color->h=current_hue;
//     }
//     hue=hsl_color->h;
//     hslpix->l=std::max(hsl_color->l, 0.5f);
//     hslpix->a=1.0;
//     ColorSpaces::RGB8 pixcolor;
//     //filling the buffer for rainbow rectangle
//     for (int j=0;j<height_;j++) {
//         pixel=buffer + (j*(stride/4));
//         for (int i=0;i<width_;i++) {
//             hslpix->h=((float)i)/((float)width_-1);
//             hslpix->s=((float)j)/((float)height_-1);
//             t=Converter::hsl_to_rgb(hslpix);
//             pixcolor=Converter::float_to_int_rgb(&t);
//             *pixel=(((unsigned int)(pixcolor.a *255) << 24) + ((unsigned)pixcolor.r << 16)
//                 + ((unsigned)pixcolor.g << 8) + ((unsigned)pixcolor.b));
//             if (i<width_-1) *pixel++;
//         }
//     }
//     cairo_surface_t* sat_light_rect;
//     sat_light_rect=cairo_image_surface_create_for_data((unsigned char*)buffer, CAIRO_FORMAT_ARGB32,
//         width_, height_, stride);
//     // cairo_t* sat_light_cr = cairo_create(sat_light_rect);
//     cairo_save(cr);
//     cairo_set_source_surface(cr, sat_light_rect, width/20, height/10);
//     cairo_paint(cr);
//     cairo_restore(cr);
//     if (sat_light_rect) cairo_surface_destroy(sat_light_rect);
//     g_free(buffer);
//     //we need a lightness slider
//     cairo_set_source_surface(cr, surface, 0, 0);
//     hslpix->h=hsl_color->h;
//     hslpix->s=hsl_color->s;
//     height_/=7;
//     buffer=g_new(unsigned int, height_*stride/4);
//     stride=cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width_);
//     std::cout<<"paint ";
//     for (int j=0;j<height_; j++) {
//         pixel=buffer + (j*(stride/4));
//         for (int i=0;i<width_;i++) {
//             hslpix->l=((float)i)/((float)width_-1);
//             ColorSpaces::RGB t=Converter::hsl_to_rgb(hslpix);
//             pixcolor=Converter::float_to_int_rgb(&t);
//             *pixel=(((unsigned int)(pixcolor.a *255) << 24) + ((unsigned)pixcolor.r << 16)
//                 + ((unsigned)pixcolor.g << 8) + ((unsigned)pixcolor.b));
//             if (i<width_-1) *pixel++;
//         }
//     }
//     sat_light_rect=cairo_image_surface_create_for_data((unsigned char*)buffer, CAIRO_FORMAT_ARGB32,
//         width_, height_, stride);
//     // cairo_t* sat_light_cr = cairo_create(sat_light_rect);
//     cairo_save(cr);
//     cairo_set_source_surface(cr, sat_light_rect, width/20, height*7/10);
//     cairo_paint(cr);
//     cairo_restore(cr);
//     if (sat_light_rect) cairo_surface_destroy(sat_light_rect);
//     GdkRGBA* outerborder=g_new(GdkRGBA, 1);
//     GdkRGBA* innerborder=g_new(GdkRGBA, 1);
//     outerborder->red=outerborder->green=outerborder->blue=outerborder->alpha=1;
//     innerborder->red=innerborder->green=innerborder->blue=0;
//     innerborder->alpha=1;
//     if (hsl_color->l < 0.5) {
//         std::swap(innerborder, outerborder);
//     }
//     //painting color dot
//     int dot_radius=10*drag_dot_scale, dot_x=hsl_color->h*width_+width/20, dot_y=hsl_color->s*height_*7+height/10;
//     niffie("dot-paint"+std::to_string(dot_x)+' '+std::to_string(dot_y)+" hsl hue:"+std::to_string(hsl_color->h));
//     cairo_set_source_surface(cr, surface, width/20, height/10);
//     cairo_set_source_rgba(cr, outerborder->red, outerborder->green, outerborder->blue, outerborder->alpha);
//     cairo_arc(cr, dot_x, dot_y, dot_radius, 0, 2*M_PI);
//     cairo_stroke(cr);
//     cairo_set_source_rgba(cr, innerborder->red, innerborder->green, innerborder->blue, innerborder->alpha);
//     cairo_arc(cr, dot_x, dot_y, dot_radius-1, 0, 2*M_PI);
//     cairo_stroke(cr);
//     cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
//     cairo_arc(cr, dot_x, dot_y, dot_radius-2, 0, 2*M_PI);
//     cairo_fill(cr);
//     cairo_stroke(cr);
//     //painting color bar on lightness slider
//     float bar_x=hsl_color->l*width_ + width/20, bar_y=height*7/10, bar_width=10*drag_bar_scale;
//     cairo_set_source_rgba(cr, outerborder->red, outerborder->green, outerborder->blue, outerborder->alpha);
//     cairo_rectangle(cr, bar_x-bar_width/2, bar_y, bar_width, height_);
//     cairo_stroke(cr);
//     width--;
//     cairo_set_source_rgba(cr, innerborder->red, innerborder->green, innerborder->blue, innerborder->alpha);
//     cairo_rectangle(cr, bar_x- (float)bar_width /2.0, bar_y, bar_width, height_);
//     cairo_stroke(cr);
//     width--;
//     cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
//     cairo_rectangle(cr, bar_x- (float)bar_width /2.0, bar_y, bar_width, height_);
//     cairo_fill(cr);
//     cairo_stroke(cr);
//     g_free(buffer);
//     g_free(hslpix);
//     g_free(color);
//     g_free(hsl_color);
//     g_free(innerborder);
//     g_free(outerborder);
//     std::cout<<"paint-eeeeeeeeeeeeeee";
// }

// ColorTile _initColorTile(GtkWidget* grid) {
//     ColorTile tile; //=*g_new(ColorTile, 1);
//     std::cout<<"mid2 ";
//     tile.frame=gtk_frame_new(NULL);
//     std::cout<<"mid2 ";
//     tile.tile=gtk_drawing_area_new();
//     gtk_widget_set_size_request(GTK_WIDGET(tile.tile), 50, 50);
//     std::cout<<"mid2 ";
//     gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(tile.tile), 40);
//     gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(tile.tile), 40);
//     std::cout<<"mid2 ";
//     gtk_frame_set_child(GTK_FRAME(tile.frame), tile.tile);
//     tile.color=CURRENT_COLOR;
//     gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(tile.tile), paint_tile, CURRENT_COLOR, (GDestroyNotify)g_free);
//     g_signal_connect_after(GTK_DRAWING_AREA(tile.tile), "resize", G_CALLBACK(resize_cb), NULL);
//     std::cout<<"mid2 ";
//     return tile;
// }

static void close_window(gpointer window) {
    if (surface) {
        // cairo_surface_destroy(surface);
    }
}

float startx, starty;

// void hsl_drag_begin(GtkGestureDrag* gesture, float x, float y, GtkWidget* area) {
//     int width=gtk_widget_get_width(area);
//     int height=gtk_widget_get_height(area);
//     double dx, dy;
//     bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
//     x=dx; y=dy;
//     niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
//     if (width/20 <= x and width*19/20 >= x) {
//         ColorSpaces::HSL* current_hsl=g_new(ColorSpaces::HSL, 1);
//         ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
//         *current_hsl=Converter::rgb_to_hsl(&t);
//         if (height/10 <= y and height*6/10 >= y) {
//             drag_dot_scale=2.0;
//             startx=x;
//             starty=y;
//             current_hsl->h=(x - width/20)*10 / (width*9);
//             current_hue=current_hsl->h;
//             current_hsl->s=(y - height/10)*2 / height;
//             t=Converter::hsl_to_rgb(current_hsl);
//             *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
//             niffie("drag-start"+std::to_string(startx)+' '+std::to_string(starty));
//         } else if (height*7/10 <= y and height*8/10 >= y) {
//             drag_bar_scale=1.5;
//             startx=x;
//             starty=y;
//             current_hsl->l=(x - width/20)*10 / (width*9);
//             t=Converter::hsl_to_rgb(current_hsl);
//             *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
//             niffie("drag-start"+std::to_string(startx)+' '+std::to_string(starty));
//         }
//         g_free(current_hsl);
//     }
//     gtk_widget_queue_draw(area);
// }

// void hsl_drag_end(GtkGestureDrag* gesture, float x, float y, GtkWidget* area) {
//     int width=gtk_widget_get_width(area);
//     int height=gtk_widget_get_height(area);
//     drag_dot_scale=1.0;
//     drag_bar_scale=1.0;
//     double dx, dy;
//     bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
//     x=dx; y=dy;
//     gtk_widget_queue_draw(area);
// }

// void hsl_drag_update(GtkGestureDrag* gesture, float x, float y, GtkWidget* area) {
//     float width=gtk_widget_get_width(area);
//     float height=gtk_widget_get_height(area);
//     double dx, dy;
//     bool sth=gtk_gesture_drag_get_offset(gesture, &dx, &dy);
//     niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
//     x=dx; y=dy;
//     x+=startx;
//     y+=starty;
//     ColorSpaces::HSL* current_hsl=g_new(ColorSpaces::HSL, 1);
//     ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
//     *current_hsl=Converter::rgb_to_hsl(&t);
//     if (drag_dot_scale>1.0) {
//         float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
//         float bound_y=std::min(height*6/10.0f, std::max(height/10.0f, y));
//         // startx=bound_x;
//         // starty=bound_y;
//         current_hsl->h=(bound_x - width/20)*10 / (width*9);
//         current_hue=current_hsl->h;
//         if (x > width*19/20.0f) {
//             current_hue=1.0;
//             hsl_dragged_farright=true;
//         } else {
//             hsl_dragged_farright=false;
//         }
//         current_hsl->s=(bound_y - height/10)*2 / height;
//         niffie("drag-upd"+std::to_string(bound_x)+' '+std::to_string(bound_y));
//         t=Converter::hsl_to_rgb(current_hsl);
//         *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
//     } else if (drag_bar_scale>1.0) {
//         float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
//         y=height*7.5/10;
//         // startx=bound_x;
//         // starty=y;
//         current_hsl->l=(bound_x - width/20)*10 / (width*9);
//         t=Converter::hsl_to_rgb(current_hsl);
//         *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
//     }
//     g_free(current_hsl);
//     niffie("trying to draw");
//     // g_signal_emit_by_name(GTK_WIDGET(area), "draw");
//     gtk_widget_queue_draw(area);
//     // resize_cb(GTK_WIDGET(area), width, height, NULL);
// }
// static void finalize(GObject *obj);
// static void drag_begin(GtkGestureDrag *gesture, float x, float y, GtkWidget *area);
// static void drag_update(GtkGestureDrag* gesture, float x, float y, GtkWidget* area);
// static void drag_end(GtkGestureDrag *gesture, float x, float y, GtkWidget *area);
// static void draw(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data);

class ColorChooserTab {
public:
    GtkWidget* frame; //contains all insides, especially content
    GtkWidget* tab; //the notebook tab that holds the label
    GtkWidget* content; //DrawingArea to render the chooser
    int page_num;
};

struct HSLprops{
    float startx, starty;
    bool hsl_dragged_farright;
    float drag_dot_scale;
    float drag_bar_scale;
};
HSLprops hslprops;

class HSLTab :ColorChooserTab {

public:
    GtkGesture* drag;
    GtkWidget* frame; //contains all insides, especially content
    GtkWidget* tab; //the notebook tab that holds the label
    GtkWidget* content; //DrawingArea to render the chooser
    int page_num;

    static HSLTab* HSLTabnew(GtkNotebook* notebook, const char* tab_name,
        int width=400, int height=400) {
        HSLTab* hsltab = g_new(HSLTab, 1);
        hsltab->frame=gtk_frame_new(NULL);
        hsltab->tab=gtk_label_new(tab_name);
        hsltab->content=gtk_drawing_area_new();
        gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hsltab->content), width);
        gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hsltab->content), height);
        gtk_frame_set_child(GTK_FRAME(hsltab->frame), hsltab->content);
        GtkWidgetClass* wclass=GTK_WIDGET_CLASS(hsltab->content);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hsltab->content), draw, CURRENT_COLOR, (GDestroyNotify)on_destroy_notify);
        g_signal_connect_after(GTK_DRAWING_AREA(hsltab->content), "resize", G_CALLBACK(resize_cb), NULL);
        hsltab->page_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hsltab->frame, hsltab->tab);
        hsltab->drag=gtk_gesture_drag_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(hsltab->drag), GDK_BUTTON_PRIMARY);
        gtk_widget_add_controller(hsltab->content, GTK_EVENT_CONTROLLER(hsltab->drag));
        g_signal_connect(hsltab->drag, "drag-begin", G_CALLBACK(drag_begin), hsltab->content);
        g_signal_connect(hsltab->drag, "drag-update", G_CALLBACK(drag_update), hsltab->content);
        g_signal_connect(hsltab->drag, "drag-end", G_CALLBACK(drag_end), hsltab->content);
        hslprops.starty = 0;
        hslprops.startx = 0;
        hslprops.drag_bar_scale = 1.0;
        hslprops.drag_dot_scale = 1.0;
        hslprops.hsl_dragged_farright = false;
        return hsltab;
    }

    static void drag_begin(GtkGestureDrag* gesture, float x, float y, GtkWidget* area) {
        int width=gtk_widget_get_width(area);
        int height=gtk_widget_get_height(area);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
        if (width/20 <= x and width*19/20 >= x) {
            ColorSpaces::HSL* current_hsl=g_new(ColorSpaces::HSL, 1);
            ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
            *current_hsl=Converter::rgb_to_hsl(&t);
            if (height/10 <= y and height*6/10 >= y) {
                hslprops.drag_dot_scale=2.0;
                hslprops.startx=x;
                hslprops.starty=y;
                current_hsl->h=(x - width/20)*10 / (width*9);
                current_hue=current_hsl->h;
                current_hsl->s=(y - height/10)*2 / height;
                t=Converter::hsl_to_rgb(current_hsl);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                niffie("drag-start"+std::to_string(hslprops.startx)+' '+std::to_string(hslprops.starty));
            } else if (height*7/10 <= y and height*8/10 >= y) {
                hslprops.drag_bar_scale=1.5;
                hslprops.startx=x;
                hslprops.starty=y;
                current_hsl->l=(x - width/20)*10 / (width*9);
                t=Converter::hsl_to_rgb(current_hsl);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                niffie("drag-start"+std::to_string(hslprops.startx)+' '+std::to_string(hslprops.starty));
            }
            g_free(current_hsl);
        }

        gtk_widget_queue_draw(area);
    }

    static void drag_update(GtkGestureDrag* gesture, float x, float y, GtkWidget* area) {
        float width=gtk_widget_get_width(area);
        float height=gtk_widget_get_height(area);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_offset(gesture, &dx, &dy);
        niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
        x=dx; y=dy;
        x+=hslprops.startx;
        y+=hslprops.starty;
        ColorSpaces::HSL* current_hsl=g_new(ColorSpaces::HSL, 1);
        ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
        *current_hsl=Converter::rgb_to_hsl(&t);
        if (hslprops.drag_dot_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            float bound_y=std::min(height*6/10.0f, std::max(height/10.0f, y));
            // hslprops.startx=bound_x;
            // hslprops.starty=bound_y;
            current_hsl->h=(bound_x - width/20)*10 / (width*9);
            current_hue=current_hsl->h;
            if (x > width*19/20.0f) {
                current_hue=1.0;
                hslprops.hsl_dragged_farright=true;
            } else {
                hslprops.hsl_dragged_farright=false;
            }
            current_hsl->s=(bound_y - height/10)*2 / height;

            niffie("drag-upd"+std::to_string(bound_x)+' '+std::to_string(bound_y));
            t=Converter::hsl_to_rgb(current_hsl);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
        } else if (hslprops.drag_bar_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            y=height*7.5/10;
            // hslprops.startx=bound_x;
            // hslprops.starty=y;
            current_hsl->l=(bound_x - width/20)*10 / (width*9);
            t=Converter::hsl_to_rgb(current_hsl);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
        }
        g_free(current_hsl);
        niffie("trying to draw");
        // g_signal_emit_by_name(GTK_WIDGET(area), "draw");
        gtk_widget_queue_draw(area);
        // resize_cb(GTK_WIDGET(area), width, height, NULL);
    }

    static void drag_end(GtkGestureDrag* gesture, float x, float y, GtkWidget* area) {
        int width=gtk_widget_get_width(area);
        int height=gtk_widget_get_height(area);
        hslprops.drag_dot_scale=1.0;
        hslprops.drag_bar_scale=1.0;
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        gtk_widget_queue_draw(area);
    }

    static void draw(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
        niffie("kk");
        g_signal_emit_by_name(drawing_area, "color-change");
        GdkRGBA* color=g_new(GdkRGBA, 1);
        *color=*CURRENT_COLOR;
        cairo_set_source_surface(cr, surface, 0, 0);
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1);
        cairo_paint(cr);

        // gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
        niffie(std::to_string(cairo_get_reference_count(cr)));
        cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
        cairo_rectangle(cr, 30, 30, 50, 50);
        cairo_stroke(cr);
        std::cout<<"paint ";
        int stride, width_, height_;
        width_=gtk_widget_get_width(GTK_WIDGET(drawing_area))*9/10;
        height_=gtk_widget_get_height(GTK_WIDGET(drawing_area))/2;
        stride=cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width_);
        unsigned int* buffer=g_new(unsigned int, (height_*stride)/4);
        unsigned int* pixel;
        int hue, sat, light;
        std::cout<<"painter ";
        niffie("ooop");
        ColorSpaces::HSL* hsl_color=g_new(ColorSpaces::HSL, 1);
        ColorSpaces::RGB t=(_gdk_rgba_to_rgb(color));
        *hsl_color=Converter::rgb_to_hsl(&t);
        ColorSpaces::HSL* hslpix=g_new(ColorSpaces::HSL, 1);
        if (hsl_color->s == 0.0 or hslprops.hsl_dragged_farright) {
            hsl_color->h=current_hue;
        }
        hue=hsl_color->h;
        hslpix->l=std::max(hsl_color->l, 0.5f);
        hslpix->a=1.0;
        ColorSpaces::RGB8 pixcolor;
        //filling the buffer for rainbow rectangle
        for (int j=0;j<height_;j++) {
            pixel=buffer + (j*(stride/4));
            for (int i=0;i<width_;i++) {
                hslpix->h=((float)i)/((float)width_-1);
                hslpix->s=((float)j)/((float)height_-1);
                t=Converter::hsl_to_rgb(hslpix);
                pixcolor=Converter::float_to_int_rgb(&t);
                *pixel=(((unsigned int)(pixcolor.a *255) << 24) + ((unsigned)pixcolor.r << 16)
                    + ((unsigned)pixcolor.g << 8) + ((unsigned)pixcolor.b));
                if (i<width_-1) *pixel++;
            }
        }

        cairo_surface_t* sat_light_rect;
        sat_light_rect=cairo_image_surface_create_for_data((unsigned char*)buffer, CAIRO_FORMAT_ARGB32,
            width_, height_, stride);
        // cairo_t* sat_light_cr = cairo_create(sat_light_rect);
        cairo_save(cr);
        cairo_set_source_surface(cr, sat_light_rect, width/20, height/10);
        cairo_paint(cr);
        cairo_restore(cr);
        if (sat_light_rect) cairo_surface_destroy(sat_light_rect);

        g_free(buffer);

        //we need a lightness slider
        cairo_set_source_surface(cr, surface, 0, 0);
        hslpix->h=hsl_color->h;
        hslpix->s=hsl_color->s;
        height_/=7;
        buffer=g_new(unsigned int, height_*stride/4);
        stride=cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width_);

        std::cout<<"paint ";
        for (int j=0;j<height_; j++) {
            pixel=buffer + (j*(stride/4));
            for (int i=0;i<width_;i++) {
                hslpix->l=((float)i)/((float)width_-1);
                ColorSpaces::RGB t=Converter::hsl_to_rgb(hslpix);
                pixcolor=Converter::float_to_int_rgb(&t);
                *pixel=(((unsigned int)(pixcolor.a *255) << 24) + ((unsigned)pixcolor.r << 16)
                    + ((unsigned)pixcolor.g << 8) + ((unsigned)pixcolor.b));
                if (i<width_-1) *pixel++;
            }
        }

        sat_light_rect=cairo_image_surface_create_for_data((unsigned char*)buffer, CAIRO_FORMAT_ARGB32,
            width_, height_, stride);
        // cairo_t* sat_light_cr = cairo_create(sat_light_rect);
        cairo_save(cr);
        cairo_set_source_surface(cr, sat_light_rect, width/20, height*7/10);
        cairo_paint(cr);
        cairo_restore(cr);
        if (sat_light_rect) cairo_surface_destroy(sat_light_rect);

        GdkRGBA* outerborder=g_new(GdkRGBA, 1);
        GdkRGBA* innerborder=g_new(GdkRGBA, 1);
        outerborder->red=outerborder->green=outerborder->blue=outerborder->alpha=1;
        innerborder->red=innerborder->green=innerborder->blue=0;
        innerborder->alpha=1;

        if (hsl_color->l < 0.5) {
            std::swap(innerborder, outerborder);
        }

        //painting color dot
        int dot_radius=10*hslprops.drag_dot_scale, dot_x=hsl_color->h*width_+width/20, dot_y=hsl_color->s*height_*7+height/10;

        niffie("dot-paint"+std::to_string(dot_x)+' '+std::to_string(dot_y)+" hsl hue:"+std::to_string(hsl_color->h));
        cairo_set_source_surface(cr, surface, width/20, height/10);
        cairo_set_source_rgba(cr, outerborder->red, outerborder->green, outerborder->blue, outerborder->alpha);
        cairo_arc(cr, dot_x, dot_y, dot_radius, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_set_source_rgba(cr, innerborder->red, innerborder->green, innerborder->blue, innerborder->alpha);
        cairo_arc(cr, dot_x, dot_y, dot_radius-1, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
        cairo_arc(cr, dot_x, dot_y, dot_radius-2, 0, 2*M_PI);
        cairo_fill(cr);
        cairo_stroke(cr);

        //painting color bar on lightness slider
        float bar_x=hsl_color->l*width_ + width/20, bar_y=height*7/10, bar_width=10*hslprops.drag_bar_scale;
        cairo_set_source_rgba(cr, outerborder->red, outerborder->green, outerborder->blue, outerborder->alpha);
        cairo_rectangle(cr, bar_x-bar_width/2, bar_y, bar_width, height_);
        cairo_stroke(cr);
        width--;
        cairo_set_source_rgba(cr, innerborder->red, innerborder->green, innerborder->blue, innerborder->alpha);
        cairo_rectangle(cr, bar_x- (float)bar_width /2.0, bar_y, bar_width, height_);
        cairo_stroke(cr);
        width--;
        cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
        cairo_rectangle(cr, bar_x- (float)bar_width /2.0, bar_y, bar_width, height_);
        cairo_fill(cr);
        cairo_stroke(cr);

        g_free(buffer);
        g_free(hslpix);
        g_free(color);
        g_free(hsl_color);
        g_free(innerborder);
        g_free(outerborder);
        std::cout<<"paint-eeeeeeeeeeeeeee";
    }
};

struct HSVprops{
    float startx, starty;
    bool hsv_dragged_farright;
    float drag_dot_scale;
    float drag_bar_scale;
};
HSVprops hsvprops;

class HSVTab :ColorChooserTab {

public:
    GtkGesture* drag;
    GtkWidget* frame; //contains all insides, especially content
    GtkWidget* tab; //the notebook tab that holds the label
    GtkWidget* content; //DrawingArea to render the chooser
    int page_num;

    static HSVTab* HSVTabnew(GtkNotebook* notebook, const char* tab_name,
                            int width=400, int height=400) {
        HSVTab* hsvtab = g_new(HSVTab, 1);
        hsvtab->frame=gtk_frame_new(NULL);
        hsvtab->tab=gtk_label_new(tab_name);
        hsvtab->content=gtk_drawing_area_new();
        gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hsvtab->content), width);
        gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hsvtab->content), height);
        gtk_frame_set_child(GTK_FRAME(hsvtab->frame), hsvtab->content);
        GtkWidgetClass* wclass=GTK_WIDGET_CLASS(hsvtab->content);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hsvtab->content), draw, CURRENT_COLOR, (GDestroyNotify)on_destroy_notify);
        g_signal_connect_after(GTK_DRAWING_AREA(hsvtab->content), "resize", G_CALLBACK(resize_cb), NULL);
        hsvtab->page_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hsvtab->frame, hsvtab->tab);
        hsvtab->drag=gtk_gesture_drag_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(hsvtab->drag), GDK_BUTTON_PRIMARY);
        gtk_widget_add_controller(hsvtab->content, GTK_EVENT_CONTROLLER(hsvtab->drag));
        g_signal_connect(hsvtab->drag, "drag-begin", G_CALLBACK(drag_begin), hsvtab->content);
        g_signal_connect(hsvtab->drag, "drag-update", G_CALLBACK(drag_update), hsvtab->content);
        g_signal_connect(hsvtab->drag, "drag-end", G_CALLBACK(drag_end), hsvtab->content);
        hsvprops.starty = 0;
        hsvprops.startx = 0;
        hsvprops.drag_bar_scale = 1.0;
        hsvprops.drag_dot_scale = 1.0;
        hsvprops.hsv_dragged_farright = false;
        return hsvtab;
    }

    static void drag_begin(GtkGestureDrag* gesture, float x, float y, GtkWidget* area) {
        int width=gtk_widget_get_width(area);
        int height=gtk_widget_get_height(area);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
        if (width/20 <= x and width*19/20 >= x) {
            ColorSpaces::HSV* current_hsv=g_new(ColorSpaces::HSV, 1);
            ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
            *current_hsv=Converter::rgb_to_hsv(&t);
            if (height/10 <= y and height*6/10 >= y) {
                hsvprops.drag_dot_scale=2.0;
                hsvprops.startx=x;
                hsvprops.starty=y;
                current_hsv->h=(x - width/20)*10 / (width*9);
                current_hue=current_hsv->h;
                current_hsv->s=(y - height/10)*2 / height;
                t=Converter::hsv_to_rgb(current_hsv);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                niffie("drag-start"+std::to_string(hsvprops.startx)+' '+std::to_string(hsvprops.starty));
            } else if (height*7/10 <= y and height*8/10 >= y) {
                hsvprops.drag_bar_scale=1.5;
                hsvprops.startx=x;
                hsvprops.starty=y;
                current_hsv->v=(x - width/20)*10 / (width*9);
                t=Converter::hsv_to_rgb(current_hsv);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                niffie("drag-start"+std::to_string(hsvprops.startx)+' '+std::to_string(hsvprops.starty));
            }
            g_free(current_hsv);
        }

        gtk_widget_queue_draw(area);
    }

    static void drag_update(GtkGestureDrag* gesture, float x, float y, GtkWidget* area) {
        float width=gtk_widget_get_width(area);
        float height=gtk_widget_get_height(area);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_offset(gesture, &dx, &dy);
        niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
        x=dx; y=dy;
        x+=hsvprops.startx;
        y+=hsvprops.starty;
        ColorSpaces::HSV* current_hsv=g_new(ColorSpaces::HSV, 1);
        ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
        *current_hsv=Converter::rgb_to_hsv(&t);
        if (hsvprops.drag_dot_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            float bound_y=std::min(height*6/10.0f, std::max(height/10.0f, y));
            // hsvprops.startx=bound_x;
            // hsvprops.starty=bound_y;
            current_hsv->h=(bound_x - width/20)*10 / (width*9);
            current_hue=current_hsv->h;
            if (x > width*19/20.0f) {
                current_hue=1.0;
                hsvprops.hsv_dragged_farright=true;
            } else {
                hsvprops.hsv_dragged_farright=false;
            }
            current_hsv->s=(bound_y - height/10)*2 / height;

            niffie("drag-upd"+std::to_string(bound_x)+' '+std::to_string(bound_y));
            t=Converter::hsv_to_rgb(current_hsv);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
        } else if (hsvprops.drag_bar_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            y=height*7.5/10;
            // hsvprops.startx=bound_x;
            // hsvprops.starty=y;
            current_hsv->v=(bound_x - width/20)*10 / (width*9);
            t=Converter::hsv_to_rgb(current_hsv);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
        }
        g_free(current_hsv);
        niffie("trying to draw");
        // g_signal_emit_by_name(GTK_WIDGET(area), "draw");
        gtk_widget_queue_draw(area);
        // resize_cb(GTK_WIDGET(area), width, height, NULL);
    }

    static void drag_end(GtkGestureDrag* gesture, float x, float y, GtkWidget* area) {
        int width=gtk_widget_get_width(area);
        int height=gtk_widget_get_height(area);
        hsvprops.drag_dot_scale=1.0;
        hsvprops.drag_bar_scale=1.0;
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        gtk_widget_queue_draw(area);
    }

    static void draw(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
        niffie("kk");
        g_signal_emit_by_name(drawing_area, "color-change");
        GdkRGBA* color=g_new(GdkRGBA, 1);
        *color=*CURRENT_COLOR;
        cairo_set_source_surface(cr, surface, 0, 0);
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1);
        cairo_paint(cr);

        // gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
        niffie(std::to_string(cairo_get_reference_count(cr)));
        cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
        cairo_rectangle(cr, 30, 30, 50, 50);
        cairo_stroke(cr);
        std::cout<<"paint ";
        int stride, width_, height_;
        width_=gtk_widget_get_width(GTK_WIDGET(drawing_area))*9/10;
        height_=gtk_widget_get_height(GTK_WIDGET(drawing_area))/2;
        stride=cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width_);
        unsigned int* buffer=g_new(unsigned int, (height_*stride)/4);
        unsigned int* pixel;
        int hue, sat, val;
        std::cout<<"painter ";
        niffie("ooop");
        ColorSpaces::HSV* hsv_color=g_new(ColorSpaces::HSV, 1);
        ColorSpaces::RGB t=(_gdk_rgba_to_rgb(color));
        *hsv_color=Converter::rgb_to_hsv(&t);
        ColorSpaces::HSV* hsvpix=g_new(ColorSpaces::HSV, 1);
        if (hsv_color->s == 0.0 or hsvprops.hsv_dragged_farright) {
            hsv_color->h=current_hue;
        }
        hue=hsv_color->h;
        hsvpix->v=std::max(hsv_color->v, 0.5f);
        hsvpix->a=1.0;
        ColorSpaces::RGB8 pixcolor;
        //filling the buffer for rainbow rectangle
        for (int j=0;j<height_;j++) {
            pixel=buffer + (j*(stride/4));
            for (int i=0;i<width_;i++) {
                hsvpix->h=((float)i)/((float)width_-1);
                hsvpix->s=((float)j)/((float)height_-1);
                t=Converter::hsv_to_rgb(hsvpix);
                pixcolor=Converter::float_to_int_rgb(&t);
                *pixel=(((unsigned int)(pixcolor.a *255) << 24) + ((unsigned)pixcolor.r << 16)
                    + ((unsigned)pixcolor.g << 8) + ((unsigned)pixcolor.b));
                if (i<width_-1) *pixel++;
            }
        }

        cairo_surface_t* sat_val_rect;
        sat_val_rect=cairo_image_surface_create_for_data((unsigned char*)buffer, CAIRO_FORMAT_ARGB32,
            width_, height_, stride);
        // cairo_t* sat_val_cr = cairo_create(sat_val_rect);
        cairo_save(cr);
        cairo_set_source_surface(cr, sat_val_rect, width/20, height/10);
        cairo_paint(cr);
        cairo_restore(cr);
        if (sat_val_rect) cairo_surface_destroy(sat_val_rect);

        g_free(buffer);

        //we need a value slider
        cairo_set_source_surface(cr, surface, 0, 0);
        hsvpix->h=hsv_color->h;
        hsvpix->s=hsv_color->s;
        height_/=7;
        buffer=g_new(unsigned int, height_*stride/4);
        stride=cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width_);

        std::cout<<"paint ";
        for (int j=0;j<height_; j++) {
            pixel=buffer + (j*(stride/4));
            for (int i=0;i<width_;i++) {
                hsvpix->v=((float)i)/((float)width_-1);
                ColorSpaces::RGB t=Converter::hsv_to_rgb(hsvpix);
                pixcolor=Converter::float_to_int_rgb(&t);
                *pixel=(((unsigned int)(pixcolor.a *255) << 24) + ((unsigned)pixcolor.r << 16)
                    + ((unsigned)pixcolor.g << 8) + ((unsigned)pixcolor.b));
                if (i<width_-1) *pixel++;
            }
        }

        sat_val_rect=cairo_image_surface_create_for_data((unsigned char*)buffer, CAIRO_FORMAT_ARGB32,
            width_, height_, stride);
        // cairo_t* sat_val_cr = cairo_create(sat_val_rect);
        cairo_save(cr);
        cairo_set_source_surface(cr, sat_val_rect, width/20, height*7/10);
        cairo_paint(cr);
        cairo_restore(cr);
        if (sat_val_rect) cairo_surface_destroy(sat_val_rect);

        GdkRGBA* outerborder=g_new(GdkRGBA, 1);
        GdkRGBA* innerborder=g_new(GdkRGBA, 1);
        outerborder->red=outerborder->green=outerborder->blue=outerborder->alpha=1;
        innerborder->red=innerborder->green=innerborder->blue=0;
        innerborder->alpha=1;

        if (hsv_color->v < 0.75) {
            std::swap(innerborder, outerborder);
        }

        //painting color dot
        int dot_radius=10*hsvprops.drag_dot_scale, dot_x=hsv_color->h*width_+width/20, dot_y=hsv_color->s*height_*7+height/10;

        niffie("dot-paint"+std::to_string(dot_x)+' '+std::to_string(dot_y)+" hsl hue:"+std::to_string(hsv_color->h));
        cairo_set_source_surface(cr, surface, width/20, height/10);
        cairo_set_source_rgba(cr, outerborder->red, outerborder->green, outerborder->blue, outerborder->alpha);
        cairo_arc(cr, dot_x, dot_y, dot_radius, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_set_source_rgba(cr, innerborder->red, innerborder->green, innerborder->blue, innerborder->alpha);
        cairo_arc(cr, dot_x, dot_y, dot_radius-1, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
        cairo_arc(cr, dot_x, dot_y, dot_radius-2, 0, 2*M_PI);
        cairo_fill(cr);
        cairo_stroke(cr);

        //painting color bar on value slider
        float bar_x=hsv_color->v*width_ + width/20, bar_y=height*7/10, bar_width=10*hsvprops.drag_bar_scale;
        cairo_set_source_rgba(cr, outerborder->red, outerborder->green, outerborder->blue, outerborder->alpha);
        cairo_rectangle(cr, bar_x-bar_width/2, bar_y, bar_width, height_);
        cairo_stroke(cr);
        width--;
        cairo_set_source_rgba(cr, innerborder->red, innerborder->green, innerborder->blue, innerborder->alpha);
        cairo_rectangle(cr, bar_x- (float)bar_width /2.0, bar_y, bar_width, height_);
        cairo_stroke(cr);
        width--;
        cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
        cairo_rectangle(cr, bar_x- (float)bar_width /2.0, bar_y, bar_width, height_);
        cairo_fill(cr);
        cairo_stroke(cr);

        g_free(buffer);
        g_free(hsvpix);
        g_free(color);
        g_free(hsv_color);
        g_free(innerborder);
        g_free(outerborder);
        std::cout<<"paint-eeeeeeeeeeeeeee";
    }
};

struct HWBprops{
    float startx, starty;
    bool hwb_dragged_outside;
    float drag_dot_scale;
    float drag_bar_scale;
    float ring_innerradius;
    float ring_outerradius;
    Geometry::Point2* CentrePoint;
    Geometry::Point2* VividPoint; 
    Geometry::Point2* BlackPoint;
    Geometry::Point2* WhitePoint;
    Geometry::LineGeneral2* NoIntensityLine;
    Geometry::LineGeneral2* NoBlackLine;
    Geometry::LineGeneral2* NoWhiteLine;
    float triangle_height;
};
HWBprops hwbprops;

class HWBTab :ColorChooserTab {
public: 
    GtkGesture* drag;
    GtkWidget* frame;
    GtkWidget* tab;
    GtkWidget* content;
    int page_num;

    static HWBTab* HWBTabnew(GtkNotebook* notebook, const char* tab_name,
                             int width=400, int height=400){
        HWBTab* hwbtab = g_new(HWBTab, 1);
        hwbtab->frame=gtk_frame_new(NULL);
        hwbtab->tab=gtk_label_new(tab_name);
        hwbtab->content=gtk_drawing_area_new();
        gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hwbtab->content), width);
        gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hwbtab->content), height);
        gtk_frame_set_child(GTK_FRAME(hwbtab->frame), hwbtab->content);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hwbtab->content), draw, NULL, (GDestroyNotify)on_destroy_notify);
        g_signal_connect_after(GTK_DRAWING_AREA(hwbtab->content), "resize", G_CALLBACK(resize_cb), NULL);
        hwbtab->page_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hwbtab->frame, hwbtab->tab);
        hwbtab->drag=gtk_gesture_drag_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(hwbtab->drag), GDK_BUTTON_PRIMARY);
        gtk_widget_add_controller(hwbtab->content, GTK_EVENT_CONTROLLER(hwbtab->drag));
        g_signal_connect(hwbtab->drag, "drag-begin", G_CALLBACK(drag_begin), hwbtab);
        g_signal_connect(hwbtab->drag, "drag-update", G_CALLBACK(drag_update), hwbtab);
        g_signal_connect(hwbtab->drag, "drag-end", G_CALLBACK(drag_end), hwbtab);
        hwbprops.starty = 0;
        hwbprops.startx = 0;
        hwbprops.drag_bar_scale = 1.0;
        hwbprops.drag_dot_scale = 1.0;
        hwbprops.hwb_dragged_outside = false;
        // *hwbprops.CentrePoint = {.x=float(width/2), .y=float(height/2)};
        hwbprops.CentrePoint = g_new(Geometry::Point2, 1);
        hwbprops.CentrePoint->x = float(width/2);
        (hwbprops.CentrePoint->y) = float(height/2);
        hwbprops.ring_outerradius = float(width/2 - 20);
        hwbprops.ring_innerradius = hwbprops.ring_outerradius - 20.0;
        hwbprops.triangle_height = (hwbprops.ring_innerradius - 15.0)*3.0 / 2.0;
        Geometry::Point2* vertice = g_new(Geometry::Point2, 1);
        *vertice = (Geometry::Point2){.x=0, .y=-(hwbprops.ring_innerradius - 15.0f)};
        hwbprops.VividPoint = vertice;
        hwbprops.WhitePoint = g_new(Geometry::Point2, 1);
        hwbprops.BlackPoint = g_new(Geometry::Point2, 1);
        *hwbprops.WhitePoint = GeoCalc_2d::rotate(vertice, 2.0*M_PI/3.0);
        *hwbprops.BlackPoint = GeoCalc_2d::rotate(vertice, 4.0*M_PI/3.0);
        hwbprops.NoIntensityLine = g_new(Geometry::LineGeneral2, 1);
        hwbprops.NoBlackLine = g_new(Geometry::LineGeneral2, 1);
        hwbprops.NoWhiteLine = g_new(Geometry::LineGeneral2, 1);
        *hwbprops.NoIntensityLine = {.A=0, .B=1, .C=(vertice->y)/2};
        *hwbprops.NoWhiteLine = {.A=std::sqrtf(3.0), .B=1, .C=-(vertice->y)};
        *hwbprops.NoBlackLine = {.A=std::sqrtf(3.0), .B=-1, .C=(vertice->y)};
        return hwbtab;
    }

    static void drag_begin(GtkGestureDrag* gesture, float x, float y, HWBTab* tab) {
        GtkWidget* area = tab->content;
        int width=gtk_widget_get_width(area);
        int height=gtk_widget_get_height(area);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
        if (width/20 <= x and width*19/20 >= x) {
            ColorSpaces::HSV* current_hsv=g_new(ColorSpaces::HSV, 1);
            ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
            *current_hsv=Converter::rgb_to_hsv(&t);
            if (height/10 <= y and height*6/10 >= y) {
                hsvprops.drag_dot_scale=2.0;
                hsvprops.startx=x;
                hsvprops.starty=y;
                current_hsv->h=(x - width/20)*10 / (width*9);
                current_hue=current_hsv->h;
                current_hsv->s=(y - height/10)*2 / height;
                t=Converter::hsv_to_rgb(current_hsv);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                niffie("drag-start"+std::to_string(hsvprops.startx)+' '+std::to_string(hsvprops.starty));
            } else if (height*7/10 <= y and height*8/10 >= y) {
                hsvprops.drag_bar_scale=1.5;
                hsvprops.startx=x;
                hsvprops.starty=y;
                current_hsv->v=(x - width/20)*10 / (width*9);
                t=Converter::hsv_to_rgb(current_hsv);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                niffie("drag-start"+std::to_string(hsvprops.startx)+' '+std::to_string(hsvprops.starty));
            }
            g_free(current_hsv);
        }

        gtk_widget_queue_draw(area);
    }

    static void drag_update(GtkGestureDrag* gesture, float x, float y, HWBTab* tab) {
        GtkWidget* area = tab->content;
        float width=gtk_widget_get_width(area);
        float height=gtk_widget_get_height(area);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_offset(gesture, &dx, &dy);
        niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
        x=dx; y=dy;
        x+=hsvprops.startx;
        y+=hsvprops.starty;
        ColorSpaces::HSV* current_hsv=g_new(ColorSpaces::HSV, 1);
        ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
        *current_hsv=Converter::rgb_to_hsv(&t);
        if (hsvprops.drag_dot_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            float bound_y=std::min(height*6/10.0f, std::max(height/10.0f, y));
            // hsvprops.startx=bound_x;
            // hsvprops.starty=bound_y;
            current_hsv->h=(bound_x - width/20)*10 / (width*9);
            current_hue=current_hsv->h;
            if (x > width*19/20.0f) {
                current_hue=1.0;
                hsvprops.hsv_dragged_farright=true;
            } else {
                hsvprops.hsv_dragged_farright=false;
            }
            current_hsv->s=(bound_y - height/10)*2 / height;

            niffie("drag-upd"+std::to_string(bound_x)+' '+std::to_string(bound_y));
            t=Converter::hsv_to_rgb(current_hsv);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
        } else if (hsvprops.drag_bar_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            y=height*7.5/10;
            // hsvprops.startx=bound_x;
            // hsvprops.starty=y;
            current_hsv->v=(bound_x - width/20)*10 / (width*9);
            t=Converter::hsv_to_rgb(current_hsv);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
        }
        g_free(current_hsv);
        niffie("trying to draw");
        // g_signal_emit_by_name(GTK_WIDGET(area), "draw");
        gtk_widget_queue_draw(area);
        // resize_cb(GTK_WIDGET(area), width, height, NULL);
    }

    static void drag_end(GtkGestureDrag* gesture, float x, float y, HWBTab* tab) {
        GtkWidget* area = tab->content;
        int width=gtk_widget_get_width(area);
        int height=gtk_widget_get_height(area);
        hsvprops.drag_dot_scale=1.0;
        hsvprops.drag_bar_scale=1.0;
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        gtk_widget_queue_draw(area);
    }

    static void draw(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
        niffie("kk");
        g_signal_emit_by_name(drawing_area, "color-change");
        GdkRGBA* color=g_new(GdkRGBA, 1);
        *color=*CURRENT_COLOR;
        cairo_set_source_surface(cr, surface, 0, 0);
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1);
        cairo_paint(cr);

        int stride, width_, height_;
        width_=gtk_widget_get_width(GTK_WIDGET(drawing_area));
        height_=gtk_widget_get_height(GTK_WIDGET(drawing_area));
        stride=cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width_);
        unsigned int* buffer=g_new(unsigned int, (height_*stride)/4);
        unsigned int* pixel;
        ColorSpaces::RGB8 bg_color = {.r=76, .g=76, .b=76, .a=1};
        float hue, white, black;
        ColorSpaces::HWB* hwb_color = g_new(ColorSpaces::HWB, 1);
        ColorSpaces::RGB t=_gdk_rgba_to_rgb(color);
        *hwb_color = Converter::rgb_to_hwb(&t);
        ColorSpaces::HWB* hwbpix = g_new(ColorSpaces::HWB, 1);
        hue = hwb_color->h;
        hwbpix->a = 1.0;
        Geometry::Point2* coords = g_new(Geometry::Point2, 1);
        Geometry::Polar2 polar;
        // unsigned int* buffer = g_new(unsigned int, (height_*stride)/4);
        // unsigned int* pixel;
        float hue_angle = hue * 2.0f * M_PI;
        niffie(std::to_string(hue_angle)+" ... "+std::to_string(hue));
        ColorSpaces::RGB8 pixcolor;
        //filling the buffer for the whole triangle picker base
        for(int j=0; j<height_;j++){
            pixel = buffer + (j*stride)/4;
            for(int i=0;i<width_;i++){
                coords->x = i; coords->y = j;
                coords->x -= hwbprops.CentrePoint->x;
                coords->y -= hwbprops.CentrePoint->y;
                polar = GeoCalc_2d::cartesian_to_polar(coords);
                *coords = GeoCalc_2d::rotate(coords, - hue_angle);
                if(polar.r <= hwbprops.ring_outerradius and polar.r >= hwbprops.ring_innerradius){
                    hwbpix->h = (polar.angle + M_PI * 0.5f) / (M_PI * 2.0f);
                    if (hwbpix->h < 0){
                        hwbpix->h += 1.0f;
                    }
                    hwbpix->w = 0; hwbpix->b = 0;
                    t = Converter::hwb_to_rgb(hwbpix);
                    pixcolor = Converter::float_to_int_rgb(&t);
                } else if(polar.r <= hwbprops.triangle_height*2.0f/3.0f 
                    and GeoCalc_2d::inside_triangle(coords, hwbprops.BlackPoint, hwbprops.WhitePoint, hwbprops.VividPoint)){
                    //calculate pixel color inside hwb triangle
                    hwbpix->h = hue;
                    hwbpix->w = (GeoCalc_2d::distance(coords, hwbprops.NoWhiteLine) / hwbprops.triangle_height);
                    hwbpix->b = (GeoCalc_2d::distance(coords, hwbprops.NoBlackLine) / hwbprops.triangle_height);
                    // float max_wb = 
                    t = Converter::hwb_to_rgb(hwbpix);
                    pixcolor = Converter::float_to_int_rgb(&t);
                } else{
                    //point is of bg_color color
                    pixcolor = bg_color;
                }
                //appply the iffed pixel color
                *pixel=(((unsigned int)(pixcolor.a *255) << 24) + ((unsigned)pixcolor.r << 16)
                    + ((unsigned)pixcolor.g << 8) + ((unsigned)pixcolor.b));
                if (i<width_-1) *pixel++;
            }
        }

        cairo_surface_t* picker_surface;
        picker_surface = cairo_image_surface_create_for_data((unsigned char*)buffer, CAIRO_FORMAT_ARGB32,
                                                             width_, height_, stride);
        cairo_save(cr);
        cairo_set_source_surface(cr, picker_surface, 0,0);
        cairo_paint(cr);
        cairo_restore(cr);
        if(picker_surface) cairo_surface_destroy(picker_surface);

        cairo_set_source_surface(cr, surface, 0,0);
        hue_angle -= (M_PI * 0.5f);
        GdkRGBA* outerborder=g_new(GdkRGBA, 1);
        GdkRGBA* innerborder=g_new(GdkRGBA, 1);
        outerborder->red=outerborder->green=outerborder->blue=outerborder->alpha=1;
        innerborder->red=innerborder->green=innerborder->blue=0;
        innerborder->alpha=1;
        std::swap(innerborder, outerborder);
        //painting the color ring's handle
        cairo_set_source_rgba(cr, outerborder->red, outerborder->green, outerborder->blue, outerborder->alpha);
        cairo_arc(cr, hwbprops.CentrePoint->x, hwbprops.CentrePoint->y, hwbprops.ring_outerradius+2.5, 
                hue_angle - 0.05f*hwbprops.drag_bar_scale, hue_angle + 0.05f*hwbprops.drag_bar_scale);
        cairo_arc_negative(cr, hwbprops.CentrePoint->x, hwbprops.CentrePoint->y, hwbprops.ring_innerradius-2.5,
                hue_angle + 0.05f*hwbprops.drag_bar_scale, hue_angle - 0.05f*hwbprops.drag_bar_scale);
        cairo_close_path(cr);
        cairo_stroke(cr);
        cairo_new_path(cr);
        cairo_set_source_rgba(cr, innerborder->red, innerborder->green, innerborder->blue, innerborder->alpha);
        cairo_arc(cr, hwbprops.CentrePoint->x, hwbprops.CentrePoint->y, hwbprops.ring_outerradius+1.3, 
                hue_angle - 0.04f*hwbprops.drag_bar_scale, hue_angle + 0.04f*hwbprops.drag_bar_scale);
        cairo_arc_negative(cr, hwbprops.CentrePoint->x, hwbprops.CentrePoint->y, hwbprops.ring_innerradius-1.3,
                hue_angle + 0.04f*hwbprops.drag_bar_scale, hue_angle - 0.04f*hwbprops.drag_bar_scale);
        cairo_close_path(cr);
        cairo_stroke(cr);
        cairo_new_path(cr);

        if (hwb_color->w > 0.75) {
            std::swap(innerborder, outerborder);
        }

        //painting the dot inside the triangle
        Geometry::Point2 dot_centre = *hwbprops.WhitePoint;
        Vector2 v(hwbprops.WhitePoint, hwbprops.VividPoint);
        t=_gdk_rgba_to_rgb(color);
        ColorSpaces::HSV hsv_color = Converter::rgb_to_hsv(&t);
        v = v.multiply(hsv_color.v);
        dot_centre = GeoCalc_2d::move(&dot_centre, &v);
        v = Vector2(hwbprops.VividPoint, hwbprops.BlackPoint);
        v = v.multiply((1 - hsv_color.s) * hsv_color.v);
        dot_centre = GeoCalc_2d::move(&dot_centre, &v);
        
        niffie("dot: "+std::to_string(dot_centre.x)+' '+std::to_string(dot_centre.y));
        dot_centre = GeoCalc_2d::rotate(&dot_centre, (- hue_angle) + M_PI*0.5f);
        niffie("dot: "+std::to_string(dot_centre.x)+' '+std::to_string(dot_centre.y));
        float dot_x = hwbprops.CentrePoint->x + dot_centre.x;
        float dot_y = hwbprops.CentrePoint->y - dot_centre.y;
        float dot_radius = 10.0f * hwbprops.drag_dot_scale + 2; 
        cairo_set_source_rgba(cr, outerborder->red, outerborder->green, outerborder->blue, outerborder->alpha);
        cairo_arc(cr, dot_x, dot_y, dot_radius, 0, 2*M_PI);
        cairo_stroke(cr);
        dot_radius--;
        cairo_new_path(cr);
        cairo_set_source_rgba(cr, innerborder->red, innerborder->green, innerborder->blue, innerborder->alpha);
        cairo_arc(cr, dot_x, dot_y, dot_radius, 0, 2*M_PI);
        cairo_stroke(cr);
        dot_radius--;
        cairo_set_source_rgba(cr, color->red, color->green, color->blue, 1.0);
        cairo_arc(cr, dot_x, dot_y, dot_radius, 0, 2*M_PI);
        cairo_fill(cr);


        g_free(buffer);
        g_free(hwbpix);
        g_free(color);
        g_free(hwb_color);
        g_free(coords);
        g_free(innerborder);
        g_free(outerborder);
    }
};

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget* window;
    GtkWidget* grid;
    GtkWidget* notebook;
    // GtkWidget* hslpage;
    // GtkWidget* hslpage_tab;
    // GtkWidget* hsl_chooser;
    // GtkGesture* hsl_drag;
    // GtkWidget* hsvpage;
    // GtkWidget* hsvpage_tab;
    // GtkWidget* hsv_chooser;
    // GtkWidget* hwbpage;
    // GtkWidget* hwbpage_tab;
    // GtkWidget* hwb_chooser;

    window=gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "color picker");
    g_signal_connect(window, "destroy", G_CALLBACK(close_window), GTK_WINDOW(window));
    grid=gtk_grid_new();
    std::cout<<"begin ";


    //notebook with all of the color choosers 
    notebook=gtk_notebook_new();
    gtk_grid_attach(GTK_GRID(grid), notebook, 0, 0, 3, 3);

    std::cout<<"begin ";
    CURRENT_COLOR=g_new(GdkRGBA, 1);
    std::cout<<"a";
    CURRENT_COLOR->red=0.4;
    CURRENT_COLOR->green=0.1;
    CURRENT_COLOR->blue=0.8;
    CURRENT_COLOR->alpha=1;

    // HSLTab hslchooser(GTK_NOTEBOOK(notebook), "HSL");
    HSLTab* hsl_chooser = (HSLTab::HSLTabnew(GTK_NOTEBOOK(notebook), "HSL"));
    // hslpage=gtk_frame_new(NULL);
    // hslpage_tab=gtk_label_new("HSL");
    // hsl_chooser=gtk_drawing_area_new();
    // gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hsl_chooser), 400);
    // gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hsl_chooser), 400);
    // gtk_frame_set_child(GTK_FRAME(hslpage), hsl_chooser);
    // gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hsl_chooser), paint_hsl_area, CURRENT_COLOR, (GDestroyNotify)g_free);
    // g_signal_connect_after(GTK_DRAWING_AREA(hsl_chooser), "resize", G_CALLBACK(resize_cb), NULL);
    // int hslpage_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hslpage, hslpage_tab);
    // hsl_drag=gtk_gesture_drag_new();
    // gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(hsl_drag), GDK_BUTTON_PRIMARY);
    // gtk_widget_add_controller(hsl_chooser, GTK_EVENT_CONTROLLER(hsl_drag));
    // g_signal_connect(hsl_drag, "drag-begin", G_CALLBACK(hsl_drag_begin), hsl_chooser);
    // g_signal_connect(hsl_drag, "drag-update", G_CALLBACK(hsl_drag_update), hsl_chooser);
    // g_signal_connect(hsl_drag, "drag-end", G_CALLBACK(hsl_drag_end), hsl_chooser);

    // hsvpage=gtk_frame_new(NULL);
    // hsvpage_tab=gtk_label_new("HSV");
    // hsv_chooser=gtk_drawing_area_new();
    // gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hsv_chooser), 400);
    // gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hsv_chooser), 400);
    // gtk_frame_set_child(GTK_FRAME(hsvpage), hsv_chooser);
    // gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hsv_chooser), paint_hsl_area, CURRENT_COLOR, (GDestroyNotify)on_destroy_notify);
    // g_signal_connect_after(GTK_DRAWING_AREA(hsv_chooser), "resize", G_CALLBACK(resize_cb), NULL);
    // int hsvpage_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hsvpage, hsvpage_tab);
    HSVTab* hsv_chooser = HSVTab::HSVTabnew(GTK_NOTEBOOK(notebook), "HSV");
    my_widget_signals[ACTION_SIGNAL] = g_signal_new(
            "color-change",
            G_TYPE_FROM_CLASS(GTK_DRAWING_AREA_GET_CLASS(hsl_chooser->content)),
            G_SIGNAL_RUN_FIRST,     
            0, /* class offset for default handler */     
            nullptr, nullptr,     
            g_cclosure_marshal_VOID__STRING,     
            G_TYPE_NONE, /* return type */     
            1,    /* n_params */     
            G_TYPE_STRING 
    );
    HWBTab* hwb_triangle_chooser = HWBTab::HWBTabnew(GTK_NOTEBOOK(notebook), "HWB triangle");
    // hwbpage=gtk_frame_new(NULL);
    // hwbpage_tab=gtk_label_new("HWB");
    // hwb_chooser=gtk_drawing_area_new();
    // gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hwb_chooser), 400);
    // gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hwb_chooser), 400);
    // gtk_frame_set_child(GTK_FRAME(hwbpage), hwb_chooser);
    // gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hwb_chooser), paint_hsl_area, CURRENT_COLOR, (GDestroyNotify)on_destroy_notify);
    // g_signal_connect_after(GTK_DRAWING_AREA(hwb_chooser), "resize", G_CALLBACK(resize_cb), NULL);
    // int hwbpage_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hwbpage, hwbpage_tab);

    //color tile under the chooser
    std::cout<<"middle ";
    // ColorTile* current_color_tile=g_new(ColorTile, 1);
    // *current_color_tile=_initColorTile(grid);
    // std::cout<<"end ";
    // gtk_grid_attach(GTK_GRID(grid), current_color_tile->frame, 2, 3, 1, 1);
    ColorTile* tile = ColorTile::ColorTilenew(grid, CURRENT_COLOR, 50, 50, 2, 3, 1, 1);
    g_signal_connect_data(GTK_DRAWING_AREA(hsl_chooser->content), "color-change", G_CALLBACK(update), tile->tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_DRAWING_AREA(hsv_chooser->content), "color-change", G_CALLBACK(update), tile->tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_DRAWING_AREA(hwb_triangle_chooser->content), "color-change", G_CALLBACK(update), tile->tile, on_closure_notify, G_CONNECT_SWAPPED);
    std::cout<<"a ";
    gtk_window_set_child(GTK_WINDOW(window), grid);
    std::cout<<"a ";
    gtk_window_present(GTK_WINDOW(window));
    std::cout<<"a ";
}

int main(int argc, char** argv) {
    GtkApplication* app;
    int status;
    std::cout<<"o ";
    app=gtk_application_new("colors.picker.app", G_APPLICATION_DEFAULT_FLAGS);
    std::cout<<"o ";
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    std::cout<<"o ";
    status=g_application_run(G_APPLICATION(app), argc, argv);
    std::cout<<"o ";
    g_object_unref(app);
    std::cout<<"o\n";
    return status;
}
