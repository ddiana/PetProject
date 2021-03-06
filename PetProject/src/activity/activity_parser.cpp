#include "activity/activity_parser.h"

using namespace std;
using namespace cv;

ActivityParser::ActivityParser(VideoCapture cap, string outputLocation, int numFramesToSkip , float minimumPercentActivity): cap(cap), outputLocation(outputLocation), numFramesToSkip(numFramesToSkip), minimumPercentActivity(minimumPercentActivity) 
{
	bgSub.set("nmixtures",1);  // one gaussian in the "mixture", don't need more than one
	frameCounter = 0 ;	
	minimumNumberFramesPerActivityGroup = ceil(100./numFramesToSkip);
	maximumGapInActivityGroup = ceil(30./numFramesToSkip);
	cap >> currentFrame;
	currentForeground = Mat::zeros(currentFrame.rows, currentFrame.cols, currentFrame.depth());
	currentForeground.copyTo(previousForeground);
	currentForeground.copyTo(relevantForegroundMask);
	activityGroupCounter = 0;
}
	
	
void ActivityParser::addCatModel(CatModel& cat)
{
	catModelVector.push_back(cat);

}
	
void ActivityParser::processActivityGroups()
{
	// needs lock from activity group mutex, which produces ActivityGroup objects
	boost::mutex::scoped_lock lock(activityGroupMutex);
	
	while (true)
	{
		// wait for activity queue to have stuff in it
		// get signal from runThroughCapture thread conditional variable
		
		while(activityGroupQueue.empty())
			activityGroupAvailable.wait(lock);
			
			
		ActivityGroup ag = activityGroupQueue.front();
			
		// save video clip
		std::string file_name;
		std::stringstream out;
		out << outputLocation << "/" << activityGroupCounter << ".avi";
		file_name = out.str();			
		ag.saveAsVideo(file_name, SAVE_FRAME );
		
		// save video clip for activity mask
		out.str("");
		out << outputLocation << "/" << activityGroupCounter << "_maks.avi";
		file_name = out.str();			
		ag.saveAsVideo(file_name, SAVE_MASK);
			
		// save text file with frame indices
		out.str("");
		out << outputLocation << "/" << activityGroupCounter << "_frame_numbers.txt";
		file_name = out.str();
		ag.saveFrameIndicesToTextFile(file_name);
			
		// test if cat is present in the foreground and track it.
		ag.testAgainstCatModel(catModelVector[0]);
		out.str("");
		out << outputLocation << "/" << activityGroupCounter << "_clip.avi";
		file_name = out.str();
		cout << file_name << endl;
		ag.saveAsVideo(file_name, SAVE_OUTPUT);
			
			
			
		activityGroupCounter++;			
			
		activityGroupQueue.pop();
		cout << "Thread 2: Processing activity group\n";
			
	}
	
}
	
	
void ActivityParser::runThroughCapture( )
/*  Read frames from capture... 
	Send them to extract foreground...
	Send foreground to check if there is activity...
*/
{	
	namedWindow("foreground",1);	
		
	for(;;)	
	{			
		// get next frame
	    for (size_t i=0; i < numFramesToSkip; i++)
	    {			   
		    cap >> currentFrame; // get a new frame from camera		     
	        if(!cap.isOpened())  // check if we succeeded
	        {
		        cout << "Finished capture frames...";	
		        exit(1);
		    }			        
	    }
		    
	    // if frame has activity, make activity frame object
		getForeground();
		//cout << "thread 1\n";
	    if (currentFrameHasEnoughMotion())
	    {
	    	cout << "ACTIVITY @ frame " << frameCounter << endl;
	    	ActivityFrame af(currentFrame, currentForeground, frameCounter);
	    	activityFrameVector.push_back(af); 
	    }
		    		    
	    // manage activity frame vector:
	    // if empty: leave it alone
	    // if latest frame was too far from the previous ones:
	    // 		- Make activity group from previous activities (if long enough), or discard them
	    manageActivityVector();
	    
	    // display
		imshow("foreground", currentFrame);
		    
		   
	    if(waitKey(30) >= 0) 
	    {
	    	tgroup.interrupt_all();
	    	break;
	    }
	}	
}
	
	
void ActivityParser::main()
{			
	// thread produced ActivityGroup objects
	tgroup.create_thread (boost::bind(&ActivityParser::runThroughCapture, this));
	// thread consumes ActivityGroup objects
	tgroup.create_thread (boost::bind(&ActivityParser::processActivityGroups, this));

	// wait for them
	tgroup.join_all();
	cout << "JOINED ALL THREADS\n";
}
	
	
void ActivityParser::manageActivityVector()
{
// manage activity frame vector:
// if empty: leave it alone
// if latest frame was too far from the previous ones:
// 		- Make activity group from previous activities (if long enough), or discard them
					
	if (activityFrameVector.size())
	{

		// if the current frame is not related to the last frame in the activity frame vector
		if ( frameCounter - activityFrameVector.back().frameIndex  > maximumGapInActivityGroup )
			// if the activity frame vector has enough frames in it to wrap up an acvitiy group:
			if ( activityFrameVector.size() > minimumNumberFramesPerActivityGroup )
			{
				

				boost::mutex::scoped_lock lock(activityGroupMutex);
				ActivityGroup  lastActivityGroup(activityFrameVector);
				activityGroupQueue.push(lastActivityGroup);
				cout << "Added activity group of size: " << lastActivityGroup.activityFrameVector.size() << endl;
				cout << "Activity group queue size: " << activityGroupQueue.size() << endl;
					
				activityGroupAvailable.notify_one();
					
				// after making the activity group, clear the activity frame vector:
				activityFrameVector.clear();
			}
			else
			{
				// if not, clear the acvity frame vector
					
				ActivityFrame lastActivityFrame = activityFrameVector.back();
				
				activityFrameVector.clear();
				activityFrameVector.push_back(lastActivityFrame);
			}
					
	}
	
}
	
bool ActivityParser::currentFrameHasEnoughMotion(){
// check is frame has enough activity:
// 15% of motion
	
	Scalar matSum = sum(relevantForegroundMask);	
 	bool hasActivity = matSum[0]> currentForeground.rows * currentForeground.cols * minimumPercentActivity;
	 	
 	//cout <<  hasActivity;
 	return hasActivity;
		  		
}
	
void ActivityParser::getForeground(){
/* Extract foreground using bg subtractor and morphological processing */
	
	boxFilter(currentFrame, currentFrame, currentFrame.depth(), Size(5,5));
	bgSub(currentFrame, currentForeground);	
		
	multiply(currentForeground , previousForeground, relevantForegroundMask);
				
	currentForeground.copyTo(previousForeground);	
	frameCounter++;			        
}
	


