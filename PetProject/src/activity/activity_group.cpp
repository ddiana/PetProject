#include "activity/activity_group.h"
#include <fstream>

using namespace cv;
using namespace std;

ActivityGroup::ActivityGroup(vector<ActivityFrame> activityFrameVector): activityFrameVector(activityFrameVector) {
	for (int i = 0; i < activityFrameVector.size(); i++)
	{
		Mat frameCopy;
		activityFrameVector[i].frame.copyTo(frameCopy);		
		outputClip.push_back(frameCopy);	
	}
}


	
void ActivityGroup::testAgainstCatModel(CatModel cat)
// check if "Cat" is in the clip, and track it.
{
		
	Rect trackWindow;
	RotatedRect initRect ;	
	int initFrameNumber;
		
		
	catDiscovery(cat, initRect, initFrameNumber);
		
	trackObject(cat, initRect, initFrameNumber);
	cout << "finished tracking " << endl;
				
}
	
		
void ActivityGroup::trackObject(CatModel cat, RotatedRect initRect, int initFrameNumber)
{
	Rect trackWindow = initRect.boundingRect();
	Rect previousTrackWindow = trackWindow;
	
	for (int i = initFrameNumber; i<activityFrameVector.size(); i++){
		
		Mat backProject(activityFrameVector[i].frame.rows, activityFrameVector[i].frame.cols, CV_64F);
		cat.applyModelToImage(activityFrameVector[i].frame, backProject);
			
		Mat mask;
			
		// shadow pixels are marked 127, fg pixels are marked 225
		compare( activityFrameVector[i].frameMask, 0, mask, CMP_GT);
			
		Mat element = getStructuringElement(MORPH_RECT ,  Size( 3,3 ));
		erode( mask, mask, element );
		element = getStructuringElement(MORPH_RECT ,  Size( 25,25 ));
		dilate( mask, mask, element );
		
			
		// get probrability of pixel being cat
		backProject.copyTo(backProject, activityFrameVector[i].frameMask);
			
		// only keeping fg area
		Mat maskedBackProject;
		multiply(backProject, mask/255., maskedBackProject);

		RotatedRect trackBox = CamShift(maskedBackProject, trackWindow,
                                   TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
		if( trackWindow.area() <= 1 )
		{
			// if nothing found, make trackwindow larger than the previous track window
			int extraSpace = int(MIN( previousTrackWindow.width, previousTrackWindow.height ) /2);
				
			trackWindow = Rect( previousTrackWindow.x - extraSpace, previousTrackWindow.y - extraSpace,
								previousTrackWindow.width + extraSpace*2, 
								previousTrackWindow.height + extraSpace*2);
		}

		// add ellipse to clip output
		ellipse( outputClip[i], trackBox, Scalar(0,0,255), 3, CV_AA );
        			
	}	
}
	

void ActivityGroup::catDiscovery(CatModel cat, RotatedRect& initRect, int& frameNumber)
// find the largest connected component in mask, set a rectangle around it.
{
	// find the max contour: what frame it's in, what index it has, how large is it
	int maxContourSum = 0;
	int maxContourIndex = 0;
	frameNumber = 0;
	vector<vector<Point> > maxContours;
	vector<Vec4i> maxHierarchy;
		
		
	Mat temp;
	
	for (int i = 0; i < 3; i++)
	// search for cat in the first 3 frames
	{	
		Mat backProject(activityFrameVector[i].frame.rows, activityFrameVector[i].frame.cols, CV_64F);
		cat.applyModelToImage(activityFrameVector[i].frame, backProject);
			
		Mat mask;
			
		// shadow pixels are marked 127, fg pixels are marked 225
		compare( activityFrameVector[i].frameMask, 0, mask, CMP_GT);
			
		Mat element = getStructuringElement(MORPH_RECT ,  Size( 3,3 ));
		erode( mask, mask, element );
		element = getStructuringElement(MORPH_RECT ,  Size( 25,25 ));
		dilate( mask, mask, element );
		
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
			
		findContours( mask, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	
		// find the largest contour 
		for (int j = 0; j < contours.size(); j++)
		{
			Mat temp = Mat::zeros(mask.size(), CV_8UC3);
			Scalar color(1,1,1);
			drawContours( temp, contours, j, color, CV_FILLED, 8, hierarchy );

			Scalar matSum = sum(temp);	
				
			// compares also to previous max's. if anything better found... replace it.
	 		if (matSum[0] > maxContourSum)
	 		{
	 			// set the largest contour information
	 			maxContourSum = matSum[0];
	 			maxContourIndex = j;
	 			frameNumber = i;
	 			maxContours = contours;
	 			maxHierarchy = hierarchy;
	 		}		
	 		cout << matSum[0] << " at " << maxContourIndex<< " at frame " << frameNumber << endl;
		}
	}
		
	// prepare the largest contour:
	// prepare the input to track:
	initRect = minAreaRect(Mat(maxContours[maxContourIndex]));
	
	// Draw contour in the output clip at the corresponding frame
	Scalar color(0,255,255);
	drawContours( outputClip[frameNumber], maxContours, maxContourIndex,color, CV_FILLED, 8, maxHierarchy );
		
	//imwrite ("test_contour.png", outputClip[frameNumber]);
	//cout << "final:" << maxContourSum << " at " << maxContourIndex << " at  frame " << frameNumber << endl;
}
	
	
void ActivityGroup::saveAsVideo(string vidName, int saveType)
{
	bool isColor = !(saveType == SAVE_MASK);
	VideoWriter outputVideo (	vidName, CV_FOURCC('D','I','V','X'),   10, activityFrameVector[0].frame.size(), isColor);
		
	if (!outputVideo.isOpened())
		cerr << "Could not save video";
	
	for (int i = 0; i<activityFrameVector.size(); i++){
		if (saveType == SAVE_MASK) // save as 
			outputVideo << activityFrameVector[i].frameMask;
		if (saveType == SAVE_FRAME)
			outputVideo << activityFrameVector[i].frame;
		if (saveType == SAVE_OUTPUT)
			outputVideo << outputClip[i];
		//stringstream out;
		//out << i << ".jpg";
		//string s = out.str();
		//imwrite(s, activityFrameVector[i].frame); 
	}	
}
	
void ActivityGroup::saveFrameIndicesToTextFile(string fileName)
{
	ofstream textFile;
	textFile.open(fileName.c_str());
	if (textFile)
		for (int i =0; i< activityFrameVector.size(); i++)
			textFile << activityFrameVector[i].frameIndex << endl;
	else
		cerr << "Could not write to file. " << fileName <<".\n";
				
	textFile.close();
}


