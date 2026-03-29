#include<gtk/gtk.h>
#include<cairo.h>
#include<iostream>
#include<math.h>
#include "colorspaces.h"

// static GdkSurface* surface = NULL;

struct ColorTile {
    GtkWidget* frame;
    GtkWidget* tile;
    GdkRGBA* color;
};

GdkRGBA* CURRENT_COLOR;

static cairo_surface_t* surface=NULL;

float drag_dot_scale = 1.0;
float drag_bar_scale = 1.0;

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

void free_tile(ColorTile* obj) {
    g_free(obj);
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

float current_hue=0.0;
bool hsl_dragged_farright = false;

void paint_hsl_area(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
    niffie("kk");
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
    if (hsl_color->s == 0.0 or hsl_dragged_farright){
        hsl_color->h = current_hue;
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

    GdkRGBA* outerborder = g_new(GdkRGBA, 1);
    GdkRGBA* innerborder = g_new(GdkRGBA, 1);
    outerborder->red = outerborder->green = outerborder->blue = outerborder->alpha = 1;
    innerborder->red = innerborder->green = innerborder->blue = 0;
    innerborder->alpha = 1;

    if(hsl_color->l < 0.5){
        std::swap(innerborder, outerborder);
    }

    //painting color dot
    int dot_radius = 10*drag_dot_scale, dot_x = hsl_color->h*width_+width/20, dot_y = hsl_color->s*height_*7+height/10;

    niffie("dot-paint"+std::to_string(dot_x)+' '+std::to_string(dot_y)+" hsl hue:"+std::to_string(hsl_color->h));
    cairo_set_source_surface(cr, surface,width/20, height/10);
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
    float bar_x = hsl_color->l*width_ + width/20, bar_y = height*7/10, bar_width=10*drag_bar_scale;
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

void paint_tile(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
    cairo_set_source_surface(cr, surface, 0, 0);
    GdkRGBA* color=((GdkRGBA*)data);
    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_paint(cr);
}

ColorTile _initColorTile(GtkWidget* grid) {
    ColorTile tile; //=*g_new(ColorTile, 1);
    std::cout<<"mid2 ";
    tile.frame=gtk_frame_new(NULL);
    std::cout<<"mid2 ";
    tile.tile=gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET(tile.tile), 50, 50);
    std::cout<<"mid2 ";
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(tile.tile), 40);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(tile.tile), 40);
    std::cout<<"mid2 ";
    gtk_frame_set_child(GTK_FRAME(tile.frame), tile.tile);
    tile.color=CURRENT_COLOR;
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(tile.tile), paint_tile, CURRENT_COLOR, (GDestroyNotify)g_free);
    g_signal_connect_after(GTK_DRAWING_AREA(tile.tile), "resize", G_CALLBACK(resize_cb), NULL);
    std::cout<<"mid2 ";
    return tile;
}

static void close_window(gpointer window) {
    if (surface) {
        cairo_surface_destroy(surface);
    }
}

float startx, starty;

void hsl_drag_begin(GtkGestureDrag *gesture, float x, float y, GtkWidget *area){
    int width = gtk_widget_get_width(area);
    int height = gtk_widget_get_height(area);
    double dx,dy;
    bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
    x=dx; y=dy;
    niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
    if(width/20 <= x and width*19/20 >= x){
        ColorSpaces::HSL* current_hsl = g_new(ColorSpaces::HSL, 1);
        ColorSpaces::RGB t = _gdk_rgba_to_rgb(CURRENT_COLOR);
        *current_hsl = Converter::rgb_to_hsl(&t);
        if (height/10 <= y and height*6/10 >= y) {
            drag_dot_scale=2.0;
            startx=x;
            starty=y;
            current_hsl->h = (x - width/20)*10 / (width*9);
            current_hue=current_hsl->h;
            current_hsl->s = (y - height/10)*2 / height;
            t=Converter::hsl_to_rgb(current_hsl);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
    niffie("drag-start"+std::to_string(startx)+' '+std::to_string(starty));
        } else if (height*7/10 <= y and height*8/10 >= y) {
            drag_bar_scale=1.5;
            startx=x;
            starty=y;
            current_hsl->l=(x - width/20)*10 / (width*9);
            t=Converter::hsl_to_rgb(current_hsl);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
    niffie("drag-start"+std::to_string(startx)+' '+std::to_string(starty));
        }
        g_free(current_hsl);
    }

    gtk_widget_queue_draw(area);
}

void hsl_drag_end(GtkGestureDrag *gesture, float x, float y, GtkWidget *area){
    int width = gtk_widget_get_width(area);
    int height = gtk_widget_get_height(area);
    drag_dot_scale = 1.0;
    drag_bar_scale=1.0;
    double dx, dy;
    bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
    x=dx; y=dy;
    gtk_widget_queue_draw(area);
}

void hsl_drag_update(GtkGestureDrag* gesture, float x, float y, GtkWidget* area) {
    float width=gtk_widget_get_width(area);
    float height = gtk_widget_get_height(area);
    double dx,dy;
    bool sth=gtk_gesture_drag_get_offset(gesture, &dx, &dy);
    niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
    x=dx; y=dy;
    x += startx;
    y+=starty;
    ColorSpaces::HSL* current_hsl=g_new(ColorSpaces::HSL, 1);
    ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
    *current_hsl=Converter::rgb_to_hsl(&t);
    if (drag_dot_scale>1.0) {
        float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
        float bound_y=std::min(height*6/10.0f, std::max(height/10.0f, y));
        // startx=bound_x;
        // starty=bound_y;
        current_hsl->h=(bound_x - width/20)*10 / (width*9);
            current_hue=current_hsl->h;
            if(x > width*19/20.0f){
                current_hue = 1.0;
                hsl_dragged_farright = true;
            } else{
                hsl_dragged_farright = false;
            }
        current_hsl->s=(bound_y - height/10)*2 / height;

    niffie("drag-upd"+std::to_string(bound_x)+' '+std::to_string(bound_y));
        t=Converter::hsl_to_rgb(current_hsl);
        *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
    } else if (drag_bar_scale>1.0) {
        float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
        y=height*7.5/10;
        // startx=bound_x;
        // starty=y;
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



static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget* window;
    GtkWidget* grid;
    GtkWidget* notebook;
    GtkWidget* hslpage;
    GtkWidget* hslpage_tab;
    GtkWidget* hsl_chooser;
    GtkGesture* hsl_drag;
    GtkWidget* hsvpage;
    GtkWidget* hsvpage_tab;
    GtkWidget* hsv_chooser;
    GtkWidget* hwbpage;
    GtkWidget* hwbpage_tab;
    GtkWidget* hwb_chooser;

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

    hslpage=gtk_frame_new(NULL);
    hslpage_tab=gtk_label_new("HSL");
    hsl_chooser=gtk_drawing_area_new();
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hsl_chooser), 400);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hsl_chooser), 400);
    gtk_frame_set_child(GTK_FRAME(hslpage), hsl_chooser);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hsl_chooser), paint_hsl_area, CURRENT_COLOR, (GDestroyNotify)g_free);
    g_signal_connect_after(GTK_DRAWING_AREA(hsl_chooser), "resize", G_CALLBACK(resize_cb), NULL);
    int hslpage_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hslpage, hslpage_tab);
    hsl_drag = gtk_gesture_drag_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(hsl_drag), GDK_BUTTON_PRIMARY);
    gtk_widget_add_controller(hsl_chooser, GTK_EVENT_CONTROLLER(hsl_drag));
    g_signal_connect(hsl_drag, "drag-begin", G_CALLBACK(hsl_drag_begin), hsl_chooser);
    g_signal_connect(hsl_drag, "drag-update", G_CALLBACK(hsl_drag_update), hsl_chooser);
    g_signal_connect(hsl_drag, "drag-end", G_CALLBACK(hsl_drag_end), hsl_chooser);

    hsvpage=gtk_frame_new(NULL);
    hsvpage_tab=gtk_label_new("HSV");
    hsv_chooser=gtk_drawing_area_new();
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hsv_chooser), 400);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hsv_chooser), 400);
    gtk_frame_set_child(GTK_FRAME(hsvpage), hsv_chooser);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hsv_chooser), paint_hsl_area, CURRENT_COLOR, (GDestroyNotify)g_free);
    g_signal_connect_after(GTK_DRAWING_AREA(hsv_chooser), "resize", G_CALLBACK(resize_cb), NULL);
    int hsvpage_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hsvpage, hsvpage_tab);

    hwbpage=gtk_frame_new(NULL);
    hwbpage_tab=gtk_label_new("HWB");
    hwb_chooser=gtk_drawing_area_new();
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hwb_chooser), 400);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hwb_chooser), 400);
    gtk_frame_set_child(GTK_FRAME(hwbpage), hwb_chooser);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hwb_chooser), paint_hsl_area, CURRENT_COLOR, (GDestroyNotify)g_free);
    g_signal_connect_after(GTK_DRAWING_AREA(hwb_chooser), "resize", G_CALLBACK(resize_cb), NULL);
    int hwbpage_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hwbpage, hwbpage_tab);

    //color tile under the chooser
    std::cout<<"middle ";
    ColorTile* current_color_tile=g_new(ColorTile, 1);
    *current_color_tile=_initColorTile(grid);
    std::cout<<"end ";
    gtk_grid_attach(GTK_GRID(grid), current_color_tile->frame, 2, 3, 1, 1);
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
