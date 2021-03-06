#ifndef ACTIVITY_GROUP_H
#define ACTIVITY_GROUP_H

#include "activity/activity_frame.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include "pet_model/cat_model.h"

#define SAVE_FRAME 1
#define SAVE_MASK 2
#define SAVE_OUTPUT 3

class ActivityGroup
{
public:
	ActivityGroup(std::vector<ActivityFrame> activityFrameVector);

	std::vector<ActivityFrame> activityFrameVector;
	std::vector<cv::Mat> outputClip;
	
	void testAgainstCatModel(CatModel cat);
	void trackObject(CatModel cat, cv::RotatedRect initRect, int initFrameNumber);
	void catDiscovery(CatModel cat, cv::RotatedRect& initRect, int& frameNumber);
	void saveAsVideo(std::string vidName, int saveType);
	void saveFrameIndicesToTextFile(std::string fileName);

};

#endif
