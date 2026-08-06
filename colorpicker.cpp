#include "colorpicker.h"
#include "status_area.cpp"
#include "picker_helpers.cpp"
#include "color_tile.cpp"
#include "chooser_tabs.cpp"
#include "eyedropper.cpp"
#include "textbox.cpp"

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

    HSLTab* hsl_chooser = (HSLTab::HSLTabnew(GTK_NOTEBOOK(notebook), CURRENT_COLOR, "HSL"));
    HSVTab* hsv_chooser = HSVTab::HSVTabnew(GTK_NOTEBOOK(notebook), CURRENT_COLOR, "HSV");

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
    HWBTab* hwb_triangle_chooser = HWBTab::HWBTabnew(GTK_NOTEBOOK(notebook), CURRENT_COLOR, "HWB triangle");

    //color tile under the chooser
    niffie("middle ");
    Eyedropper* eyedropper = Eyedropper::Eyedropper_new(GTK_GRID(grid), CURRENT_COLOR, "pick from screen", "color-select", 4, 0);
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
    Textbox* hex_box = Textbox::Textbox_new(GTK_GRID(text_editables_grid), G_CALLBACK(Textbox::valid_24bit_hexadecimal), "#", "000000", "123abc", 6, CC_HEX3, 1, 0, 2);
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

    StatusBar* sbar = new StatusBar;
    sbar = StatusBar::StatusBar_new(10, "");
    ItemPack rgb_status = ItemPack(0, 21, "rgb (, , ) ", false);
    rgb_status.set_nth_child(0, Item(5,3,CC_I_RED, "1"));
    rgb_status.set_nth_child(10, Item(7,3,CC_I_GREEN, "11"));
    rgb_status.set_nth_child(-1, Item(9,3,CC_I_BLUE, "123"));
    ItemPack hsv_status = ItemPack(0, 25, "hsv (, , )", false);
    hsv_status.set_nth_child(0, Item(5,5,CC_HUE, "1"));
    hsv_status.set_nth_child(10, Item(7,5,CC_HSV_SATURATION, "11"));
    hsv_status.set_nth_child(-1, Item(9,5,CC_VALUE, "123"));
    ItemPack hex_status = ItemPack(0, 9, "# ", false);
    hex_status.set_nth_child(-1, Item(1, 6, CC_HEX3, "000000", false));
    sbar->set_nth_child(-1, rgb_status);
    sbar->set_nth_child(-1, hex_status);
    niffie("packed first one!");
    sbar->set_nth_child(-1, hsv_status);
    StatusBar::update(sbar);
    gtk_grid_attach(GTK_GRID(grid), sbar->get_frame(), 0, 7, 6, 1);
    niffie("status bar ready...");

    
    ColorTile* tile = ColorTile::ColorTilenew(grid, CURRENT_COLOR, CURRENT_COLOR, 50, 50, 0, 3, 1, 1);
    g_signal_connect_data(GTK_WIDGET(hsl_chooser->get_content()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_WIDGET(hsv_chooser->get_content()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_WIDGET(hwb_triangle_chooser->get_content()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_WIDGET(eyedropper->get_button()), "color-change", G_CALLBACK(update_nb), notebook, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_WIDGET(eyedropper->get_button()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    niffie("signals...");
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
        niffie("signals...");
    //blue input box handlers
    g_signal_connect_data(GTK_EDITABLE(blue_box->get_entry()), "color-change", G_CALLBACK(update_nb), notebook, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_EDITABLE(blue_box->get_entry()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_EDITABLE(blue_box->get_entry()), "editing-done", G_CALLBACK(unfocus), window, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_DRAWING_AREA(tile->get_tile()), "color-change", G_CALLBACK(Textbox::update_box_content), blue_box, on_closure_notify, G_CONNECT_SWAPPED);
    //color hex input box handlers
    g_signal_connect_data(GTK_EDITABLE(hex_box->get_entry()), "color-change", G_CALLBACK(update_nb), notebook, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_EDITABLE(hex_box->get_entry()), "color-change", G_CALLBACK(update_tile), tile, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_EDITABLE(hex_box->get_entry()), "editing-done", G_CALLBACK(unfocus), window, on_closure_notify, G_CONNECT_SWAPPED);
    g_signal_connect_data(GTK_DRAWING_AREA(tile->get_tile()), "color-change", G_CALLBACK(Textbox::update_box_content), hex_box, on_closure_notify, G_CONNECT_SWAPPED);

    //status bar signal handlers
    g_signal_connect_data(GTK_DRAWING_AREA(tile->get_tile()), "color-change", G_CALLBACK(StatusBar::update), sbar, on_closure_notify, G_CONNECT_SWAPPED);
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
