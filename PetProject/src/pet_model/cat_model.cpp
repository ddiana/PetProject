#include "pet_model/cat_model.h"
#include <dirent.h>
#include <sys/stat.h>

using namespace std;
using namespace cv;

CatModel::CatModel(string directory, string inputCatName)
{
	catName = inputCatName;
	imageDirectory = directory;
		
	cout << "-------------------- Loading model for cat " << catName << " -------------------- " << endl;
		
	maskFileNameExpression.assign ("(.*)(_result_mask.png)");
	// get names of files in directory
	getDirFileNames(imageDirectory);
	// make list of file locations, including directory path, for those file which contain original image and mask
	pruneImageLocations();
	// load the images at these pruned locations
	loadImages();
	learnModel();
		
		
	//for (int i = 0; i < imageLocations.size(); i++)
	//	cout << imageLocations[i] << endl;
		
}
	
void CatModel::pruneImageLocations(){
// Keep on the file names which are relevant.

// Takes as input a vector of files located in imageDirectory
// Returns a vector of file paths to the original images that also have a mask, and to the masks
// in the order: image, mask, image, mask, image, mask.. etc
	
	vector<string> prunedImageLocations;
	boost::smatch regexMatch;
			
	for (int i =0; i< imageLocations.size(); i++)
	{
		bool foundFile = false; // this image is a mask and has the original file
		string originalImageFileName; // pair of the mask image
			
		// if this is a result segmentation
		if (boost::regex_search (imageLocations[i],regexMatch, maskFileNameExpression)) {
				

			stringstream s;
			s << imageDirectory << "/"<< regexMatch[1] << ".png";
			originalImageFileName = s.str();
			
			cout << "\nFile "<< imageLocations[i] << " is a segmentation file. \n   Checking if original file, " << regexMatch[1] <<".png or .jpg, exists: ";
				
			struct stat st;
			// check if original image is there:
			if ( !stat(originalImageFileName.c_str() ,&st)){
				foundFile = true;
				}
			else 
			{
			// if it does not, try .jpg extension
				s.str("");
				s << imageDirectory<< "/"<< regexMatch[1] << ".jpg";
				originalImageFileName = s.str();
				cout << originalImageFileName<< "   ..... ";
				if ( !stat(originalImageFileName.c_str() ,&st) )
					foundFile = true;
			}					

			
			if (foundFile)
			{
				cout << "Found!\n";
				// if file is a mask and its original image was found...
				prunedImageLocations.push_back(originalImageFileName);
				s.str("");
				s << imageDirectory << "/" <<imageLocations[i];
				prunedImageLocations.push_back(s.str());
			}
			else
			{
				cout << "NOT FOUND.\n";
			}
		}
	}
			
	// copy pruned locations into the original vector
	imageLocations.clear();
	for (int i = 0; i < prunedImageLocations.size(); i++)
	{
		imageLocations.push_back(prunedImageLocations[i]);
		//cout << prunedImageLocations[i] << endl;
	}
		
}
	
bool CatModel::isMaskImage(string fileName){
	//cout << "checking " << fileName << " ";
	bool isMask = regex_match (fileName, maskFileNameExpression);
	//cout << isMask << endl;
	return isMask;
}

void CatModel::loadImages() {
// Load the images and the masks
	
cout << "\n\nLoading the images:\n";

	for (int i = 0; i<imageLocations.size(); i++)
	{
		Mat im;
		im = imread(imageLocations[i]);
		if (isMaskImage(imageLocations[i]))
		{	
			cout << "Loading \"" << imageLocations[i] << "\" as MASK.\n"; 
			fgMasks.push_back(im);
		}
		else
		{
			cout << "Loading \"" << imageLocations[i] << "\" as IMAGE.\n";
			images.push_back(im);
		}
	}	
}
	
	
int CatModel::getDirFileNames (string dir){
// Get all the files in the given diretory
	DIR *dp;
	struct dirent *dirp;
	if((dp  = opendir(dir.c_str())) == NULL) {
	    cout << "Error opening " << dir << endl;
	    return 1;
	}
		while ((dirp = readdir(dp)) != NULL) {
	    imageLocations.push_back(string(dirp->d_name));
	}
	closedir(dp);
	return 0;
}

void CatModel::learnModel()
{
// hist equalize within mask
// get color histogram, within mask 
// get texture descriptor within mask 
// repeat for different scales: cat is 15%, 25%, 35% of the image 
			
	makeColorHistogramForModel();
	makeTextureDescriptor();
	

}
	
void CatModel::applyModelToImage(Mat input, Mat& backProject){
// apply the model to a given image+mask pair and return the results

	Mat inputImage;
	input.copyTo(inputImage);
	int channels[] = {0, 1};
	float ch1Ranges[] = {0,180};  // hue
	float ch2Ranges[] = {0,255};  // saturation
	const float* ranges[] = {ch1Ranges, ch2Ranges};
		
	cvtColor(inputImage, inputImage, CV_RGB2HSV);
	calcBackProject(&inputImage, 1, channels, catColorHistogram.colorHistogram, backProject, ranges, 1., true);
		
}
	
void CatModel::makeColorHistogramForModel()
{
// make the NormalizedThreeChannelHistogram object, with three 1-D histograms for every channel
// get cummulative histogram for all the input images, only in the cat region.

		
	
	MatND colorHistogram;
	for (int i = 0; i < images.size(); i++)
	{
		Mat bgMask = Mat::ones(fgMasks[i].rows, fgMasks[i].cols, CV_8UC3);
		bgMask = 255*bgMask - fgMasks[i];
		getColorHistogramFromImage(images[i], fgMasks[i], colorHistogram);	
		 
	
	}
		
	normalize( colorHistogram, colorHistogram, 0, 255, NORM_MINMAX, -1, Mat() );
	catColorHistogram.setChannelHistograms(colorHistogram);
	catColorHistogram.plotNormalizedColorHistogram();	

	//normalize( colorHistogram) ;
		
		
		
	
}
	

	
void CatModel::getColorHistogramFromImage(Mat input, Mat mask, MatND& histogram)
{	
// get 1-D histogram for every channel in the image
	
	float ch1Ranges[] = {0,180};  // hue
	float ch2Ranges[] = {0,255};  // saturation
	Mat image;
	input.copyTo(image);
	
	const float* ranges[] = {ch1Ranges, ch2Ranges};
		
	int ch1Bins = 90;
	int ch2Bins = 127;
	int histSize [] = { ch1Bins, ch2Bins};
	int channels[] = {0, 1};
				
	// prepare image
	cvtColor(image, image, CV_RGB2HSV);
					
	// split mask in 3 channels (need a 1channel image)
	vector<Mat> mask_planes;
	split( mask, mask_planes );
			
	// histogram for every channel
	calcHist( &image, 1, channels, mask_planes[0], histogram, 2, histSize, ranges, true, true );
			
	//cout << "image #" << i << ": " << hist2 << endl;		
	
}

	
	
	
void CatModel::makeTextureDescriptor()
{
	
	
	
}
	

