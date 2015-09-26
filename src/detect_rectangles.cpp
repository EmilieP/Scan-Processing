#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <iostream>
#include <typeinfo>

using namespace cv;
using namespace std;

Mat source, source_gray;
int width, height;

void ensureArgumentsPresence(int);
void assignSource(string);
void assignDimensions(const char*, const char*);
Mat keepLines(string);
void setLabel(Mat&, const std::string, std::vector<Point>&);
string intToString(int);

int main( int argc, char *argv[] )
{
	ensureArgumentsPresence(argc);
	assignSource(argv[1]);
	assignDimensions(argv[2], argv[3]);

	Mat horizontal_image = keepLines("horizontal");
	Mat vertical_image   = keepLines("vertical");

	// combine the vertical and horizontal edges
	Mat binary_image = horizontal_image & vertical_image;
	threshold(binary_image, binary_image, 75, 255.0, CV_THRESH_BINARY_INV);


	// just for illustration
	Mat rgb = source.clone();

	// find contours
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(binary_image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0));
	// filter contours by area to obtain boxes
	double expected_area = width * height;
	double max_area = expected_area + expected_area * 0.1;

	for(int idx = 0; idx >= 0; idx = hierarchy[idx][0])
	{
		double area = contourArea(contours[idx]);
		Rect   rect = boundingRect(contours[idx]);
		if (rect.width >= width && rect.height >= height && area <= max_area)
		{
			drawContours(rgb, contours, idx, Scalar(0, 0, 255), 2, 8, hierarchy);
			string checkbox_label = intToString(rect.width) + "x" + intToString(rect.height);
			setLabel( rgb, checkbox_label, contours[idx] );
			// take bounding rectangle. better to use filled countour as a mask
			// to extract the rectangle because then you won't get any stray elements
			// cout << " rect: (" << rect.x << ", " << rect.y << ") " << rect.width << " x " << rect.height << endl;
			Mat imRect(source, rect);
		}
	}




	// imshow("horizontal", horizontal_image);
	// imshow("vertical", bin_v);
	namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
	imshow( "Contours", rgb );
	// imshow("bin", binary_image);


	waitKey(0);
	return(0);

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
	double scale  = 0.4;
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
	cvtColor( source, source_gray, CV_BGR2GRAY );
}

void assignDimensions(const char* w, const char* h)
{
	sscanf(w,"%d",&width);
	sscanf(h,"%d",&height);
}
