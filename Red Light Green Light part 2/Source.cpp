#include <iostream>
#include <opencv2/core/utility.hpp>
#include "opencv2/cudaobjdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc.hpp"


float detectScaleFactor = 1.05;
cv::Size padding = cv::Size(16, 16);
cv::Size winStride = cv::Size(8, 8);

using namespace cv;

HOGDescriptor hog;

-(void)setupHOG {
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
}

- (NSArray*)hogDetect:(UIImage*)source {

    cv::Mat input = [self cvMatFromUIImage : source];

    cv::Mat colorConvertedImage;
    cv::cvtColor(input, colorConvertedImage, COLOR_BGR2RGB);

    std::vector<cv::Rect> found;
    std::vector<double> weights;

    hog.detectMultiScale(colorConvertedImage, found, 0, winStride, padding, detectScaleFactor, 2);

    float threshold = 0.5;

    std::vector<cv::Rect> reducedRectangles;

    nms(found, reducedRectangles, threshold);

    NSMutableArray* result = @[].mutableCopy;
    for (size_t i = 0; i < reducedRectangles.size(); i++)
    {
        [result addObject : [NSValue valueWithCGRect : CGRectMake(reducedRectangles[i].x, reducedRectangles[i].y, reducedRectangles[i].width, reducedRectangles[i].height)] ] ;
    }

    found.clear();
    weights.clear();
    reducedRectangles.clear();

    return result;
}