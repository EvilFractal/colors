#include<gtk/gtk.h>
#include<gdk/gdk.h>
#include<cairo.h>
#include<iostream>
#include<math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include<cstring>
#include "colorspaces.h"
#include "geometry.h"

#define DEBUG true

GdkRGBA* CURRENT_COLOR;

static cairo_surface_t* surface=NULL;

class ColorTile;
void update(GtkWidget* widget, gpointer data);
void update_tile(ColorTile* tile, gpointer data);

GtkCssProvider* css_provider;

enum { 
    TEXTBOX_CC_CHANGED_SIGNAL,
    TOGGLE_PICKER_SIGNAL,
    DRAW_ACTION_SIGNAL,
    LAST_SIGNAL
};
static guint my_widget_signals[LAST_SIGNAL] = { 0 };

enum controllable_properties {
    NO_CONTROL = -1,  /* for testing / any tbox that influences nothing, safe default value */
    CC_F_RED, CC_F_GREEN, CC_F_BLUE, /* CURRENT_COLOR floating-point rgb values */
    CC_I_RED, CC_I_GREEN, CC_I_BLUE, /* CURRENT_COLOR floating-point rgb values */
    CC_ALPHA, /* CURENT_COLOR alpha */
    CC_HUE, /* CURRENT_COLOR hue */
};

/* convert ColorSpaces::RGB color format to GdkRGBA */
GdkRGBA _rgb_to_gdk_rgba(ColorSpaces::RGB* color) {
    GdkRGBA result; 
    result.red=color->r;
    result.green=color->g;
    result.blue=color->b;
    result.alpha=color->a;
    return result;
}

/* convert GdkRGBA color format to ColorSpaces::RGB */
ColorSpaces::RGB _gdk_rgba_to_rgb(GdkRGBA* color) {
    ColorSpaces::RGB result;
    result.r=color->red;
    result.g=color->green;
    result.b=color->blue;
    result.a=color->alpha;
    return result;
}

/* convenience function for printing stuff */
void niffie(std::string message) {
    if(DEBUG) std::cout<<message<<'\n'<<std::flush;
    return;
}

/* clear the surface */
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
    }
    if (gtk_native_get_surface(gtk_widget_get_native(widget))) {
        GdkRGBA* color=g_new(GdkRGBA, 1);
        *color=*CURRENT_COLOR;
        surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, gtk_widget_get_width(widget), gtk_widget_get_height(widget));
        niffie(std::to_string(gtk_widget_get_width(widget))+' '+std::to_string(gtk_widget_get_height(widget))+'\n');
        /* Initialize the surface to color */
        clear_surface(color);
        g_free(color);
    }
}


/** dummy function to fill 'GClosureNotify' field 
 * as it is required but does nothing
*/
static void on_closure_notify(gpointer data, GClosure *closure){
    return;
}

/* 'GDestroyNotify', destroys given data */
static void on_destroy_notify(gpointer data){
    g_free(data);
}

static void unfocus(GtkWindow* window, gpointer data){
    gtk_window_set_focus(window, NULL);
}

class ColorTile {
private:
    GtkWidget* frame; /* contains the tile */
    GtkWidget* tile; /* rectangle of a specific color */
    GdkRGBA* color; /* color of the tile */

public:
    /** method for constructing a ColorTile 
     * 
     * @param parent the parent widget (container) in which will the tile reside
     * @param def_color default (initial) color of the tile
     * @param width tile width in pixels
     * @param height tile height in pixels
     * 
     * if the parent is a grid:
     * @param grid_column column where the tile should be put
     * @param grid_row row where the tile should be put
     * @param grid_vert_span how many rows should the tile span
     * @param grid_hz_span how many columns should the tile span
     */
    static ColorTile * ColorTilenew(GtkWidget* parent, GdkRGBA* def_color,
        int width=50, int height=50,
        int grid_column=0, int grid_row=0, int grid_vert_span=1, int grid_hz_span=1) {
        ColorTile * tile = g_new(ColorTile, 1);
        tile->color=g_new(GdkRGBA, 1);
        *(tile->color) =  *def_color;
        tile->frame=gtk_frame_new(NULL);
        tile->tile=gtk_drawing_area_new();
        gtk_widget_set_size_request(GTK_WIDGET(tile->tile), width, height);
        gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(tile->tile), width);
        gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(tile->tile), height);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(tile->tile), paint_tile, tile, (GDestroyNotify)on_destroy_notify);
        g_signal_connect_after(GTK_DRAWING_AREA(tile->tile), "resize", G_CALLBACK(resize_cb), NULL);
        // g_signal_connect_data(GTK_GRID(parent), "state-flags-changed", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
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

    GtkWidget* get_tile(){ return tile; } /* get the drawingArea of the tile */
    GtkWidget* get_frame(){ return frame; } /* get the frame of the tile */
    GdkRGBA* get_color(){ return color; } /* get the color of the tile */

    void set_color(GdkRGBA* new_color){ *color = *new_color; }

    /* GtkDrawingAreaDrawFunc drawing function of the tile */
    static void paint_tile(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
        niffie("paint-tile");
        cairo_set_source_surface(cr, surface, 0, 0);
        ColorTile* tile=((ColorTile*)data);
        cairo_set_source_rgba(cr, tile->color->red, tile->color->green, tile->color->blue, tile->color->alpha);
        cairo_paint(cr);
    }

};

/* utility function for passing update signal to a widget */
void update(GtkWidget* widget, gpointer data){
    gtk_widget_queue_draw(widget);
}

/* utility function for passing update signal to a tile */
void update_tile(ColorTile* tile, gpointer data){
    niffie("update_tile");
    if (CURRENT_COLOR != tile->get_color()){
        tile->set_color(CURRENT_COLOR);
        niffie("-----------------------------------");
        g_signal_emit_by_name(GTK_DRAWING_AREA(tile->get_tile()), "color-change");
    }
    gtk_widget_queue_draw(tile->get_tile());
}

/* utility function for passing update signal to a notebook */
void update_nb(GtkNotebook* notebook, gpointer data){
    int active_page = gtk_notebook_get_current_page(notebook);
    gtk_widget_queue_draw(gtk_frame_get_child(GTK_FRAME(gtk_notebook_get_nth_page(notebook, active_page))));

}

float current_hue=0.0;

/** dummy function to fill window closing callback
 * does nothing
 */
static void close_window(gpointer window) {
    return;
}

/** color chooser class primarily to be used as a notebook tab
 * @class ColorChooserTab 
 */
class ColorChooserTab {
protected:
    GtkGesture* drag; /* drag gesture to interact with the chooser */
    GtkWidget* frame; /* contains all insides, especially content */
    GtkWidget* tab; /* the notebook tab that holds the label */
    GtkWidget* content; /* DrawingArea to render the chooser */
    int page_num; /* notebook tab page number of the chooser */

public:
    GtkGesture* get_drag_gesture(){ return drag; } /* get the drag gesture to interact with the chooser */
    GtkWidget* get_frame(){ return frame; } /* get the frame of the chooser tab */
    GtkWidget* get_tab(){ return tab; } /* get the tab that allows to switch to the page of the chooser */
    GtkWidget* get_content(){ return content; } /* get the drawingArea that contains the chooser */
    int get_page_num(){ return page_num; } /* get the number of the notebook page the chooser is at */
};

class HSLTab :public ColorChooserTab {
private:
    float startx, starty; /* start coordinates of a gesture */
    bool hsl_dragged_farright; /* was the dot dragged beyond right border, needed to preserve position of the dot */
    float drag_dot_scale; /* if not 1.0, then the user is probably dragging the dot */
    float drag_bar_scale; /* if not 1.0, the bar is probably being dragged */

public:
    /** method for constructing a HSLTab
     * 
     * @param notebook the notebook of which the page shall it be
     * @param tab_name string to display as the tab name
     * @param width page width in pixels
     * @param height page height in pixels
     */
    static HSLTab* HSLTabnew(GtkNotebook* notebook, const char* tab_name, int width=400, int height=400) {
        HSLTab* hsltab = g_new(HSLTab, 1);
        hsltab->frame=gtk_frame_new(NULL);
        hsltab->tab=gtk_label_new(tab_name);
        hsltab->content=gtk_drawing_area_new();
        gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hsltab->content), width);
        gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hsltab->content), height);
        gtk_frame_set_child(GTK_FRAME(hsltab->frame), hsltab->content);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hsltab->content), draw, hsltab, (GDestroyNotify)on_destroy_notify);
        g_signal_connect_after(GTK_DRAWING_AREA(hsltab->content), "resize", G_CALLBACK(resize_cb), NULL);
        hsltab->page_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hsltab->frame, hsltab->tab);
        hsltab->drag=gtk_gesture_drag_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(hsltab->drag), GDK_BUTTON_PRIMARY);
        gtk_widget_add_controller(hsltab->content, GTK_EVENT_CONTROLLER(hsltab->drag));
        g_signal_connect(hsltab->drag, "drag-begin", G_CALLBACK(drag_begin), hsltab);
        g_signal_connect(hsltab->drag, "drag-update", G_CALLBACK(drag_update), hsltab);
        g_signal_connect(hsltab->drag, "drag-end", G_CALLBACK(drag_end), hsltab);
        hsltab->starty = 0;
        hsltab->startx = 0;
        hsltab->drag_bar_scale = 1.0;
        hsltab->drag_dot_scale = 1.0;
        hsltab->hsl_dragged_farright = false;
        return hsltab;
    }

    /** GCallback for drag gesture start for the hsl tab */
    static void drag_begin(GtkGestureDrag* gesture, float x, float y, HSLTab* tab) {
        int width=gtk_widget_get_width(tab->content);
        int height=gtk_widget_get_height(tab->content);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
        if (width/20 <= x and width*19/20 >= x) {
            ColorSpaces::HSL* current_hsl=g_new(ColorSpaces::HSL, 1);
            ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
            *current_hsl=Converter::rgb_to_hsl(&t);
            if (height/10 <= y and height*6/10 >= y) {
                tab->drag_dot_scale=2.0;
                tab->startx=x;
                tab->starty=y;
                current_hsl->h=(x - width/20)*10 / (width*9);
                current_hue=current_hsl->h;
                current_hsl->s=(y - height/10)*2 / height;
                t=Converter::hsl_to_rgb(current_hsl);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
                niffie("drag-start"+std::to_string(tab->startx)+' '+std::to_string(tab->starty));
            } else if (height*7/10 <= y and height*8/10 >= y) {
                tab->drag_bar_scale=1.5;
                tab->startx=x;
                tab->starty=y;
                current_hsl->l=(x - width/20)*10 / (width*9);
                t=Converter::hsl_to_rgb(current_hsl);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
                niffie("drag-start"+std::to_string(tab->startx)+' '+std::to_string(tab->starty));
            }
            g_free(current_hsl);
        }

        gtk_widget_queue_draw(tab->content);
    }

    /* GCallback for drag gesture updates for the hsl tab */
    static void drag_update(GtkGestureDrag* gesture, float x, float y, HSLTab* tab) {
        float width=gtk_widget_get_width(tab->content);
        float height=gtk_widget_get_height(tab->content);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_offset(gesture, &dx, &dy);
        niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
        x=dx; y=dy;
        x+=tab->startx;
        y+=tab->starty;
        ColorSpaces::HSL* current_hsl=g_new(ColorSpaces::HSL, 1);
        ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
        *current_hsl=Converter::rgb_to_hsl(&t);
        if (tab->drag_dot_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            float bound_y=std::min(height*6/10.0f, std::max(height/10.0f, y));
            current_hsl->h=(bound_x - width/20)*10 / (width*9);
            current_hue=current_hsl->h;
            if (x > width*19/20.0f) {
                current_hue=1.0;
                tab->hsl_dragged_farright=true;
            } else {
                tab->hsl_dragged_farright=false;
            }
            current_hsl->s=(bound_y - height/10)*2 / height;

            niffie("drag-upd"+std::to_string(bound_x)+' '+std::to_string(bound_y));
            t=Converter::hsl_to_rgb(current_hsl);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
            g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
        } else if (tab->drag_bar_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            y=height*7.5/10;
            current_hsl->l=(bound_x - width/20)*10 / (width*9);
            t=Converter::hsl_to_rgb(current_hsl);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
            g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
        }
        g_free(current_hsl);
        niffie("trying to draw");
        gtk_widget_queue_draw(tab->content);
    }

    /* GCallback for drag gesture end for the hsl tab */
    static void drag_end(GtkGestureDrag* gesture, float x, float y, HSLTab* tab) {
        int width=gtk_widget_get_width(tab->content);
        int height=gtk_widget_get_height(tab->content);
        tab->drag_dot_scale=1.0;
        tab->drag_bar_scale=1.0;
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        gtk_widget_queue_draw(tab->content);
    }

    /* GtkDrawingAreaDrawFunc drawing function of the hsl tab
     * from outside called by gtk_widget_queue_draw(drawing_area) */
    static void draw(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
        niffie("kk");
        // g_signal_emit_by_name(drawing_area, "color-change");
        HSLTab* tab = (HSLTab*)data;
        GdkRGBA* color=g_new(GdkRGBA, 1);
        *color=*CURRENT_COLOR;
        cairo_set_source_surface(cr, surface, 0, 0);
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1);
        cairo_paint(cr);

        niffie(std::to_string(cairo_get_reference_count(cr)));
        // cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
        // cairo_rectangle(cr, 30, 30, 50, 50);
        // cairo_stroke(cr);
        niffie("paint ");
        int stride, width_, height_;
        width_=gtk_widget_get_width(GTK_WIDGET(drawing_area))*9/10;
        height_=gtk_widget_get_height(GTK_WIDGET(drawing_area))/2;
        stride=cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width_);
        unsigned int* buffer=g_new(unsigned int, (height_*stride)/4);
        unsigned int* pixel;
        int hue, sat, light;
        niffie("painter ");
        niffie("ooop");
        ColorSpaces::HSL* hsl_color=g_new(ColorSpaces::HSL, 1);
        ColorSpaces::RGB t=(_gdk_rgba_to_rgb(color));
        *hsl_color=Converter::rgb_to_hsl(&t);
        ColorSpaces::HSL* hslpix=g_new(ColorSpaces::HSL, 1);
        if (hsl_color->s == 0.0 or tab->hsl_dragged_farright) {
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

        niffie("paint ");
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
        int dot_radius=10*tab->drag_dot_scale, dot_x=hsl_color->h*width_+width/20, dot_y=hsl_color->s*height_*7+height/10;

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
        float bar_x=hsl_color->l*width_ + width/20, bar_y=height*7/10, bar_width=10*tab->drag_bar_scale;
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
        niffie("paint-eeeeeeeeeeeeeee");
    }
};

class HSVTab :public ColorChooserTab {

private:
    float startx, starty; /* start coordinates of a gesture */
    bool hsv_dragged_farright; /* was the dot dragged beyond right border, needed to preserve position of the dot */
    float drag_dot_scale; /* if not 1.0, then the user is probably dragging the dot */
    float drag_bar_scale; /* if not 1.0, the bar is probably being dragged */

public:
    /** metho, needed to preserod for constructing a HSVTab
     * 
     * @param notebook the notebook of which the page shall it be
     * @param tab_name string to display as the tab name
     * @param width page width in pixels
     * @param height page height in pixels
     */
    static HSVTab* HSVTabnew(GtkNotebook* notebook, const char* tab_name,
                            int width=400, int height=400) {
        HSVTab* hsvtab = g_new(HSVTab, 1);
        hsvtab->frame=gtk_frame_new(NULL);
        hsvtab->tab=gtk_label_new(tab_name);
        hsvtab->content=gtk_drawing_area_new();
        gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hsvtab->content), width);
        gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hsvtab->content), height);
        gtk_frame_set_child(GTK_FRAME(hsvtab->frame), hsvtab->content);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hsvtab->content), draw, hsvtab, (GDestroyNotify)on_destroy_notify);
        g_signal_connect_after(GTK_DRAWING_AREA(hsvtab->content), "resize", G_CALLBACK(resize_cb), NULL);
        hsvtab->page_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hsvtab->frame, hsvtab->tab);
        hsvtab->drag=gtk_gesture_drag_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(hsvtab->drag), GDK_BUTTON_PRIMARY);
        gtk_widget_add_controller(hsvtab->content, GTK_EVENT_CONTROLLER(hsvtab->drag));
        g_signal_connect(hsvtab->drag, "drag-begin", G_CALLBACK(drag_begin), hsvtab);
        g_signal_connect(hsvtab->drag, "drag-update", G_CALLBACK(drag_update), hsvtab);
        g_signal_connect(hsvtab->drag, "drag-end", G_CALLBACK(drag_end), hsvtab);
        hsvtab->starty = 0;
        hsvtab->startx = 0;
        hsvtab->drag_bar_scale = 1.0;
        hsvtab->drag_dot_scale = 1.0;
        hsvtab->hsv_dragged_farright = false;
        return hsvtab;
    }

    /* GCallback for drag gesture start for the hsv tab */
    static void drag_begin(GtkGestureDrag* gesture, float x, float y, HSVTab* tab) {
        int width=gtk_widget_get_width(tab->content);
        int height=gtk_widget_get_height(tab->content);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
        if (width/20 <= x and width*19/20 >= x) {
            ColorSpaces::HSV* current_hsv=g_new(ColorSpaces::HSV, 1);
            ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
            *current_hsv=Converter::rgb_to_hsv(&t);
            if (height/10 <= y and height*6/10 >= y) {
                tab->drag_dot_scale=2.0;
                tab->startx=x;
                tab->starty=y;
                current_hsv->h=(x - width/20)*10 / (width*9);
                current_hue=current_hsv->h;
                current_hsv->s=(y - height/10)*2 / height;
                t=Converter::hsv_to_rgb(current_hsv);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
                niffie("drag-start"+std::to_string(tab->startx)+' '+std::to_string(tab->starty));
            } else if (height*7/10 <= y and height*8/10 >= y) {
                tab->drag_bar_scale=1.5;
                tab->startx=x;
                tab->starty=y;
                current_hsv->v=(x - width/20)*10 / (width*9);
                t=Converter::hsv_to_rgb(current_hsv);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
                niffie("drag-start"+std::to_string(tab->startx)+' '+std::to_string(tab->starty));
            }
            g_free(current_hsv);
        }

        gtk_widget_queue_draw(tab->content);
    }

    /* GCallback for drag gesture updates for the hsv tab */
    static void drag_update(GtkGestureDrag* gesture, float x, float y, HSVTab* tab) {
        float width=gtk_widget_get_width(tab->content);
        float height=gtk_widget_get_height(tab->content);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_offset(gesture, &dx, &dy);
        niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
        x=dx; y=dy;
        x+=tab->startx;
        y+=tab->starty;
        ColorSpaces::HSV* current_hsv=g_new(ColorSpaces::HSV, 1);
        ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
        *current_hsv=Converter::rgb_to_hsv(&t);
        if (tab->drag_dot_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            float bound_y=std::min(height*6/10.0f, std::max(height/10.0f, y));
            current_hsv->h=(bound_x - width/20)*10 / (width*9);
            current_hue=current_hsv->h;
            if (x > width*19/20.0f) {
                current_hue=1.0;
                tab->hsv_dragged_farright=true;
            } else {
                tab->hsv_dragged_farright=false;
            }
            current_hsv->s=(bound_y - height/10)*2 / height;

            niffie("drag-upd"+std::to_string(bound_x)+' '+std::to_string(bound_y));
            t=Converter::hsv_to_rgb(current_hsv);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
            g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
        } else if (tab->drag_bar_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            y=height*7.5/10;
            current_hsv->v=(bound_x - width/20)*10 / (width*9);
            t=Converter::hsv_to_rgb(current_hsv);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
            g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
        }
        g_free(current_hsv);
        niffie("trying to draw");
        gtk_widget_queue_draw(tab->content);
    }

    /* GCallback for drag gesture end for the hsv tab */
    static void drag_end(GtkGestureDrag* gesture, float x, float y, HSVTab* tab) {
        int width=gtk_widget_get_width(tab->content);
        int height=gtk_widget_get_height(tab->content);
        tab->drag_dot_scale=1.0;
        tab->drag_bar_scale=1.0;
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        gtk_widget_queue_draw(tab->content);
    }

    /* GtkDrawingAreaDrawFunc drawing function of the hsv tab 
     * from outside called by gtk_widget_queue_draw(drawing_area) */
    static void draw(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
        niffie("kk");
        // g_signal_emit_by_name(drawing_area, "color-change");
        HSVTab* tab = (HSVTab*)data;
        GdkRGBA* color=g_new(GdkRGBA, 1);
        *color=*CURRENT_COLOR;
        cairo_set_source_surface(cr, surface, 0, 0);
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1);
        cairo_paint(cr);

        niffie(std::to_string(cairo_get_reference_count(cr)));
        // cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
        // cairo_rectangle(cr, 30, 30, 50, 50);
        // cairo_stroke(cr);
        niffie("paint ");
        int stride, width_, height_;
        width_=gtk_widget_get_width(GTK_WIDGET(drawing_area))*9/10;
        height_=gtk_widget_get_height(GTK_WIDGET(drawing_area))/2;
        stride=cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width_);
        unsigned int* buffer=g_new(unsigned int, (height_*stride)/4);
        unsigned int* pixel;
        int hue, sat, val;
        niffie("painter ");
        niffie("ooop");
        ColorSpaces::HSV* hsv_color=g_new(ColorSpaces::HSV, 1);
        ColorSpaces::RGB t=(_gdk_rgba_to_rgb(color));
        *hsv_color=Converter::rgb_to_hsv(&t);
        ColorSpaces::HSV* hsvpix=g_new(ColorSpaces::HSV, 1);
        if (hsv_color->s == 0.0 or tab->hsv_dragged_farright) {
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

        niffie("paint ");
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
        int dot_radius=10*tab->drag_dot_scale, dot_x=hsv_color->h*width_+width/20, dot_y=hsv_color->s*height_*7+height/10;

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
        float bar_x=hsv_color->v*width_ + width/20, bar_y=height*7/10, bar_width=10*tab->drag_bar_scale;
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
        niffie("paint-eeeeeeeeeeeeeee");
    }
};

class HWBTab :public ColorChooserTab {
private: 
    float startx, starty; /* start coordinates of a gesture */
    bool hwb_dragged_outside; /* was the dot dragged out of the ring/triangle, needed to preserve position of the dot */
    float drag_dot_scale; /* if not 1.0, then the user is probably dragging the dot */
    float drag_bar_scale; /* if not 1.0, the bar is probably being dragged */
    float ring_innerradius; /* inner ring radius */
    float ring_outerradius; /* outer ring radius */
    Geometry::Point2* CentrePoint; /* central point of the rotating triangle & outer ring*/
    Geometry::Point2* VividPoint; /* the vivid vertice of the triangle */
    Geometry::Point2* BlackPoint; /* the black vertice of the triangle */
    Geometry::Point2* WhitePoint; /* the white vertice of the triangle */
    Geometry::LineGeneral2* NoIntensityLine; /* line containing the edge between black & white vertices */
    Geometry::LineGeneral2* NoBlackLine; /* line containing the edge between white & vivid vertices */
    Geometry::LineGeneral2* NoWhiteLine; /* line containing the edge between black & vivid vertices */
    float triangle_height; /* the equilateral triangle chooser height */

public:
    /** method for constructing a HWBTab
     * 
     * @param notebook the notebook of which the page shall it be
     * @param tab_name string to display as the tab name
     * @param width page width in pixels
     * @param height page height in pixels
     */
    static HWBTab* HWBTabnew(GtkNotebook* notebook, const char* tab_name,
                             int width=400, int height=400){
        HWBTab* hwbtab = g_new(HWBTab, 1);
        hwbtab->frame=gtk_frame_new(NULL);
        hwbtab->tab=gtk_label_new(tab_name);
        hwbtab->content=gtk_drawing_area_new();
        gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(hwbtab->content), width);
        gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(hwbtab->content), height);
        gtk_frame_set_child(GTK_FRAME(hwbtab->frame), hwbtab->content);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hwbtab->content), draw, hwbtab, (GDestroyNotify)on_destroy_notify);
        g_signal_connect_after(GTK_DRAWING_AREA(hwbtab->content), "resize", G_CALLBACK(resize_cb), NULL);
        hwbtab->page_num=gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hwbtab->frame, hwbtab->tab);
        hwbtab->drag=gtk_gesture_drag_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(hwbtab->drag), GDK_BUTTON_PRIMARY);
        gtk_widget_add_controller(hwbtab->content, GTK_EVENT_CONTROLLER(hwbtab->drag));
        g_signal_connect(hwbtab->drag, "drag-begin", G_CALLBACK(drag_begin), hwbtab);
        g_signal_connect(hwbtab->drag, "drag-update", G_CALLBACK(drag_update), hwbtab);
        g_signal_connect(hwbtab->drag, "drag-end", G_CALLBACK(drag_end), hwbtab);
        hwbtab->starty = 0;
        hwbtab->startx = 0;
        hwbtab->drag_bar_scale = 1.0;
        hwbtab->drag_dot_scale = 1.0;
        hwbtab->hwb_dragged_outside = false;
        hwbtab->CentrePoint = g_new(Geometry::Point2, 1);
        hwbtab->CentrePoint->x = float(width/2);
        (hwbtab->CentrePoint->y) = float(height/2);
        hwbtab->ring_outerradius = float(width/2 - 20);
        hwbtab->ring_innerradius = hwbtab->ring_outerradius - 20.0;
        hwbtab->triangle_height = (hwbtab->ring_innerradius - 15.0)*3.0 / 2.0;
        Geometry::Point2* vertice = g_new(Geometry::Point2, 1);
        *vertice = (Geometry::Point2){.x=0, .y=-(hwbtab->ring_innerradius - 15.0f)};
        hwbtab->VividPoint = vertice;
        hwbtab->WhitePoint = g_new(Geometry::Point2, 1);
        hwbtab->BlackPoint = g_new(Geometry::Point2, 1);
        *hwbtab->WhitePoint = GeoCalc_2d::rotate(vertice, 2.0*M_PI/3.0);
        *hwbtab->BlackPoint = GeoCalc_2d::rotate(vertice, 4.0*M_PI/3.0);
        hwbtab->NoIntensityLine = g_new(Geometry::LineGeneral2, 1);
        hwbtab->NoBlackLine = g_new(Geometry::LineGeneral2, 1);
        hwbtab->NoWhiteLine = g_new(Geometry::LineGeneral2, 1);
        *hwbtab->NoIntensityLine = {.A=0, .B=1, .C=(vertice->y)/2};
        *hwbtab->NoWhiteLine = {.A=std::sqrtf(3.0), .B=1, .C=-(vertice->y)};
        *hwbtab->NoBlackLine = {.A=std::sqrtf(3.0), .B=-1, .C=(vertice->y)};
        return hwbtab;
    }

    /* GCallback for drag gesture start for the hwb tab */
    static void drag_begin(GtkGestureDrag* gesture, float x, float y, HWBTab* tab) {
        GtkWidget* area = tab->content;
        int width=gtk_widget_get_width(area);
        int height=gtk_widget_get_height(area);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        niffie("drag"+std::to_string(dx)+' '+std::to_string(dy));
        Geometry::Point2 P = {.x=x, .y=y};
        Geometry::Polar2 polar = GeoCalc_2d::cartesian_to_polar(&P, tab->CentrePoint);
        if (polar.r <= tab->ring_outerradius) {
            ColorSpaces::HWB* current_hwb=g_new(ColorSpaces::HWB, 1);
            ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
            *current_hwb=Converter::rgb_to_hwb(&t);
            Geometry::Point2 coords;
            coords.x = x - tab->CentrePoint->x;
            coords.y = y - tab->CentrePoint->y;
            coords = GeoCalc_2d::rotate(&coords, -(current_hwb->h * 2 * M_PI));
            niffie("coords: "+std::to_string(coords.x)+' '+std::to_string(coords.y));
            if(polar.r >= tab->ring_innerradius){
                tab->drag_bar_scale = 1.5;
                tab->startx = x;
                tab->starty = y;
                current_hwb->h = std::fmodf(polar.angle + 0.5f * M_PI, (2 * M_PI)) / (2 * M_PI);
                t=Converter::hwb_to_rgb(current_hwb);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
            } else if(GeoCalc_2d::inside_triangle(&coords, tab->BlackPoint, tab->WhitePoint, tab->VividPoint)){
                niffie("in triangle");
                tab->drag_dot_scale=1.5;
                current_hue = current_hwb->h;
                tab->startx = x;
                tab->starty = y;
                current_hwb->w = GeoCalc_2d::distance(&coords, tab->NoWhiteLine) / tab->triangle_height;
                current_hwb->b = GeoCalc_2d::distance(&coords, tab->NoBlackLine) / tab->triangle_height;
                t=Converter::hwb_to_rgb(current_hwb);
                *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
                g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
            }
            g_free(current_hwb);
        }

        gtk_widget_queue_draw(area);
    }

    /* GCallback for drag gesture updates for the hwb tab */
    static void drag_update(GtkGestureDrag* gesture, float x, float y, HWBTab* tab) {
        GtkWidget* area = tab->content;
        float width=gtk_widget_get_width(area);
        float height=gtk_widget_get_height(area);
        double dx, dy;
        bool sth=gtk_gesture_drag_get_offset(gesture, &dx, &dy);
        niffie("hwb   drag "+std::to_string(dx)+' '+std::to_string(dy));
        x=dx; y=dy;
        x+=tab->startx;
        y+=tab->starty;
        ColorSpaces::HWB* current_hwb=g_new(ColorSpaces::HWB, 1);
        ColorSpaces::RGB t=_gdk_rgba_to_rgb(CURRENT_COLOR);
        *current_hwb=Converter::rgb_to_hwb(&t);
        niffie(std::to_string(tab->drag_dot_scale)+' '+std::to_string(tab->drag_bar_scale));
        tab->hwb_dragged_outside = false;
        if (tab->drag_dot_scale>1.0) {
            niffie("dragging the dot ----------------------------");
            Geometry::Point2 coords = {.x = x, .y = y};
            Geometry::Polar2 polar = GeoCalc_2d::cartesian_to_polar(&coords, tab->CentrePoint);
            coords.x = x - tab->CentrePoint->x;
            coords.y = y - tab->CentrePoint->y;
            coords = GeoCalc_2d::rotate(&coords, -(current_hwb->h * 2 * M_PI));
            if(!GeoCalc_2d::inside_triangle(&coords, tab->BlackPoint, tab->WhitePoint, tab->VividPoint)){
                tab->hwb_dragged_outside = true;
                float rmax = polar.r, rmin = 0, rcandidate;
                while(abs(rmax-rmin) > 0.05){
                    rcandidate = (rmin + rmax)/2.0f;
                    polar.r = rcandidate;
                    coords = GeoCalc_2d::polar_to_cartesian(&polar);
                    coords = GeoCalc_2d::rotate(&coords, -(current_hwb->h * 2 * M_PI));
                    if(GeoCalc_2d::inside_triangle(&coords, tab->BlackPoint, tab->WhitePoint, tab->VividPoint)){
                        rmin = rcandidate;
                    } else{
                        rmax = rcandidate;
                    }
                }
                polar.r = rmin;
                coords = GeoCalc_2d::polar_to_cartesian(&polar);
                coords = GeoCalc_2d::rotate(&coords, -(current_hwb->h * 2 * M_PI));
            }
            current_hwb->w = GeoCalc_2d::distance(&coords, tab->NoWhiteLine) / tab->triangle_height;
            current_hwb->b = GeoCalc_2d::distance(&coords, tab->NoBlackLine) / tab->triangle_height;
            current_hwb->h = current_hue;
            t=Converter::hwb_to_rgb(current_hwb);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
            g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
        } else if (tab->drag_bar_scale>1.0) {
            Geometry::Point2 coords = {.x = x, .y = y};
            Geometry::Polar2 polar = GeoCalc_2d::cartesian_to_polar(&coords, tab->CentrePoint);
            current_hwb->h = std::fmodf(polar.angle + 0.5f * M_PI, (2 * M_PI)) / (2 * M_PI);
            t=Converter::hwb_to_rgb(current_hwb);
            *CURRENT_COLOR=_rgb_to_gdk_rgba(&t);
            g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
        }
        g_free(current_hwb);
        niffie("trying to draw");
        gtk_widget_queue_draw(area);
    }

    /* GCallback for drag gesture end for the hwb tab */
    static void drag_end(GtkGestureDrag* gesture, float x, float y, HWBTab* tab) {
        GtkWidget* area = tab->content;
        int width=gtk_widget_get_width(area);
        int height=gtk_widget_get_height(area);
        tab->drag_dot_scale=1.0;
        tab->drag_bar_scale=1.0;
        tab->hwb_dragged_outside = false;
        double dx, dy;
        bool sth=gtk_gesture_drag_get_start_point(gesture, &dx, &dy);
        x=dx; y=dy;
        gtk_widget_queue_draw(area);
    }

    /* GtkDrawingAreaDrawFunc drawing function of the hwb tab 
     * from outside called by gtk_widget_queue_draw(drawing_area) */
    static void draw(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height, gpointer data) {
        niffie("kk");
        HWBTab* tab = (HWBTab*)data;
        // g_signal_emit_by_name(drawing_area, "color-change");
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
        if(tab->drag_dot_scale > 1.0){
            hue = current_hue;
            hwb_color->h = hue;
        }
        float hue_angle = hue * 2.0f * M_PI;
        niffie(std::to_string(hue_angle)+" ... "+std::to_string(hue));
        ColorSpaces::RGB8 pixcolor;
        //filling the buffer for the whole triangle picker base
        for(int j=0; j<height_;j++){
            pixel = buffer + (j*stride)/4;
            for(int i=0;i<width_;i++){
                coords->x = i; coords->y = j;
                coords->x -= tab->CentrePoint->x;
                coords->y -= tab->CentrePoint->y;
                polar = GeoCalc_2d::cartesian_to_polar(coords);
                *coords = GeoCalc_2d::rotate(coords, - hue_angle);
                if(polar.r <= tab->ring_outerradius and polar.r >= tab->ring_innerradius){
                    hwbpix->h = (polar.angle + M_PI * 0.5f) / (M_PI * 2.0f);
                    if (hwbpix->h < 0){
                        hwbpix->h += 1.0f;
                    }
                    hwbpix->w = 0; hwbpix->b = 0;
                    t = Converter::hwb_to_rgb(hwbpix);
                    pixcolor = Converter::float_to_int_rgb(&t);
                } else if(polar.r <= tab->triangle_height*2.0f/3.0f 
                    and GeoCalc_2d::inside_triangle(coords, tab->BlackPoint, tab->WhitePoint, tab->VividPoint)){
                    //calculate pixel color inside hwb triangle
                    hwbpix->h = hue;
                    hwbpix->w = (GeoCalc_2d::distance(coords, tab->NoWhiteLine) / tab->triangle_height);
                    hwbpix->b = (GeoCalc_2d::distance(coords, tab->NoBlackLine) / tab->triangle_height);
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
        cairo_arc(cr, tab->CentrePoint->x, tab->CentrePoint->y, tab->ring_outerradius+2.5, 
                hue_angle - 0.05f*tab->drag_bar_scale, hue_angle + 0.05f*tab->drag_bar_scale);
        cairo_arc_negative(cr, tab->CentrePoint->x, tab->CentrePoint->y, tab->ring_innerradius-2.5,
                hue_angle + 0.05f*tab->drag_bar_scale, hue_angle - 0.05f*tab->drag_bar_scale);
        cairo_close_path(cr);
        cairo_stroke(cr);
        cairo_new_path(cr);
        cairo_set_source_rgba(cr, innerborder->red, innerborder->green, innerborder->blue, innerborder->alpha);
        cairo_arc(cr, tab->CentrePoint->x, tab->CentrePoint->y, tab->ring_outerradius+1.3, 
                hue_angle - 0.04f*tab->drag_bar_scale, hue_angle + 0.04f*tab->drag_bar_scale);
        cairo_arc_negative(cr, tab->CentrePoint->x, tab->CentrePoint->y, tab->ring_innerradius-1.3,
                hue_angle + 0.04f*tab->drag_bar_scale, hue_angle - 0.04f*tab->drag_bar_scale);
        cairo_close_path(cr);
        cairo_stroke(cr);
        cairo_new_path(cr);

        if (hwb_color->w > 0.75) {
            std::swap(innerborder, outerborder);
        }

        //painting the dot inside the triangle
        Geometry::Point2 dot_centre = *tab->WhitePoint;
        Vector2 v(tab->WhitePoint, tab->VividPoint);
        t=_gdk_rgba_to_rgb(color);
        ColorSpaces::HSV hsv_color = Converter::rgb_to_hsv(&t);
        v = v.multiply(hsv_color.v);
        dot_centre = GeoCalc_2d::move(&dot_centre, &v);
        v = Vector2(tab->VividPoint, tab->BlackPoint);
        v = v.multiply((1 - hsv_color.s) * hsv_color.v);
        dot_centre = GeoCalc_2d::move(&dot_centre, &v);
        
        niffie("dot: "+std::to_string(dot_centre.x)+' '+std::to_string(dot_centre.y));
        dot_centre = GeoCalc_2d::rotate(&dot_centre, (- hue_angle) + M_PI*0.5f);
        niffie("dot: "+std::to_string(dot_centre.x)+' '+std::to_string(dot_centre.y));
        float dot_x = tab->CentrePoint->x + dot_centre.x;
        float dot_y = tab->CentrePoint->y - dot_centre.y;
        float dot_radius = 10.0f * tab->drag_dot_scale + 2; 
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

class Eyedropper {
private:
    GtkWidget* button; /* button activating the screen color picker (eyedropper) */
    GtkEventController* enter; /* event controller for capturing picking mode off 'enter' key */
    guint timeout; /* [ms] interval on which is the picking method called when active */
    gulong handler_id; /* 'enter' event controller handler id */

public:
    /** method for constructing aan Eyedropper
     * 
     * @param grid the parent grid on which shall the eyedropper button reside
     * @param button_name text to put on the button if icon not possible
     * @param icon_name name of the icon to be displayed on the button
     * @param grid_row row where the tile should be put
     * @param grid_col column where the tile should be put
     * @param width how many columns should the tile span
     * @param height how many rows should the tile span
     */
    static Eyedropper* Eyedropper_new(GtkGrid* grid, const char* button_name=NULL, const char* icon_name=NULL,
                                      int grid_row=0, int grid_col=0, int width=1, int height=1) {
        Eyedropper* eyedropper=g_new(Eyedropper, 1);
        //if icon is specified and available, use the icon
        eyedropper->button=gtk_toggle_button_new();
        gtk_button_set_can_shrink(GTK_BUTTON(eyedropper->button), TRUE);
        gtk_widget_set_hexpand(GTK_WIDGET(eyedropper->button), FALSE);
        if (icon_name != NULL and icon_exists(icon_name)) {
            gtk_button_set_icon_name(GTK_BUTTON(eyedropper->button), icon_name);
        }
        else if (button_name != NULL) {
            gtk_button_set_label(GTK_BUTTON(eyedropper->button), button_name);
        }
        g_signal_connect(GTK_TOGGLE_BUTTON(eyedropper->button), "toggled", G_CALLBACK(togglebutton), eyedropper);
        gtk_grid_attach(grid, eyedropper->button, grid_col, grid_row, width, height);
        eyedropper->enter=gtk_event_controller_key_new();
        return eyedropper;
    }

    GtkWidget* get_button(){ return this->button; } /* get the eyedropper button */
    GtkEventController* get_enter_controller(){ return this->enter; } /* get the event controller that captures 'enter' key */
    guint get_timeout(){ return this->timeout; } /* [ms] interval on which is the picking method called when active */
    gulong get_handler_id(){ return this->handler_id; } /* get 'enter' event controller handler id */

    /* set eyedropper button dimensions */
    void resize(int width, int height){
        gtk_widget_set_size_request(GTK_WIDGET(button), width, height);
    }

    /* deals with button toggling, activates/deactivates eyedropper*/
    static void togglebutton(GtkToggleButton* button, float x, float y, Eyedropper* eyedropper){
        bool button_is_active = gtk_toggle_button_get_active(button);
        if(button_is_active){
            eyedropper->handler_id=g_signal_connect_data(eyedropper->enter, "key-pressed", G_CALLBACK(eyedropper_end), eyedropper, on_closure_notify, G_CONNECT_SWAPPED);
            eyedropper->timeout = g_timeout_add(100, eyedropper_run, (Eyedropper*)eyedropper);
        } else{
            if (eyedropper->timeout != 0) {
                g_source_remove(eyedropper->timeout);
                eyedropper->timeout = 0;
                eyedropper_end(eyedropper, 0, 0);
            }
        }
        g_signal_emit_by_name(GTK_WIDGET(button), "color-change");
    }

    /* picker adminstrator repeatedly called by togglebutton() */
    static gboolean eyedropper_run(gpointer user_data) {
        getpixcolor();
        Eyedropper* eyedropper = (Eyedropper*) user_data;
        niffie("works!");
        g_signal_emit_by_name(GTK_WIDGET(eyedropper->button), "color-change");
        return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(eyedropper->button));
    }

    /* gets app out of screen color picking mode */
    static void eyedropper_end(Eyedropper* eyedropper, float x, float y) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(eyedropper->button), false);
        getpixcolor();
        g_signal_handler_disconnect(eyedropper->enter, eyedropper->handler_id);
        g_signal_emit_by_name(GTK_WIDGET(eyedropper->button), "color-change");
    }

    /* x11-specific method for getting the pixel color from under the pointer */
    static void getpixcolor() {
        Display* display=XOpenDisplay(NULL);
        Window root=DefaultRootWindow(display);
        // Get cursor position
        Window root_return, child_return;
        int root_x, root_y, win_x, win_y;
        unsigned int mask;
        XQueryPointer(display, root, &root_return, &child_return,
            &root_x, &root_y, &win_x, &win_y, &mask);

        // Get pixel at cursor position
        XImage* image=XGetImage(display, root, root_x, root_y, 1, 1, AllPlanes, ZPixmap);
        if (image) {
            unsigned long pixel=XGetPixel(image, 0, 0);
            XDestroyImage(image);
            unsigned int red, green, blue;
            red=(pixel >>16)^ ((pixel>>24)<<8);
            green=(pixel >> 8)^ ((pixel>>16)<<8);
            blue=pixel ^ ((pixel >> 8)<<8);
            niffie(std::to_string(red)+' '+std::to_string(green)+' '+std::to_string(blue));
            CURRENT_COLOR->red=(float)red / 255.0f;
            CURRENT_COLOR->green=(float)green / 255.0f;
            CURRENT_COLOR->blue=(float)blue / 255.0f;
        }
        XCloseDisplay(display);

    }

    /* utility for validating the existence of an icon in the theme */
    static bool icon_exists(const char* name){
        GdkDisplay* display = gdk_display_get_default();
        GtkIconTheme* theme = gtk_icon_theme_get_for_display(display);
        return (bool) gtk_icon_theme_has_icon(theme, name);
    }
};

class Textbox{
private:
    GtkWidget* entry; /* GtkEntry holding the editable input */
    GtkWidget* frame; /* frame containing both the label and entry */
    GtkWidget* field_name; /* title to be displayed alongside the entry; its label */
    const char* default_text; /* string the entry will default to if emptied / incorrectly filled */
    bool valid; /* holds validity status of the currrent field content */
    controllable_properties property; /* id of outside_obj property the text box influences */

public:    
    /** constructor of a Textbox
     * 
     * @param grid the GtkGrid object for the textbox to be thrown into
     * @param validator GCallback used to validate entry content to choose from: 
     *  valid_8bit - for 0-255 integer values /
     *  valid_0_to_1_float - for floating point in 0..1 range
     * @param fieldname label / title of the entry
     * @param placeholder text for the entry to default to when empty
     * @param buffer text different form placeholder that will only be displayed
     * until anything else is put into the entry
     * @param length length of the entry field (maximum input length)
     * @param prop controllable_properties id what the text box influences
     * @param grid_row row where the textbox should be put
     * @param grid_col column where the textbox should be put
     * @param width how many columns should the textbox span
     * @param height how many rows should the textbox span
    */
    static Textbox* Textbox_new (GtkGrid* grid, GCallback validator, const char* fieldname, const char* placeholder=NULL,
            const char* buffer=NULL, int length=3, controllable_properties prop=NO_CONTROL, int grid_row=0, int grid_col=0, int width=1, int height=1){
        Textbox* tbox = g_new(Textbox,1);
        int len = -1;
        if(buffer){
            len = ((string)buffer).size();
        }
        tbox->entry = gtk_entry_new_with_buffer(gtk_entry_buffer_new(buffer, len));
        tbox->frame = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        tbox->field_name = gtk_label_new(fieldname);
        gtk_editable_set_max_width_chars(GTK_EDITABLE(tbox->entry), length);
        gtk_box_append(GTK_BOX(tbox->frame), tbox->field_name);
        gtk_box_append(GTK_BOX(tbox->frame), tbox->entry);
        gtk_grid_attach(grid, tbox->frame, grid_col, grid_row, width, height);
        g_signal_connect(GTK_EDITABLE(tbox->entry), "changed", validator, tbox);
        g_signal_connect(GTK_CELL_EDITABLE(tbox->entry), "activate", G_CALLBACK(editing_done), tbox);
        g_signal_connect(GTK_EDITABLE(tbox->entry), "editing-done", G_CALLBACK(editing_done_2), tbox);
        // gtk_entry_set_overwrite_mode(GTK_ENTRY(tbox->entry), true);
        gtk_entry_set_placeholder_text(GTK_ENTRY(tbox->entry), placeholder);
        tbox->valid = true;
        tbox->default_text = placeholder;
        tbox->property=prop;
        niffie("got textbox");
        return tbox;
    }

    GtkWidget* get_entry(){ return entry; } /* get the editable text box */
    GtkWidget* get_frame(){ return frame; } /* get the textbox frame */
    GtkWidget* get_field_name(){ return field_name; } /* get the label of the entry */
    const char* get_default_text(){ return default_text; } /* get the placeholder text displayed when entry is empty */
    bool get_valid(){ return valid; } /* get the validity status of the input */
    controllable_properties get_controlled_id(){ return property; } /* get the id of the property influenced by the textbox */

    /* implements the update of controlled property when a valid textbox content edit happens */
    void set_controlled_property(auto value) {
        controllable_properties id = property;
        switch (id) {
        case CC_F_RED: {
            niffie(std::to_string(value)+' '+std::to_string(CURRENT_COLOR->red));
            if(CURRENT_COLOR->red!=value){
                CURRENT_COLOR->red = value;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_F_GREEN: {
            if(CURRENT_COLOR->green!=value){
                CURRENT_COLOR->green = value;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_F_BLUE: {
            if(CURRENT_COLOR->blue!=value){
                CURRENT_COLOR->blue = value;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_I_RED: {
            float val = (float)value / 255.0f;
            niffie(std::to_string(val)+' '+std::to_string(CURRENT_COLOR->red));
            if(CURRENT_COLOR->red != val){
                CURRENT_COLOR->red = val;
                g_signal_emit_by_name(entry, "color-change");
            }
        }
        case CC_I_GREEN: {
            float val = (float)value / 255.0f;
            if(CURRENT_COLOR->green != val){
                CURRENT_COLOR->green = val;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_I_BLUE: {
            float val = (float)value / 255.0f;
            if(CURRENT_COLOR->blue != val){
                CURRENT_COLOR->blue = val;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_ALPHA: {
            if(CURRENT_COLOR->alpha!=value){
                CURRENT_COLOR->alpha = value;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_HUE: {
            ColorSpaces::RGB rgb = _gdk_rgba_to_rgb(CURRENT_COLOR);
            ColorSpaces::HSV temp = Converter::rgb_to_hsv(&rgb);
            temp.h = value;
            ColorSpaces::RGB newcolor = Converter::hsv_to_rgb(&temp);
            if(rgb != newcolor){
                *CURRENT_COLOR = _rgb_to_gdk_rgba(&newcolor);
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        default:
            break;
        }
    }
    
    /* updates input box when sth else changes the value of controlled property */
    static void update_box_content(Textbox* tbox, gpointer data) {
        niffie("update triggered -----------------------------------------");
        if(!(tbox->valid) or gtk_entry_get_text_length(GTK_ENTRY(tbox->entry))==0){
            int bufflen = std::strlen(tbox->default_text);
            tbox->valid = true;
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(tbox->default_text, bufflen));
        }
        controllable_properties id = tbox->property;
        switch (id) {
        case CC_F_RED: {
            float value = CURRENT_COLOR->red;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(value));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(value);
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_F_GREEN: {
            float value = CURRENT_COLOR->green;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(value));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(value);
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_F_BLUE: {
            float value = CURRENT_COLOR->blue;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(value));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(value);
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_I_RED: {
            int value = std::round(CURRENT_COLOR->red * 255);
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(value));
            if(value == stoi(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(value);
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_I_GREEN: {
            int value = std::round(CURRENT_COLOR->green * 255);
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(value));
            if(value == stoi(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(value);
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_I_BLUE: {
            int value = std::round(CURRENT_COLOR->blue * 255);
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(value));
            if(value == stoi(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(value);
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_ALPHA: {
            float value = CURRENT_COLOR->alpha;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(value));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(value);
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_HUE: {
            ColorSpaces::RGB rgb = _gdk_rgba_to_rgb(CURRENT_COLOR);
            ColorSpaces::HSV temp = Converter::rgb_to_hsv(&rgb);
            float value = temp.h;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(value));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(value);
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        default:
            break;
        }
        gtk_widget_queue_draw(tbox->entry);
    }

    /* sets entry status to invalid and applies indicators in the ui */
    static void set_invalid(Textbox* tbox){
        tbox->valid = false;
        niffie("invalid input");
        const char* c = "invalid";
        gtk_widget_add_css_class(tbox->entry, c);
    }

    /* sets entry status to valid and applies indicators in the ui */
    static void set_valid(Textbox* tbox){
        tbox->valid = true;
        niffie("valid input");
        const char* c = "invalid";
        bool was_invalid = gtk_widget_has_css_class(tbox->entry, c);
        if(was_invalid){
            gtk_widget_remove_css_class(tbox->entry, c);
        }
    }

    /* checks if entry content is a valid 8-bit integer and updates entry status accordingly */
    static void valid_8bit(GtkWidget* entryfield, gpointer data){
        Textbox* field = (Textbox*) data;
        niffie("checking...");
        string text = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entryfield)));
        if(gtk_entry_get_text_length(GTK_ENTRY(field->entry))==0){
            set_valid(field);
            return;
        }
        for (char c: text){
            if (c<'0' or c>'9'){
                set_invalid(field);
                return;
            }
        }
        try {
            float value = std::stof(text);
            if(value == std::round(value)  and 0 <= value and value < 256){
                set_valid(field);
                niffie("current val: "+std::to_string((int)value));
                field->set_controlled_property((int)value);
            } else{
                set_invalid(field);
                niffie("floating point?");
            }
        }
        catch (const std::exception& e) {
            set_invalid(field);
        }

    }

    /* checks if entry content is a valid 0..1 float and updates entry status accordingly */
    static void valid_0_to_1_float(GtkWidget* entryfield, gpointer data){
        Textbox* field = (Textbox*) data;
        niffie("checking...");
        string text = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entryfield)));
        if(gtk_entry_get_text_length(GTK_ENTRY(field->entry))==0){
            set_valid(field);
            return;
        }
        for (char c: text){
            if ((c<'0' or c>'9') and c!='.'){
                set_invalid(field);
                return;
            }
        }
        try {
            float value = std::stof(text);
            if(0 <= value and value <= 1){
                set_valid(field);
                niffie("current val: "+std::to_string(value));
                field->set_controlled_property(value);
            } else{
                set_invalid(field);
                niffie("floating point?");
            }
        }
        catch (const std::exception& e) {
            set_invalid(field);
        }

    }

    /* signal handler for when the user tries to exit the entry field */
    static void editing_done(GtkWidget* entryfield, gpointer data){
        Textbox* field = (Textbox*)data;
        niffie("done?");
        g_signal_emit_by_name(GTK_EDITABLE(field->entry), "changed");
        if(!(field->valid) or gtk_entry_get_text_length(GTK_ENTRY(field->entry))==0){
            int bufflen = std::strlen(field->default_text);
            field->valid = true;
            gtk_entry_set_buffer(GTK_ENTRY(field->entry), gtk_entry_buffer_new(field->default_text, bufflen));

        }
        g_signal_emit_by_name(GTK_EDITABLE(field->entry), "editing-done");
    }
    
    /* cleans up after editing_done() when it does its job poorly */
    static void editing_done_2(GtkWidget* entryfield, gpointer data){
        Textbox* field = (Textbox*)data;
        niffie("finally!");
        if(!(field->valid) or gtk_entry_get_text_length(GTK_ENTRY(field->entry))==0){
            int bufflen = std::strlen(field->default_text);
            field->valid = true;
            gtk_entry_set_buffer(GTK_ENTRY(field->entry), gtk_entry_buffer_new(field->default_text, bufflen));
        }
    }
};

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget* window;
    GtkWidget* grid;
    GtkWidget* notebook;

    window=gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "color picker");
    g_signal_connect(window, "destroy", G_CALLBACK(close_window), GTK_WINDOW(window));
    grid=gtk_grid_new();

    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    niffie("begin ");

    //load the stylesheet
    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "styles.css");
    auto display = gdk_display_get_default();
    gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    //notebook with all of the color choosers 
    notebook=gtk_notebook_new();
    gtk_grid_attach(GTK_GRID(grid), notebook, 0, 0, 4, 3);

    niffie("begin ");
    CURRENT_COLOR=g_new(GdkRGBA, 1);
    niffie("a");
    CURRENT_COLOR->red=0.4;
    CURRENT_COLOR->green=0.1;
    CURRENT_COLOR->blue=0.8;
    CURRENT_COLOR->alpha=1;

    HSLTab* hsl_chooser = (HSLTab::HSLTabnew(GTK_NOTEBOOK(notebook), "HSL"));
    HSVTab* hsv_chooser = HSVTab::HSVTabnew(GTK_NOTEBOOK(notebook), "HSV");

    my_widget_signals[DRAW_ACTION_SIGNAL] = g_signal_new(
            "color-change",
            G_TYPE_FROM_CLASS(GTK_WIDGET_GET_CLASS(hsl_chooser->get_content())),
            G_SIGNAL_RUN_FIRST,     
            0, /* class offset for default handler */     
            nullptr, nullptr,     
            g_cclosure_marshal_VOID__STRING,     
            G_TYPE_NONE, /* return type */     
            1,    /* n_params */     
            G_TYPE_STRING 
    );
    HWBTab* hwb_triangle_chooser = HWBTab::HWBTabnew(GTK_NOTEBOOK(notebook), "HWB triangle");

    //color tile under the chooser
    niffie("middle ");
    Eyedropper* eyedropper = Eyedropper::Eyedropper_new(GTK_GRID(grid), "pick from screen", "color-select", 4, 0);
    eyedropper->resize(50,50);
    my_widget_signals[TOGGLE_PICKER_SIGNAL] = g_signal_new(
            "color-change",
            G_TYPE_FROM_CLASS(GTK_WIDGET_GET_CLASS(eyedropper->get_button())),
            G_SIGNAL_RUN_FIRST,     
            0, /* class offset for default handler */     
            nullptr, nullptr,     
            g_cclosure_marshal_VOID__STRING,     
            G_TYPE_NONE, /* return type */     
            1,    /* n_params */     
            G_TYPE_STRING 
    );

    GtkWidget* text_editables_grid;
    text_editables_grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), text_editables_grid, 1, 3, 3, 1);

    gtk_grid_set_row_spacing(GTK_GRID(text_editables_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(text_editables_grid), 10);

    Textbox* red_box = Textbox::Textbox_new(GTK_GRID(text_editables_grid), G_CALLBACK(Textbox::valid_8bit), "r", "0", "123", 3, CC_I_RED, 0, 0);
    Textbox* green_box = Textbox::Textbox_new(GTK_GRID(text_editables_grid), G_CALLBACK(Textbox::valid_8bit), "g", "0", "123", 3, CC_I_GREEN, 0, 1);
    Textbox* blue_box = Textbox::Textbox_new(GTK_GRID(text_editables_grid), G_CALLBACK(Textbox::valid_8bit), "b", "0", "123", 3, CC_I_BLUE, 0, 2);
    my_widget_signals[TEXTBOX_CC_CHANGED_SIGNAL] = g_signal_new(
            "color-change",
            G_TYPE_FROM_CLASS(GTK_WIDGET_GET_CLASS(red_box->get_entry())),
            G_SIGNAL_RUN_FIRST,     
            0, /* class offset for default handler */     
            nullptr, nullptr,     
            g_cclosure_marshal_VOID__STRING,     
            G_TYPE_NONE, /* return type */     
            1,    /* n_params */     
            G_TYPE_STRING 
    );

    
    ColorTile* tile = ColorTile::ColorTilenew(grid, CURRENT_COLOR, 50, 50, 0, 3, 1, 1);
    g_signal_connect_data(GTK_WIDGET(hsl_chooser->get_content()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_WIDGET(hsv_chooser->get_content()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_WIDGET(hwb_triangle_chooser->get_content()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_WIDGET(eyedropper->get_button()), "color-change", G_CALLBACK(update_nb), notebook, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_WIDGET(eyedropper->get_button()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    //red input box handlers
    g_signal_connect_data(GTK_EDITABLE(red_box->get_entry()), "color-change", G_CALLBACK(update_nb), notebook, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_EDITABLE(red_box->get_entry()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_EDITABLE(red_box->get_entry()), "editing-done", G_CALLBACK(unfocus), window, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_DRAWING_AREA(tile->get_tile()), "color-change", G_CALLBACK(Textbox::update_box_content), red_box, on_closure_notify, G_CONNECT_SWAPPED);
    //green input box handlers
    g_signal_connect_data(GTK_EDITABLE(green_box->get_entry()), "color-change", G_CALLBACK(update_nb), notebook, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_EDITABLE(green_box->get_entry()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_EDITABLE(green_box->get_entry()), "editing-done", G_CALLBACK(unfocus), window, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_DRAWING_AREA(tile->get_tile()), "color-change", G_CALLBACK(Textbox::update_box_content), green_box, on_closure_notify, G_CONNECT_SWAPPED);
    //blue input box handlers
    g_signal_connect_data(GTK_EDITABLE(blue_box->get_entry()), "color-change", G_CALLBACK(update_nb), notebook, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_EDITABLE(blue_box->get_entry()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_EDITABLE(blue_box->get_entry()), "editing-done", G_CALLBACK(unfocus), window, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_DRAWING_AREA(tile->get_tile()), "color-change", G_CALLBACK(Textbox::update_box_content), blue_box, on_closure_notify, G_CONNECT_SWAPPED);
    niffie("a ");
    gtk_window_set_child(GTK_WINDOW(window), grid);
    niffie("a ");
    gtk_window_present(GTK_WINDOW(window));
    niffie("a ");
}

int main(int argc, char** argv) {
    GtkApplication* app;
    int status;
    niffie("o ");
    app=gtk_application_new("colors.picker.app", G_APPLICATION_DEFAULT_FLAGS);
    niffie("o ");
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    niffie("o ");
    status=g_application_run(G_APPLICATION(app), argc, argv);
    niffie("o ");
    g_object_unref(app);
    niffie("o\n");
    return status;
}
