#ifndef CHOOSER_TABS_MODULE
#define CHOOSER_TABS_MODULE

#include "colorpicker.h"
#include "picker_helpers.cpp"

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
    GdkRGBA* linked_color; /* pointer to the color the chooser interacts with */

public:
    /** method for constructing a HSLTab
     * 
     * @param notebook the notebook of which the page shall it be
     * @param tab_name string to display as the tab name
     * @param width page width in pixels
     * @param height page height in pixels
     */
    static HSLTab* HSLTabnew(GtkNotebook* notebook, GdkRGBA* color, const char* tab_name, int width=400, int height=400) {
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
        hsltab->linked_color = color;
        hsltab->starty = 0;
        hsltab->startx = 0;
        hsltab->drag_bar_scale = 1.0;
        hsltab->drag_dot_scale = 1.0;
        hsltab->hsl_dragged_farright = false;
        return hsltab;
    }

    GdkRGBA* get_linked_color(){ return linked_color; } /* get pointer to rhe color the chooser interacts with */

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
            ColorSpaces::RGB t=_gdk_rgba_to_rgb(tab->linked_color);
            *current_hsl=Converter::rgb_to_hsl(&t);
            if (height/10 <= y and height*6/10 >= y) {
                tab->drag_dot_scale=2.0;
                tab->startx=x;
                tab->starty=y;
                current_hsl->h=(x - width/20)*10 / (width*9);
                current_hue=current_hsl->h;
                current_hsl->s=(y - height/10)*2 / height;
                t=Converter::hsl_to_rgb(current_hsl);
                *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
                g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
                niffie("drag-start"+std::to_string(tab->startx)+' '+std::to_string(tab->starty));
            } else if (height*7/10 <= y and height*8/10 >= y) {
                tab->drag_bar_scale=1.5;
                tab->startx=x;
                tab->starty=y;
                current_hsl->l=(x - width/20)*10 / (width*9);
                t=Converter::hsl_to_rgb(current_hsl);
                *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
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
        ColorSpaces::RGB t=_gdk_rgba_to_rgb(tab->linked_color);
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
            *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
            g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
        } else if (tab->drag_bar_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            y=height*7.5/10;
            current_hsl->l=(bound_x - width/20)*10 / (width*9);
            t=Converter::hsl_to_rgb(current_hsl);
            *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
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
        *color=*(tab->linked_color);
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
    GdkRGBA* linked_color; /* pointer to the color the chooser interacts with */

public:
    /** metho, needed to preserod for constructing a HSVTab
     * 
     * @param notebook the notebook of which the page shall it be
     * @param tab_name string to display as the tab name
     * @param width page width in pixels
     * @param height page height in pixels
     */
    static HSVTab* HSVTabnew(GtkNotebook* notebook, GdkRGBA* color, const char* tab_name,
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
        hsvtab->linked_color = color;
        hsvtab->starty = 0;
        hsvtab->startx = 0;
        hsvtab->drag_bar_scale = 1.0;
        hsvtab->drag_dot_scale = 1.0;
        hsvtab->hsv_dragged_farright = false;
        return hsvtab;
    }

    GdkRGBA* get_linked_color(){ return linked_color; } /* get pointer to rhe color the chooser interacts with */

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
            ColorSpaces::RGB t=_gdk_rgba_to_rgb(tab->linked_color);
            *current_hsv=Converter::rgb_to_hsv(&t);
            if (height/10 <= y and height*6/10 >= y) {
                tab->drag_dot_scale=2.0;
                tab->startx=x;
                tab->starty=y;
                current_hsv->h=(x - width/20)*10 / (width*9);
                current_hue=current_hsv->h;
                current_hsv->s=(y - height/10)*2 / height;
                t=Converter::hsv_to_rgb(current_hsv);
                *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
                g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
                niffie("drag-start"+std::to_string(tab->startx)+' '+std::to_string(tab->starty));
            } else if (height*7/10 <= y and height*8/10 >= y) {
                tab->drag_bar_scale=1.5;
                tab->startx=x;
                tab->starty=y;
                current_hsv->v=(x - width/20)*10 / (width*9);
                t=Converter::hsv_to_rgb(current_hsv);
                *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
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
        ColorSpaces::RGB t=_gdk_rgba_to_rgb(tab->linked_color);
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
            *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
            g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
        } else if (tab->drag_bar_scale>1.0) {
            float bound_x=std::min(width*19/20.0f, std::max(width/20.0f, x));
            y=height*7.5/10;
            current_hsv->v=(bound_x - width/20)*10 / (width*9);
            t=Converter::hsv_to_rgb(current_hsv);
            *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
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
        HSVTab* tab = (HSVTab*)data;
        GdkRGBA* color=g_new(GdkRGBA, 1);
        *color=*(tab->linked_color);
        cairo_set_source_surface(cr, surface, 0, 0);
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1);
        cairo_paint(cr);

        niffie(std::to_string(cairo_get_reference_count(cr)));
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
    GdkRGBA* linked_color; /* pointer to the color the chooser interacts with */

public:
    /** method for constructing a HWBTab
     * 
     * @param notebook the notebook of which the page shall it be
     * @param tab_name string to display as the tab name
     * @param width page width in pixels
     * @param height page height in pixels
     */
    static HWBTab* HWBTabnew(GtkNotebook* notebook, GdkRGBA* color, const char* tab_name,
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
        hwbtab->linked_color = color;
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

    GdkRGBA* get_linked_color(){ return linked_color; } /* get pointer to rhe color the chooser interacts with */

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
            ColorSpaces::RGB t=_gdk_rgba_to_rgb(tab->linked_color);
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
                *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
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
                *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
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
        ColorSpaces::RGB t=_gdk_rgba_to_rgb(tab->linked_color);
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
            *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
            g_signal_emit_by_name(GTK_DRAWING_AREA(tab->content), "color-change");
        } else if (tab->drag_bar_scale>1.0) {
            Geometry::Point2 coords = {.x = x, .y = y};
            Geometry::Polar2 polar = GeoCalc_2d::cartesian_to_polar(&coords, tab->CentrePoint);
            current_hwb->h = std::fmodf(polar.angle + 0.5f * M_PI, (2 * M_PI)) / (2 * M_PI);
            t=Converter::hwb_to_rgb(current_hwb);
            *(tab->linked_color)=_rgb_to_gdk_rgba(&t);
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
        *color=*(tab->linked_color);
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


#endif
