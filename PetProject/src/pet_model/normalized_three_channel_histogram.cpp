#include "pet_model/normalized_three_channel_histogram.h"

using namespace cv;

NormalizedThreeChannelHistogram::NormalizedThreeChannelHistogram(MatND h1, MatND h2, MatND h3):
		 hist_channel1(h1), hist_channel2(h2), hist_channel3(h3) {}
		
NormalizedThreeChannelHistogram::NormalizedThreeChannelHistogram() {}
	
void NormalizedThreeChannelHistogram::setChannelHistograms( MatND h1, MatND h2, MatND h3 ){
	h1.copyTo(hist_channel1);
	h2.copyTo(hist_channel2);
	h3.copyTo(hist_channel3);	
}
		
void NormalizedThreeChannelHistogram::plotNormalizedColorHistogram()
{
	
	int hist_h = 300;
	int hist_w = 400;
	int bin_w = cvRound( (double) hist_w/hist_channel1.rows );
	Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
		
	for( int i = 1; i < hist_channel1.rows; i++ )
	{
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(hist_channel1.at<float>(i-1)*hist_h) ) ,
						Point( bin_w*(i), hist_h - cvRound(hist_channel1.at<float>(i)*hist_h) ),
						Scalar( 255, 0, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(hist_channel2.at<float>(i-1)*hist_h) ) ,
						Point( bin_w*(i), hist_h - cvRound(hist_channel2.at<float>(i)*hist_h) ),
						Scalar( 0, 255, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(hist_channel3.at<float>(i-1)*hist_h) ) ,
						Point( bin_w*(i), hist_h - cvRound(hist_channel3.at<float>(i)*hist_h) ),
						Scalar( 0, 0, 255), 2, 8, 0  );
	}

	namedWindow("Color Histogram", CV_WINDOW_AUTOSIZE );
	imshow("Color Histogram", histImage );
	waitKey(0);
	
}

