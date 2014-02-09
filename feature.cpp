#include "utility.h"
#include "feature.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

#include <highgui.h>

using namespace std;
using namespace boost;
using namespace cv;

FeatureExtract::FeatureExtract(Size size) : bsub(500,5,0.6), buffer(2),
    frame_size(size)
{

    r = 1;
    fs = 25;
    w = 1.0/fs * 2.0 * M_PI;


    filter_a = {2.0*r*cos(w),-r*r};
}


boost::optional<Point2f> FeatureExtract::operator()(Mat& image)
{
    cvtColor(image,image,CV_BGR2GRAY);
    image.convertTo(image,CV_64FC1);
    //normalize(image,image, 0, 1, NORM_MINMAX, CV_32FC1);


    //adaptiveThreshold(image,image,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,63,-150);

    //bsub(image,image,0.002);


    //image /= 255.0;
    //image -= 0.5;

    image /= 50.0;

    if(!buffer.empty())
        image = inner_product(begin(buffer),end(buffer),begin(filter_a), image);

    
    buffer.push_front(image.clone()); // Oh honey. This is going to churn the heap.


    //image *= 255.0;
    


    image.convertTo(image,CV_8UC1);


    return getBiggestBlob(image);

}

