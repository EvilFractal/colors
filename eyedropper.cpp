#ifndef EYEDROPPER_MODULE
#define EYEDROPPER_MODULE

#include "colorpicker.h"
#include "picker_helpers.cpp"

class Eyedropper {
private:
    GtkWidget* button; /* button activating the screen color picker (eyedropper) */
    GtkEventController* enter; /* event controller for capturing picking mode off 'enter' key */
    guint timeout; /* [ms] interval on which is the picking method called when active */
    gulong handler_id; /* 'enter' event controller handler id */
    GdkRGBA* linked_color; /* pointer to the color the eyedropper interacts with */

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
    static Eyedropper* Eyedropper_new(GtkGrid* grid, GdkRGBA* color, const char* button_name=NULL, const char* icon_name=NULL,
                                      int grid_row=0, int grid_col=0, int width=1, int height=1) {
        Eyedropper* eyedropper=g_new(Eyedropper, 1);
        //if icon is specified and available, use the icon
        eyedropper->button=gtk_toggle_button_new();
        gtk_button_set_can_shrink(GTK_BUTTON(eyedropper->button), TRUE);
        gtk_widget_set_hexpand(GTK_WIDGET(eyedropper->button), FALSE);
        eyedropper->linked_color = color;
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
    GdkRGBA* get_linked_color(){ return linked_color; } /* get pointer to rhe color the chooser interacts with */

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
        Eyedropper* eyedropper = (Eyedropper*) user_data;
        eyedropper->getpixcolor();
        niffie("works!");
        g_signal_emit_by_name(GTK_WIDGET(eyedropper->button), "color-change");
        return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(eyedropper->button));
    }

    /* gets app out of screen color picking mode */
    static void eyedropper_end(Eyedropper* eyedropper, float x, float y) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(eyedropper->button), false);
        eyedropper->getpixcolor();
        g_signal_handler_disconnect(eyedropper->enter, eyedropper->handler_id);
        g_signal_emit_by_name(GTK_WIDGET(eyedropper->button), "color-change");
    }

    /* x11-specific method for getting the pixel color from under the pointer */
    void getpixcolor() {
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
            linked_color->red=(float)red / 255.0f;
            linked_color->green=(float)green / 255.0f;
            linked_color->blue=(float)blue / 255.0f;
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


#endif
