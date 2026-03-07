#include<math.h>
#include<string>
#include "color_containers.h"
using string = std::string;

class Converter:ColorSpaces{
    public:
    static string hex(RGB8 *color){
        string hex="#";
        for(int int_part: {color->r,color->g,color->b}){
            string str_part="";
            int res=int_part%16;
            if(res>=10){
                res-=10;
                str_part=(char)('a'+res-10);
            } else {
                str_part=(char)('0'+res);
            }
            int_part/=16;
            res=int_part%16;
            if(res>=10){
                res-=10;
                str_part=(char)('a'+res-10)+str_part;
            } else {
                str_part=(char)('0'+res)+str_part;
            }
            hex+=str_part;
        }
        return hex;
    }

    static RGB8 unhex(string *hex){
        RGB8 result;
        int nums[7];
        for(int i=1;i<6;i+=2){
            char cbig=(*hex)[i],csmall=(*hex)[i+1];
            if(cbig>='a'){
                nums[i]=16*(cbig-'a'+10);
            } else{ 
                nums[i]=16*(cbig-'0');
            } if(csmall>='a'){
                nums[i]+=csmall-'a'+10;
            } else{
                nums[i]+=csmall-'0';
            }
        }
        result.r = nums[1];
        result.g = nums[3];
        result.b = nums[5];
        return result;
    }

    static RGB8 float_to_int_rgb(RGB *color){
        RGB8 result;
        result.r=(int)(color->r*255);
        result.g=(int)(color->g*255);
        result.b=(int)(color->b*255);
        result.a=color->a;
        return result;
    }
    
    static RGB int_to_float_rgb(RGB8 *color){
        RGB result;
        result.r=(float)(color->r)/255.0;
        result.g=(float)(color->g)/255.0;
        result.b=(float)(color->b)/255.0;
        result.a=color->a;
        return result;
    }

    static HSL rgb_to_hsl(RGB *color){
        HSL result;
        // implemented from https://www.easyrgb.com/en/math.php
        float minimum = std::min(color->r,std::min(color->g,color->b));
        float maximum = std::max(color->r,std::max(color->g,color->b));
        float delta = maximum-minimum;
        result.l=(maximum+minimum)/2;
        if(delta==0){  //grey
            result.h=0;
            result.s=0;
        } else{
            if(result.l < 0.5){
                result.s=delta/(maximum+minimum);
            } else{
                result.s=delta/(2-(maximum+minimum));
            }

            float red_delta = ((maximum-color->r)/6 + delta/2)/delta;
            float green_delta = ((maximum-color->g)/6 + delta/2)/delta;
            float blue_delta = ((maximum-color->b)/6 + delta/2)/delta;

            if(color->r == maximum){
                result.h = blue_delta-green_delta;
            } else if(color->g == maximum){
                result.h = 1.0/3.0 + red_delta-blue_delta;
            } else{
                result.h = 2.0/3.0 + green_delta-red_delta;
            }
        }
        result.h=fmod(result.h, 1.0);
        result.a=color->a;
        return result;
    }
    
    static HSV rgb_to_hsv(RGB *color){
        HSV result;
        // implemented from https://www.easyrgb.com/en/math.php
        float minimum = std::min(color->r,std::min(color->g,color->b));
        float maximum = std::max(color->r,std::max(color->g,color->b));
        float delta = maximum-minimum;
        result.v=maximum;
        if(delta==0){  //grey
            result.h=0;
            result.s=0;
        } else{
            result.s = delta/maximum;

            float red_delta = ((maximum-color->r)/6 + delta/2)/delta;
            float green_delta = ((maximum-color->g)/6 + delta/2)/delta;
            float blue_delta = ((maximum-color->b)/6 + delta/2)/delta;

            if(color->r == maximum){
                result.h = blue_delta-green_delta;
            } else if(color->g == maximum){
                result.h = 1.0/3.0 + red_delta-blue_delta;
            } else{
                result.h = 2.0/3.0 + green_delta-red_delta;
            }
        }
        result.h=fmod(result.h, 1.0);
        result.a=color->a;
        return result;
    }

    static HWB rgb_to_hwb(RGB *color){
        HWB result;
        HSV midpoint=rgb_to_hsv(color);
        result.h=midpoint.h;
        result.w=(1.0 - midpoint.s)*midpoint.v;
        result.b=1.0 - midpoint.v;
        result.a=color->a;
        return result;
    }

    static RGB hsl_to_rgb(HSL *color){
        // implemented from Wikipedia: HSL_and_HSV article
        RGB result;
        result.r=hsl_helper_torgb(0.0, color);
        result.g=hsl_helper_torgb(8.0, color);
        result.b=hsl_helper_torgb(4.0, color);
        result.a=color->a;
        return result;
    }

    static RGB hsv_to_rgb(HSV *color){
        // implemented from Wikipedia: HSL_and_HSV article
        RGB result;
        result.r=hsv_helper_torgb(5.0, color);
        result.g=hsv_helper_torgb(3.0, color);
        result.b=hsv_helper_torgb(1.0, color);
        result.a=color->a;
        return result;
    }

    static RGB hwb_to_rgb(HWB *color){
        RGB result;
        HSV midpoint;
        midpoint.h=color->h;
        if(color->b + color->w >1.0){
            float total = color->b +color->w;
            color->b /= total;
            color->w /= total;
        }
        midpoint.v=1.0 - color->b;
        if(midpoint.v>0.0){
            midpoint.s=1.0 - color->w/midpoint.v;
        } else{
            midpoint.s=0;
        }
        midpoint.a=color->a;
        return hsv_to_rgb(&midpoint);
    }

    private:
    static float hsl_helper_torgb(float num, HSL *color){
        float a=color->s * std::min(color->l, 1-color->l);
        float k=fmod(num + color->h*12.0,12.0);
        return color->l - a * std::max(-1.0, std::min(1.0, std::min(k-3.0,9.0-k)));
    }

    static float hsv_helper_torgb(float num, HSV *color){
        float k=fmod(num + color->h*6.0,6.0);
        return color->v * (1.0 - color->s*std::max(0.0f, std::min(1.0f, std::min(k, 4.0f-k))));
    }
};

