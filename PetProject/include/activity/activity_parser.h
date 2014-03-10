#ifndef ACTIVITY_PARSER_H
#define ACTIVITY_PARSER_H

#include <opencv2/opencv.hpp>
#include "pet_model/cat_model.h"
#include "activity/activity_frame.h"
#include "activity/activity_group.h"
#include <vector>
#include <queue>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/condition.hpp>


class ActivityParser
{

public:
	
	ActivityParser(cv::VideoCapture cap, std::string outputLocation, int numFramesToSkip = 5, float minimumPercentActivity = 0.15);
	void addCatModel(CatModel& cat);
	void processActivityGroups();
	void runThroughCapture( );
	void main();
	void manageActivityVector();
	bool currentFrameHasEnoughMotion();
	void getForeground();

	
private:
	int numFramesToSkip;
	int frameCounter;
	cv::BackgroundSubtractorMOG2 bgSub;
	cv::VideoCapture cap;
	std::vector<ActivityFrame> activityFrameVector;
	cv::Mat currentForeground;
	cv::Mat previousForeground;
	cv::Mat currentFrame;
	cv::Mat relevantForegroundMask;
	float minimumPercentActivity;
	std::string outputLocation;
	
	int minimumNumberFramesPerActivityGroup;
	int maximumGapInActivityGroup;
	std::queue<ActivityGroup> activityGroupQueue;
	
	boost::thread_group tgroup;
	boost::mutex activityGroupMutex;
	boost::condition activityGroupAvailable;
	int activityGroupCounter;
	
	std::vector<CatModel> catModelVector;
};

#endif

