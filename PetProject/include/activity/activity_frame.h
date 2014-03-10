#ifndef ACTIVITY_FRAME_H 
#define ACTIVITY_FRAME_H

#include "opencv2/opencv.hpp"

class ActivityFrame
{
public:
	cv::Mat frame;
	cv::Mat frameMask;
	int frameIndex;
	
	ActivityFrame(cv::Mat frameIn, cv::Mat frameMaskIn, int frameIndex); 
	
};

#endif
