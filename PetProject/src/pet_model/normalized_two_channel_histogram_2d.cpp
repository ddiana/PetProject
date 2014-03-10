#include "pet_model/normalized_two_channel_histogram_2d.h"

using namespace cv;

NormalizedTwoCHannelHistogram2d::NormalizedTwoCHannelHistogram2d(MatND hist): colorHistogram(hist) {}
		
NormalizedTwoCHannelHistogram2d::NormalizedTwoCHannelHistogram2d() {}

void NormalizedTwoCHannelHistogram2d::setChannelHistograms( MatND hist ){
	hist.copyTo(colorHistogram);
}


void NormalizedTwoCHannelHistogram2d::plotNormalizedColorHistogram(){

	double maxVal=0;
	minMaxLoc(colorHistogram, 0, &maxVal, 0, 0);

	int scale = 4;
	int sbins = colorHistogram.cols;
	int hbins = colorHistogram.rows;
	Mat histImg = Mat::zeros(sbins*scale, hbins*scale, CV_8UC3);

	for( int h = 0; h < hbins; h++ )
		for( int s = 0; s < sbins; s++ )
		{
			float binVal = colorHistogram.at<float>(h, s);
			int intensity = cvRound(binVal*255/maxVal);
			rectangle( histImg, Point(h*scale, s*scale),
						Point( (h+1)*scale - 1, (s+1)*scale - 1),
						Scalar::all(intensity),
						CV_FILLED );
		}

	namedWindow( "H-S Histogram", 1 );
	imshow( "H-S Histogram", histImg );
	waitKey();
	
}


