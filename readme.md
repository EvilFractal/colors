# Some color utilities

<i> disclaimer: I frankly have no idea what I'm doing regarding this whole thing's structure </i>

## ColorSpaces tools 

```<colorspaces.h>``` provides some colorspace tools:
- color-holding types implemented in ```color_containers.h```: 
    - <code>RGB8 {int r, int g, int b, float a=1.0}</code> – currently the only one holding hex-compatible (integer) rgb color value
    - <code>RGB {float r, float g, float b, float a=1.0}</code> – the most versatile, preferred way to store colors

    and for their respective color space formats:
    - <code>HSV {float h, float s, float v, float a=1.0} </code> 
    - <code>HSL {float h, float s, float l, float a=1.0} </code>
    - <code>HWB {float h, float w, float b, float a=1.0} </code>

    accessible as ```ColorSpaces::<typename>```, all colors have a hidden floating-point alpha value one needs not be concerned about, for it's set to full coverage by default. However, shall it prove useful, the option of changing alpha remains.
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
    - ```std::string hex (RGB8 *color)``` – note that it ignores the color's alpha value
    - ```RGB8 unhex (std::string *hex)```

    all called by ```Converter.<function_name>(<args>)```

everything should work when ```<colorspaces.h>``` gets included, as it does not have any dependencies outside of the standard c++ library

## ColorPicker utility

custom gtk-based color picker (no, the gtk built-in Gtk.ColorChooser was not used in the process) <br>
requires only gtk libraries (aside from ColorPicker's friends from here)<br>

functionality:
- hsl color picker
- hsv color picker
- \* hwb triangle-in-a-ring color picker
- \* basic rgb/hex editor maybe?
- \* eyedropper

\* not yet implemented

## Geometry module
will contain various geometry utilities primarily needed by ColorPicker, but it shall eventually consist of ```Geometry``` class (accessible through ```<geometry.h>```) and its methods, dependent only on the standard c++ library and usable by itself <br>
current features:
- geometrical structures defined in ```<geometry-containers.h>```:
    - ```Point2 {float x, float y}``` – a two-dimensional point
    - ```LineGeneral2 {float A, float B, float C}``` – a line with standard two-dimensional coordinates described by its general equation of form ```A*x + B*y + C = 0```

    accessible as ```Geometry::<typename>```
- two-dimensional geometry utilities implemented in ```<geometry-2d.cpp>```:
    - ```distance```
        1. ```float (Point2* P, LineGeneral2* k)``` – calculates the distance between a given point an a line in general form
        2. ```float (Point2* P, Point2* Q)``` – calculates the distance between two points
    - ```float f_radians (float fullcircle_fraction)``` – converts a floating-point fraction of a full circle, p.ex. a color's hue, into its equivalent in radians
    - ```Point2 rotate (Point2* P, float angle, Point2* Centre=Point0_0)``` – returns P's coordinates after the rotation by ```angle``` around the ```Centre``` point

    all called by ```GeoCalc_2d.<function_name>(<args>)```
    
    and constant:
    - ```Point2 Point0_0 {x = 0, y = 0}```

## Miscellaneous
if needed, create some ```Painter``` class for painting events <br>
eyedropper is strictly a part of ColorPicker <br>
```colorspaces.h``` should remain the only include needed for color manipulation at all times, although if p.ex. ```Converter``` can be used as a standalone utility, it shall be given its own file as to keep it from dragging the whole color space utility pack along

ideas for the future:
- palettes support
- ColorSpaces.ColorMaps
- ColorSpaces color map / gradient -> rgb sequence generator using bezier/hermite splines based off given points' coordinates

