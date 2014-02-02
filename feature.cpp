#include "feature.h"

#include <vector>
#include <algorithm>

using namespace std;
using namespace boost;
using namespace cv;

FeatureExtract::FeatureExtract() : bsub(500,5,0.6)
{

}


boost::optional<Point2f> FeatureExtract::operator()(Mat& image)
{
        cvtColor(image,image,CV_BGR2GRAY);
        normalize(image,image, 0, 255, NORM_MINMAX, CV_8UC1);


        //adaptiveThreshold(image,image,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,63,-150);
        
        bsub(image,image,0.002);

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

        vector<Moments> mu(contours.size() );
        for( size_t i = 0; i < contours.size(); i++ ) { 
            mu[i] = moments( contours[i], false );
        }

        ///  Get the mass centers:
        vector<Point2f> mc( contours.size() );
        for( size_t i = 0; i < contours.size(); i++ ) { 
            mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
        }

        /*for( size_t  i = 0; i< contours.size(); i++ )
        {
            Scalar color = Scalar(0,255,0);
            drawContours( disp, contours, i, color, 2, 8, hierarchy, 0, Point() );

        }*/

        auto blob = max_element(begin(mu),end(mu),[](Moments a, Moments b){ return a.m00 < b.m00; });

        Point2f measurement;

        if(blob != end(mu) && blob->m00 > 40) {
            measurement.x = blob->m10/blob->m00;
            measurement.y = blob->m01/blob->m00;
            return optional<Point2f>(measurement);

        } else {
            return optional<Point2f>();
        }

}

