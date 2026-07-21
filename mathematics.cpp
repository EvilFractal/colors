#include<math.h>

class Math{
public:
    static float round(float number, float precision){
        float res = std::roundf(number/precision);
        return res*precision;
    }
};