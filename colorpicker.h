#ifndef COLOR_PICKER_HEADER
#define COLOR_PICKER_HEADER

#include<gtk/gtk.h>
#include<gdk/gdk.h>
#include<cairo.h>
#include<iostream>
#include<math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include<cstring>
#include<any>
#include<vector>
#include<iomanip>
#include "colorspaces.h"
#include "geometry.h"

#define DEBUG true

GdkRGBA* CURRENT_COLOR;

static cairo_surface_t* surface=NULL;

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
    CC_HSL_SATURATION, CC_LIGHTNESS, /* CURRENT_COLOR hsl saturation, lightness */
    CC_HSV_SATURATION, CC_VALUE, /* CURRENT_COLOR hsv saturation, value */
    CC_HEX3, /* CURRENT_COLOR r,g,b by hex */
    CC_HEX4, /* CURRENT_COLOR r,g,b,a by hex */
};

float current_hue=0.0;

/* convert ColorSpaces::RGB color format to GdkRGBA */
GdkRGBA _rgb_to_gdk_rgba(ColorSpaces::RGB* color);

/* convert GdkRGBA color format to ColorSpaces::RGB */
ColorSpaces::RGB _gdk_rgba_to_rgb(GdkRGBA* color);

/* convenience function for printing stuff */
void niffie(std::string message);

/* clear the surface */
void clear_surface(GdkRGBA* color);

/* Create a new surface of the appropriate size to store our scribbles */
void resize_cb(GtkWidget* widget, int width, int height, gpointer data);

/** dummy function to fill 'GClosureNotify' field 
 * as it is required but does nothing
*/
static void on_closure_notify(gpointer data, GClosure *closure);

/* 'GDestroyNotify', destroys given data */
static void on_destroy_notify(gpointer data);

static void unfocus(GtkWindow* window, gpointer data);

/* utility function for passing update signal to a widget */
void update(GtkWidget* widget, gpointer data);

/* utility function for passing update signal to a notebook */
void update_nb(GtkNotebook* notebook, gpointer data);
/** dummy function to fill window closing callback
 * does nothing
 */
static void close_window(gpointer window);



#endif