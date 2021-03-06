#ifndef CAT_MODEL_H
#define CAT_MODEL_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include "normalized_two_channel_histogram_2d.h"
#include <vector>
#include <boost/regex.hpp>


class CatModel{

public:
	CatModel(std::string directory, std::string inputCatName);
	void pruneImageLocations();
	bool isMaskImage(std::string fileName);
	void loadImages();
	int getDirFileNames (std::string dir);
	void learnModel();
	void applyModelToImage(cv::Mat input, cv::Mat& backProject);
	void makeColorHistogramForModel();
	void getColorHistogramFromImage(cv::Mat input, cv::Mat mask, cv::MatND& histogram);
	void makeTextureDescriptor();

	
private:
	std::vector<std::string> imageLocations;
	std::vector<cv::Mat> images;
	std::vector<cv::Mat> fgMasks;
	std::string catName;
	boost::regex maskFileNameExpression;
	std::string imageDirectory;
	NormalizedTwoCHannelHistogram2d catColorHistogram;
	
};

#endif
