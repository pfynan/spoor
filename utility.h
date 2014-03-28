
#pragma once

#include <cv.h>
#include <boost/optional.hpp>

#include <cmath>

inline void cross(cv::Mat& img,cv::Point pt,int size=10) {
    using namespace cv;


    Scalar color = Scalar(0,0,255);
    int thick = 3;
    line(img,pt - Point(size/2,size/2),pt + Point(size/2,size/2),color,thick);
    line(img,pt - Point(-size/2,size/2),pt + Point(-size/2,size/2),color,thick);
}

inline int mod(int a, int b) {
    return ((a%b)+b)%b;
}

// Mean of 0; ||square_wave|| = sqrt(N)
inline float square_wave(int t) {
    return 4.0*std::floor(t/(2*M_PI)) - 2.0*std::floor(2.0*t/(2*M_PI)) + 1.0;
}

inline float bit_pattern(int i, char pattern,float bps,float sample_rate) {
    int N = sample_rate / bps * 8;
    float Tbit = sample_rate / bps;
    return (float) ((pattern >> ((int)floor(mod(i,N)/Tbit))) & 1);
}

inline std::vector<float> conv(std::vector<float> f, std::vector<float> g) {
    int N = f.size();
    std::vector<float> ret(N);
    fill(ret.begin(),ret.end(),0);
    for(int n = 0; n < N; ++n) {
        for(int m = 0; m < N; ++m) {
            ret[n] += f[m] * g[mod(n-m,N)];
        }
    }
    return ret;
}

