#ifndef COLOR_TILE_MODULE
#define COLOR_TILE_MODULE

#include "colorpicker.h"
#include "picker_helpers.cpp"

class ColorTile;
void update(GtkWidget* widget, gpointer data);
void update_tile(ColorTile* tile, gpointer data);

class ColorTile {
private:
    GtkWidget* frame; /* contains the tile */
    GtkWidget* tile; /* rectangle of a specific color */
    GdkRGBA* color; /* color of the tile */
    GdkRGBA* followed_color; /* pointer to a color instance the tile should follow */

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
    static ColorTile * ColorTilenew(GtkWidget* parent, GdkRGBA* def_color, GdkRGBA* follow=NULL,
        int width=50, int height=50,
        int grid_column=0, int grid_row=0, int grid_vert_span=1, int grid_hz_span=1) {
        ColorTile * tile = g_new(ColorTile, 1);
        tile->color=g_new(GdkRGBA, 1);
        *(tile->color) =  *def_color;
        tile->frame=gtk_frame_new(NULL);
        tile->tile=gtk_drawing_area_new();
        tile->followed_color = follow;
        gtk_widget_set_size_request(GTK_WIDGET(tile->tile), width, height);
        gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(tile->tile), width);
        gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(tile->tile), height);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(tile->tile), paint_tile, tile, (GDestroyNotify)on_destroy_notify);
        g_signal_connect_after(GTK_DRAWING_AREA(tile->tile), "resize", G_CALLBACK(resize_cb), NULL);
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
    GdkRGBA* get_followed_color(){ return followed_color; } /* get the color instance the tile follows */

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

/* utility function for passing update signal to a tile */
void update_tile(ColorTile* tile, gpointer data){
    niffie("update_tile");
    if (tile->get_followed_color() != tile->get_color()){
        tile->set_color(tile->get_followed_color());
        niffie("-----------------------------------");
        g_signal_emit_by_name(GTK_DRAWING_AREA(tile->get_tile()), "color-change");
    }
    gtk_widget_queue_draw(tile->get_tile());
}



#endif