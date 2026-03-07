# Some color utilities

<i> disclaimer: I frankly have no idea what I'm doing regarding this whole thing's structure </i>

## ColorSpaces tools 

```<colorspaces.h>``` provides some colorspace tools:
- color-holding types implemented in ```color_containers.h```: 
    - <code>struct RGB8 {int r, int g, int b}</code> – currently the only one holding hex-compatible (integer) rgb color value
    - <code>struct RGB {float r, float g, float b}</code> – the most versatile, preferred way to store colors

    and for their respective color space formats:
    - <code>struct HSV {float h, float s, float v}</code> 
    - <code> struct HSL {float h, float s, float l}  </code>
    - <code> struct HWB {float h, float w, float b} </code>

    additionally it allows for any of the color formats to gain alpha value:
    - <code> struct ALPHA <color_typename> {color_typename base_color, float a}</code>

    accessible as ```ColorSpaces::<typename>``` 
- ```Converter``` implemented in ```converter.cpp``` is a class of functions for color space conversions:<br>
    between color spaces:
    - ```HSL rgb_to_hsl (RGB *color)``` 
    - ```HSV rgb_to_hsv (RGB *color)``` 
    - ```HWB rgb_to_hwb (RGB *color)``` 
    - ```RGB hsl_to_rgb (HSL *color)``` 
    - ```RGB hsv_to_rgb (HSV *color)``` 
    - ```RGB hwb_to_rgb (HWB *color)``` 

    and within rgb color space:
    - ```RGB8 float_to_int_rgb (RGB *color)```
    - ```RGB int_to_float_rgb (RGB8 *color)```
    - ```std::string hex (RGB8 *color)```
    - ```RGB8 unhex (std::string *hex)```

    all called by ```Converter.<function_name>(<args>)```

everything should work when ```<colorspaces.h>``` gets included, as it does not have any dependencies outside of the standard c++ library

## ColorPicker utility

custom gtk-based color picker (no, the gtk built-in Gtk.ColorChooser was not used in the process) <br>
requires only gtk libraries (aside from ColorPicker's friends from here)<br>

functionality:
- \* hsl color picker
- \* hsv color picker
- \* hwb triangle-in-a-ring color picker
- \* basic rgb/hex editor maybe?
- \* eyedropper

\* not yet implemented

## Geometry module
will contain various geometry utilities primarily needed by ColorPicker, but it shall eventually consist of ```Geometry``` class (defined in ```<geometry.h>```) and its methods, dependent only on the standard c++ library and usable by itself <br>
current features:
- none -_-

## Miscellaneous
if needed, create some ```Painter``` class for painting events <br>
eyedropper is strictly a part of ColorPicker <br>
```colorspaces.h``` should remain the only include needed for color manipulation at all times, although if p.ex. ```Converter``` can be used as a standalone utility, it shall be given its own file as to keep it from dragging the whole color space utility pack along

ideas for the future:
- palettes support
- ColorSpaces.ColorMaps
- ColorSpaces color map / gradient -> rgb sequence generator using bezier/hermite splines based off given points' coordinates

