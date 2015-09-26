#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <iostream>
#include <typeinfo>

using namespace cv;
using namespace std;

Mat source;
int width, height;

void ensureArgumentsPresence(int);
void assignSource(string);
void assignDimensions(const char*, const char*);

int main( int argc, char *argv[] )
{
	ensureArgumentsPresence(argc);
	assignSource(argv[1]);
	assignDimensions(argv[2], argv[3]);
}

void ensureArgumentsPresence(int arguments_count)
{
	if( arguments_count != 4 )
	{
		cerr << "Missing arguments : ./detect_rectangles [path to image] [width of the rectangles] [height of the rectangles]" << endl;
		exit( EXIT_FAILURE );
	}
}

void assignSource(string filename)
{
	source = imread( filename );
	if ( !source.data )
	{
		cerr << "Problem loading image !" << endl;
		exit( EXIT_FAILURE );
	}
}

void assignDimensions(const char* w, const char* h)
{
	sscanf(w,"%d",&width);
	sscanf(h,"%d",&height);
}
