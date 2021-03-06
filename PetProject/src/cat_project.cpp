#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include "activity/activity_parser.h"
#include "pet_model/cat_model.h"

using namespace cv;
using namespace std;



int main(int argc, char *argv[])
{
	string usage;
	
	usage = "Usage: cat_project CAPTURE_LOCATION EXAMPLE_CAT_LOCATION OUTPUT_DIRECTORY\n";

	if (argc != 3)
	{
		cout << usage;
		cin.get();
		exit(0);
	}
		
	string videoFileLocation = argv[1];
	string examples_location = argv[2];
	string outputLocation = argv[3];

	CatModel scuzzy (examples_location, "Scuzzy");	
	VideoCapture cap(videoFileLocation);

	ActivityParser actParser(cap, outputLocation);
	actParser.addCatModel(scuzzy);
	actParser.main();
	
    return 0;
}




