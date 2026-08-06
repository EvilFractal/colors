#ifndef TEXT_BOX_MODULE
#define TEXT_BOX_MODULE

#include "colorpicker.h"
#include "picker_helpers.cpp"

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
    static Textbox* Textbox_new (GtkGrid* grid, GCallback validator, const char* fieldname, 
            const char* placeholder=NULL, const char* buffer=NULL, int length=3, controllable_properties prop=NO_CONTROL, 
            int grid_row=0, int grid_col=0, int width=1, int height=1){
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
        return tbox;
    }

    GtkWidget* get_entry(){ return entry; } /* get the editable text box */
    GtkWidget* get_frame(){ return frame; } /* get the textbox frame */
    GtkWidget* get_field_name(){ return field_name; } /* get the label of the entry */
    const char* get_default_text(){ return default_text; } /* get the placeholder text displayed when entry is empty */
    bool get_valid(){ return valid; } /* get the validity status of the input */
    controllable_properties get_controlled_id(){ return property; } /* get the id of the property influenced by the textbox */

    /* implements the update of controlled property when a valid textbox content edit happens */
    void set_controlled_property(std::any value_holder) {
        controllable_properties id = property;
        switch (id) {
        case CC_F_RED: {
            float value = std::any_cast<float>(value_holder);
            niffie(std::to_string(Math::round(value,0.001))+' '+std::to_string(CURRENT_COLOR->red));
            if(CURRENT_COLOR->red!=value){
                CURRENT_COLOR->red = value;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_F_GREEN: {
            float value = std::any_cast<float>(value_holder);
            if(CURRENT_COLOR->green!=value){
                CURRENT_COLOR->green = value;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_F_BLUE: {
            float value = std::any_cast<float>(value_holder);
            if(CURRENT_COLOR->blue!=value){
                CURRENT_COLOR->blue = value;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_I_RED: {
            int value = std::any_cast<int>(value_holder);
            float val = (float)value / 255.0f;
            niffie(std::to_string(val)+' '+std::to_string(CURRENT_COLOR->red));
            if(CURRENT_COLOR->red != val){
                CURRENT_COLOR->red = val;
                g_signal_emit_by_name(entry, "color-change");
            }
        }
        case CC_I_GREEN: {
            int value = std::any_cast<int>(value_holder);
            float val = (float)value / 255.0f;
            if(CURRENT_COLOR->green != val){
                CURRENT_COLOR->green = val;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_I_BLUE: {
            int value = std::any_cast<int>(value_holder);
            float val = (float)value / 255.0f;
            if(CURRENT_COLOR->blue != val){
                CURRENT_COLOR->blue = val;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_ALPHA: {
            float value = std::any_cast<float>(value_holder);
            if(CURRENT_COLOR->alpha!=value){
                CURRENT_COLOR->alpha = value;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_HEX3: {
            int value = std::any_cast<int>(value_holder);
            int CURRENT_COLOR_hex = std::round(CURRENT_COLOR->red * 255);
            CURRENT_COLOR_hex = 256 * CURRENT_COLOR_hex + std::round(CURRENT_COLOR->green * 255);
            CURRENT_COLOR_hex = 256 * CURRENT_COLOR_hex + std::round(CURRENT_COLOR->blue * 255);
            if(CURRENT_COLOR_hex!=value){
                CURRENT_COLOR->blue = (float)(value%256) /255;
                value/=256;
                CURRENT_COLOR->green = (float)(value%256) /255;
                value/=256;
                CURRENT_COLOR->red = (float)value /255;
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_HUE: {
            float value = std::any_cast<float>(value_holder);
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
        case CC_HSL_SATURATION: {
            float value = std::any_cast<float>(value_holder);
            ColorSpaces::RGB rgb = _gdk_rgba_to_rgb(CURRENT_COLOR);
            ColorSpaces::HSL temp = Converter::rgb_to_hsl(&rgb);
            temp.s = value;
            ColorSpaces::RGB newcolor = Converter::hsl_to_rgb(&temp);
            if(rgb != newcolor){
                *CURRENT_COLOR = _rgb_to_gdk_rgba(&newcolor);
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_LIGHTNESS: {
            float value = std::any_cast<float>(value_holder);
            ColorSpaces::RGB rgb = _gdk_rgba_to_rgb(CURRENT_COLOR);
            ColorSpaces::HSL temp = Converter::rgb_to_hsl(&rgb);
            temp.l = value;
            ColorSpaces::RGB newcolor = Converter::hsl_to_rgb(&temp);
            if(rgb != newcolor){
                *CURRENT_COLOR = _rgb_to_gdk_rgba(&newcolor);
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_HSV_SATURATION: {
            float value = std::any_cast<float>(value_holder);
            ColorSpaces::RGB rgb = _gdk_rgba_to_rgb(CURRENT_COLOR);
            ColorSpaces::HSV temp = Converter::rgb_to_hsv(&rgb);
            temp.s = value;
            ColorSpaces::RGB newcolor = Converter::hsv_to_rgb(&temp);
            if(rgb != newcolor){
                *CURRENT_COLOR = _rgb_to_gdk_rgba(&newcolor);
                g_signal_emit_by_name(entry, "color-change");
            }
            break;
        }
        case CC_VALUE: {
            float value = std::any_cast<float>(value_holder);
            ColorSpaces::RGB rgb = _gdk_rgba_to_rgb(CURRENT_COLOR);
            ColorSpaces::HSV temp = Converter::rgb_to_hsv(&rgb);
            temp.v = value;
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
            niffie(buffer+' '+std::to_string(Math::round(value,0.001)));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(Math::round(value,0.001));
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_F_GREEN: {
            float value = CURRENT_COLOR->green;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(Math::round(value,0.001)));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(Math::round(value,0.001));
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_F_BLUE: {
            float value = CURRENT_COLOR->blue;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(Math::round(value,0.001)));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(Math::round(value,0.001));
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
            niffie(buffer+' '+std::to_string(Math::round(value,0.001)));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(Math::round(value,0.001));
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_HEX3: {
            int value = std::round(CURRENT_COLOR->red * 255);
            value = 256 * value + std::round(CURRENT_COLOR->green * 255);
            value = 256 * value + std::round(CURRENT_COLOR->blue * 255);
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(value));
            if(value == stoi(buffer, 0, 16)){
                niffie("no change");
                break;
            }
            ColorSpaces::RGB8 col = {.b=(value%256)};
            value/=256;
            col.g = value%256;
            value/=256;
            col.r = value;
            buffer = Converter::hex(&col).substr(1,6);
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_HUE: {
            ColorSpaces::RGB rgb = _gdk_rgba_to_rgb(CURRENT_COLOR);
            ColorSpaces::HSV temp = Converter::rgb_to_hsv(&rgb);
            float value = temp.h;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(Math::round(value,0.001)));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(Math::round(value,0.001));
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_HSL_SATURATION: {
            ColorSpaces::RGB rgb = _gdk_rgba_to_rgb(CURRENT_COLOR);
            ColorSpaces::HSL temp = Converter::rgb_to_hsl(&rgb);
            float value = temp.s;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(Math::round(value,0.001)));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(Math::round(value,0.001));
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_LIGHTNESS: {
            ColorSpaces::RGB rgb = _gdk_rgba_to_rgb(CURRENT_COLOR);
            ColorSpaces::HSL temp = Converter::rgb_to_hsl(&rgb);
            float value = temp.l;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(Math::round(value,0.001)));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(Math::round(value,0.001));
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_HSV_SATURATION: {
            ColorSpaces::RGB rgb = _gdk_rgba_to_rgb(CURRENT_COLOR);
            ColorSpaces::HSV temp = Converter::rgb_to_hsv(&rgb);
            float value = temp.s;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(Math::round(value,0.001)));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(Math::round(value,0.001));
            int buffer_size = buffer.size();
            gtk_entry_set_buffer(GTK_ENTRY(tbox->entry), gtk_entry_buffer_new(buffer.c_str(), buffer_size));
            break;
        }
        case CC_VALUE: {
            ColorSpaces::RGB rgb = _gdk_rgba_to_rgb(CURRENT_COLOR);
            ColorSpaces::HSV temp = Converter::rgb_to_hsv(&rgb);
            float value = temp.v;
            string buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(tbox->entry)));
            niffie(buffer+' '+std::to_string(Math::round(value,0.001)));
            if(value == stof(buffer)){
                niffie("no change");
                break;
            }
            buffer = std::to_string(Math::round(value,0.001));
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
                niffie("current val: "+std::to_string(Math::round(value,0.001)));
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

    /* checks if entry content is a valid 24-bit hexadecimal and updates entry status accordingly */
    static void valid_24bit_hexadecimal(GtkWidget* entryfield, gpointer data){
        Textbox* field = (Textbox*) data;
        niffie("checking...");
        string text = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entryfield)));
        if(gtk_entry_get_text_length(GTK_ENTRY(field->entry))==0){
            set_valid(field);
            return;
        } else if(gtk_entry_get_text_length(GTK_ENTRY(field->entry))!=6){
            set_invalid(field);
            return;
        }
        for (int i=0;i<text.size();i++){
            char c = text[i];
            if ((c<'0' or c>'9') and (c<'A' or c>'F')){
                if(c>='a' and c<='f'){
                    text[i]=c-32;
                } else{
                    set_invalid(field);
                    return;
                }
            }
        }
        try {
            int value = std::stoi(text, 0, 16);
            set_valid(field);
            niffie("current val: "+std::to_string(value));
            field->set_controlled_property(value);
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


#endif
