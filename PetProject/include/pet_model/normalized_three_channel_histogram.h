#ifndef NORMALIZED_THREE_CHANNEL_HISTOGRAM
#define NORMALIZED_THREE_CHANNEL_HISTOGRAM

#include <opencv2/opencv.hpp>

class NormalizedThreeChannelHistogram
{
public:
	NormalizedThreeChannelHistogram(cv::MatND h1, cv::MatND h2, cv::MatND h3);
	NormalizedThreeChannelHistogram();
	void plotNormalizedColorHistogram();
	void setChannelHistograms( cv::MatND h1, cv::MatND h2, cv::MatND h3);

	cv::MatND hist_channel1;
	cv::MatND hist_channel2;
	cv::MatND hist_channel3;
};

#endif
