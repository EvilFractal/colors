#ifndef GEOMETRY_CONTAINERS_TWO_DIMENSIONS_HEADER
#define GEOMETRY_CONTAINERS_TWO_DIMENSIONS_HEADER

class Geometry{
public:
    struct Point2{
        float x;
        float y;
    };

    struct Polar2{
        float angle;
        float r;
    };

    struct LineGeneral2{
        float A;
        float B;
        float C;
    };
};

#endif