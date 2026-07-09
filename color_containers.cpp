#include "color_containers.h"


bool operator==(const ColorSpaces::RGB& lhs, const ColorSpaces::RGB& rhs) {
    return (lhs.a == rhs.a and lhs.r == rhs.r and lhs.g == rhs.g and lhs.b == rhs.b);
}
bool operator!=(const ColorSpaces::RGB& lhs, const ColorSpaces::RGB& rhs) {
    return !(lhs == rhs);
}