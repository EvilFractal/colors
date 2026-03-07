// #include<string>

class ColorSpaces{
    public:
    struct RGB {  //most versatile, recommended for storing colors
        float r;
        float g;
        float b;
        float a=1.0;
    };

    struct RGB8{  //stores ints: compatible with hex
        int r;
        int g;
        int b;
        float a=1.0;
    };

    struct HSV{  //container for hsv color space -type colors
        float h;
        float s;
        float v;
        float a=1.0;
    };

    struct HSL{  //container for hsl color space -type colors
        float h;
        float s;
        float l;
        float a=1.0;
    };

    struct HWB{  //container for hwb color space -type colors
        float h;
        float w;
        float b;
        float a=1.0;
    };
};