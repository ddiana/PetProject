#include "activity/activity_frame.h"


ActivityFrame::ActivityFrame(cv::Mat frameIn, cv::Mat frameMaskIn, int frameIndex) : frameIndex(frameIndex) {
	
	//Mat element = getStructuringElement(MORPH_RECT ,  Size( 25,25 ));
	//dilate( frameMaskIn, frameMaskIn, element );
	frameMaskIn.copyTo(frameMask);
	frameIn.copyTo(frame);
}
	

