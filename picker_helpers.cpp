#ifndef PICKER_HELPERS_MODULE
#define PICKER_HELPERS_MODULE

#include "colorpicker.h"

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
void resize_cb(GtkWidget* widget, int width, int height, gpointer data) {
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

/* utility function for passing update signal to a widget */
void update(GtkWidget* widget, gpointer data){
    gtk_widget_queue_draw(widget);
}

/* utility function for passing update signal to a notebook */
void update_nb(GtkNotebook* notebook, gpointer data){
    int active_page = gtk_notebook_get_current_page(notebook);
    gtk_widget_queue_draw(gtk_frame_get_child(GTK_FRAME(gtk_notebook_get_nth_page(notebook, active_page))));

}

/** dummy function to fill window closing callback
 * does nothing
 */
static void close_window(gpointer window) {
    return;
}


#endif
