#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <iostream>
#include <typeinfo>

using namespace cv;
using namespace std;

Mat source, source_gray, debug_image;
int width, height;
vector<Rect> rectangles;
bool debug_mode = false;

Mat createBinaryImage(Mat, Mat);
Mat keepLines(string);
string intToString(int);
vector<Rect> findRectangles(Mat);
void assignDimensions(const char*, const char*);
void assignSource(string);
void checkDebugMode(const char*);
void drawDebugImage(int, Rect, vector<vector<Point> >, int, vector<Vec4i>);
void ensureArgumentsPresence(int);
void setLabel(Mat&, const std::string, std::vector<Point>&);

int main( int argc, char *argv[] )
{
	ensureArgumentsPresence(argc);
	// Read the source image given as first argument
	assignSource(argv[1]);
	// Init width and height given as second and third arguments
	assignDimensions(argv[2], argv[3]);
	// Prepare the debug mode if a fourth argument is "-d"
	checkDebugMode(argv[4]);

	// Build an image with only horizontal lines
	Mat horizontal_image = keepLines("horizontal");
	// Build an image with only vertical lines
	Mat vertical_image   = keepLines("vertical");

	// Create an image which combine both images
	Mat binary_image = createBinaryImage(horizontal_image, vertical_image);

	// Create a image for debug mode
	debug_image = source.clone();

	rectangles = findRectangles(binary_image);

	int compt = 0;
	for(size_t i = 0; i < rectangles.size(); i++)
	{
		compt ++;
		Rect rect = rectangles[i];
		Mat checkbox(source_gray, rect);
		checkbox = checkbox > 128;
		int total_pixels = checkbox.rows * checkbox.cols;
		int black_pixels = total_pixels - countNonZero(checkbox);
		if ( black_pixels > 0)
			cout<<"The number of pixels that are zero is "<<black_pixels<<" of " << total_pixels << " for checkbox "<<compt<<endl;

	}

	// imshow("horizontal", horizontal_image);
	// imshow("vertical", bin_v);
	if (debug_mode)
	{
		namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
		imshow( "Contours", debug_image );
	}

	waitKey(0);
	return(0);

}

vector<Rect> findRectangles(Mat binary_image)
{
	vector<Rect> boxes;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(binary_image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0));

	double expected_area = width * height;
	double max_area      = expected_area + expected_area * 0.1;
	int compt            = 0;

	for(int idx = 0; idx >= 0; idx = hierarchy[idx][0])
	{
		double area = contourArea(contours[idx]);
		Rect   rect = boundingRect(contours[idx]);

		if (hierarchy[idx][3]<0 && rect.width >= width && rect.height >= height && area <= max_area)
		{
			compt ++;
			rect.width = width;
			rect.height = height;
			boxes.push_back(rect);
			if (debug_mode) drawDebugImage(compt, rect, contours, idx, hierarchy);
		}
	}

	return boxes;
}

void drawDebugImage(int compt, Rect rect, vector<vector<Point> > contours, int idx, vector<Vec4i> hierarchy)
{
	// drawContours(debug_image, contours, idx, Scalar(0, 0, 255), 2, 8, hierarchy);
	rectangle(debug_image, rect.tl(), rect.br(), Scalar(0, 0, 255), 1, 8, 0);
	string checkbox_label = intToString(compt);
	setLabel( debug_image, checkbox_label, contours[idx] );
}

void checkDebugMode(const char* arg)
{
	string debug_argument;
	if(arg) debug_argument = arg;
	if ( debug_argument == "-d")
		debug_mode = true;
}

// Combined horizontal and vertical image in order to create the binary image
Mat createBinaryImage(Mat horizontal, Mat vertical)
{
	Mat binary_image = horizontal & vertical;
	threshold(binary_image, binary_image, 75, 255.0, CV_THRESH_BINARY_INV);
	return binary_image;
}

Mat keepLines(string dimension_type)
{
	Mat morph, bin;

	Size size;
	if (dimension_type == "horizontal")
	{
		size = Size(width, 1);
	} else {
		size = Size(1, height);
	}

	// morphological closing with a row filter : retain only large horizontal edges
	Mat morphKernelH = getStructuringElement(MORPH_RECT, size);
	morphologyEx(source_gray, morph, MORPH_CLOSE, morphKernelH);
	// binarize: will contain only large horizontal edges
	threshold(morph, bin, 100, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);

	return bin;
}

void setLabel(Mat& im, const std::string label, std::vector<Point>& contour)
{
	double scale  = 0.3;
	int baseline  = 0;
	int fontface  = FONT_HERSHEY_SIMPLEX;
	int thickness = 1;

	Size text = getTextSize( label, fontface, scale, thickness, &baseline );
	Rect r    = boundingRect( contour );

	Point pt( r.x + ( ( r.width - text.width ) / 2 ), r.y + ( ( r.height + text.height ) / 2 ) );
	rectangle( im, pt + Point( 0, baseline ), pt + Point( text.width, -text.height ), CV_RGB(255,255,255), CV_FILLED );
	putText( im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8 );
}

string intToString(int number)
{
	string result;
	ostringstream convert;
	convert << number;
	result = convert.str();

	return result;
}

void ensureArgumentsPresence(int arguments_count)
{
	if( arguments_count < 4 )
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
	cvtColor( source, source_gray, CV_BGR2GRAY );
}

void assignDimensions(const char* w, const char* h)
{
	sscanf(w,"%d",&width);
	sscanf(h,"%d",&height);
}
