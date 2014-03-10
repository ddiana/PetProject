#ifndef NORMALIZED_TWO_CHANNEL_HISTOGRAM_2D_H
#define NORMALIZED_TWO_CHANNEL_HISTOGRAM_2D_H

#include <opencv2/opencv.hpp>

class NormalizedTwoCHannelHistogram2d
{
public:
	NormalizedTwoCHannelHistogram2d(cv::MatND hist);
	NormalizedTwoCHannelHistogram2d();
	void setChannelHistograms( cv::MatND hist );
	void plotNormalizedColorHistogram();
	cv::MatND colorHistogram;
};

#endif
