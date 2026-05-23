#include<math.h>
#include "geometry-containers.h"

Geometry::Point2 Point0_0 = {.x=0, .y=0};

class Vector2 :Geometry{
public:
    float x;
    float y;

    Vector2(Geometry::Point2* P, Geometry::Point2* Q){
        x = (Q->x - P->x);
        y = (Q->y - P->y);
    }

    Vector2(float x_coord, float y_coord){
        x = x_coord;
        y = y_coord;
    }

    Vector2 multiply(float a){
        return Vector2(this->x * a, this->y * a);
    }
}; 


class GeoCalc_2d:Geometry{
public:
    static float distance(Point2* P, LineGeneral2* k){
        float distance = (k->A * P->x) + (k->B * P->y) + k->C;
        distance = std::abs(distance) / std::sqrtf(std::powf(k->A, 2) + std::powf(k->B, 2));
        return distance;
    }

    static float distance(Point2* P, Point2* Q){
        float distance = std::sqrtf(std::powf(P->x - Q->x, 2) + std::powf(P->y - Q->y, 2));
        return distance;
    }

    //converts an angle from 'fraction of a full circle' to 'radians', f_ to allow for future deg->rad
    static float f_radians(float fullcircle_fraction){
        float result = fullcircle_fraction * 2 * M_PI;
        return result;
    }

    static Polar2 cartesian_to_polar(Point2* P, Point2* Centre=&Point0_0){
        Polar2 Q;
        Point2 relative;
        relative.x = P->x - Centre->x;
        relative.y = P->y - Centre->y;
        Q.angle = std::atan2f(relative.y, relative.x);
        Q.r = distance(P, Centre);
        return Q;
    }

    static Point2 polar_to_cartesian(Polar2* P, Point2* Centre=&Point0_0){
        Point2 Q;
        Q.x = P->r * std::cosf(P->angle);
        Q.y = P->r * std::sinf(P->angle);
        Q.x += Centre->x;
        Q.y += Centre->y;
        return Q;
    }

    static Point2 rotate(Point2* P, float angle, Point2* Centre=&Point0_0){
        Point2 Q;
        Point2 relative;
        relative.x = P->x - Centre->x;
        relative.y = P->y - Centre->y;
        Q.x = relative.x * std::cos(angle) - relative.y * std::sin(angle);
        Q.y = relative.y * std::cos(angle) + relative.x * std::sin(angle);
        Q.x += Centre->x;
        Q.y += Centre->y;
        return Q;
    }

    //check if point P is inside the ABC triangle
    static bool inside_triangle(Point2* P, Point2* A, Point2* B, Point2* C){
        //compute the cross products of each side and its start-to-P vectors
        //if all of the same sign, the point lies inside - 'same side' of each edge vector
        float ABxAP = (B->x - A->x)*(P->y - A->y) - (B->y - A->y)*(P->x - A->x);
        float BCxBP = (C->x - B->x)*(P->y - B->y) - (C->y - B->y)*(P->x - B->x);
        float CAxCP = (A->x - C->x)*(P->y - C->y) - (A->y - C->y)*(P->x - C->x);
        return ((ABxAP>=0 and BCxBP>=0 and CAxCP>=0) or (ABxAP<=0 and BCxBP<=0 and CAxCP<=0));
    }

    static float get_x(LineGeneral2* k, float y){
        if(k->A == 0) return INFINITY;
        return ( - (k->B*y + k->C) / k->A);
    }

    static float get_y(LineGeneral2* k, float x){
        if(k->B == 0) return INFINITY;
        return ( - (k->A*x + k->C) / k->B);
    }

    static LineGeneral2 move(LineGeneral2* k, float orthogonal_distance){
        float cosAlpha = std::cosf(std::atan2f(-k->B, k->A));
        if(std::abs(cosAlpha)<0.001) return *k;
        LineGeneral2 m = *k;
        m.C += orthogonal_distance / cosAlpha;
        return m;
    }

    static Point2 intersection_point(LineGeneral2* k, LineGeneral2* m){
        if(k->A * (m->B) - k->B * (m->A) == 0) return Point2{.x=INFINITY, .y=INFINITY};
        Point2 P;
        if(m->A == 0){
            P.y = - m->C / m->B;
            P.x = (m->C * (k->B))/(m->B * (k->A)) - k->C / k->A;
        }
        else{
            P.y = (k->C * (m->A) - (k->A * (m->C))) / (k->A * (m->B) - (k->B * (m->A)));
            P.x = - (m->B * P.y) / m->A - (m->C / m->A);
        }
        return P;
    }

    static Point2 move(Point2* P, Vector2* v){
        Point2 Q;
        Q.x = P->x + v->x;
        Q.y = P->y + v->y;
        return Q;
    }
};