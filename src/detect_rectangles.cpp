#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

Mat source, source_gray, debug_image;
int width, height, thin, total_rectangles;
vector<Rect> rectangles;
bool debug_mode = false;

Mat createBinaryImage(Mat, Mat);
Mat keepLines(string);
Rect reduceRectangleSelection(Rect);
string intToString(int);
vector<Rect> findRectangles(Mat);
void assignArgs(char *argv[]);
void assignSource(string);
void checkDebugMode(const char*);
void drawDebugImage(int, Rect, vector<vector<Point> >, int, vector<Vec4i>);
void ensureArgumentsPresence(int);
void setLabel(Mat&, const std::string, std::vector<Point>&);

// ./bin/detect_rectangles [nom de l'image] [largeur] [hauteur] [épaisseur] [nombre de rectangles] (-d)
// ./bin/detect_rectangles mon-image.jpg 20 30 3 12 -d
int main( int argc, char *argv[] )
{
	// Vérifie la présence de tous les arguments
	ensureArgumentsPresence(argc);
	// Lit l'image source
	assignSource(argv[1]);
	// Initialise la hauteur des rectangles, largeur des rectangles, épaisseur des contours des rectangles, nombre de rectangles à trouver
	assignArgs(argv);
	// Active le mode debug si précisé dans les arguments
	checkDebugMode(argv[6]);

	// Créer une image contenant uniquement les lignes horizontales
	Mat horizontal_image = keepLines("horizontal");
	// Créer une image contenant uniquement les lignes verticales
	Mat vertical_image   = keepLines("vertical");

	// Combine les 2 images précédentes pour ne garder que les rectangles
	Mat binary_image = createBinaryImage(horizontal_image, vertical_image);

	// Initialize l'image utilisée dans le mode debug
	debug_image = source.clone();

	// Retourne un tableau de rectangles trouvés
	rectangles = findRectangles(binary_image);

	stringstream return_string;
	int count_rectangles = rectangles.size();
	int compt            = 0;

	for(size_t i = 0; i < count_rectangles; i++)
	{
		compt ++;
		Rect rect = rectangles[i];
		rect      = reduceRectangleSelection(rect);
		Mat checkbox(source_gray, rect);
		threshold(checkbox, checkbox, 127, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		int total_pixels = checkbox.rows * checkbox.cols;
		int black_pixels = total_pixels - countNonZero(checkbox);
		if ( black_pixels > 0 && debug_mode )
		{
			cout << "[" << compt << "]" << "(" << rect.tl().x << "," << rect.tl().y << ")->" << black_pixels << endl;
		} else if ( black_pixels > 0 )
		{
			return_string << rect.tl().x << "," << rect.tl().y << "," << black_pixels << "|";
		}
	}

	if (debug_mode && count_rectangles != total_rectangles)
	{
		int missing_rectangles = total_rectangles - count_rectangles;
		cout << "Attention : " << missing_rectangles << " rectangles n'ont pas été trouvé." << endl;
	}

	if (debug_mode)
	{
		namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
		imshow( "Contours", debug_image );
	} else if ( !debug_mode && count_rectangles == total_rectangles ){
		cout << return_string.str();
	}

	waitKey(0);
	return(0);

}

Rect reduceRectangleSelection(Rect rect)
{
	return Rect(
		rect.tl().x + thin,
		rect.tl().y + thin,
		rect.width  - thin,
		rect.height - thin
	);
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

		if ( hierarchy[idx][3] < 0 && rect.width >= width && rect.height >= height && area <= max_area )
		{
			compt ++;
			rect.width  = width;
			rect.height = height;
			boxes.push_back(rect);
			if (debug_mode) drawDebugImage(compt, rect, contours, idx, hierarchy);
		}
	}

	return boxes;
}

// Draw each rectangles with its number on debug mode image
void drawDebugImage(int compt, Rect rect, vector<vector<Point> > contours, int idx, vector<Vec4i> hierarchy)
{
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

	Mat morph_kernel = getStructuringElement(MORPH_RECT, size);
	morphologyEx(source_gray, morph, MORPH_CLOSE, morph_kernel);
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
	if( arguments_count < 6 )
	{
		cerr << "Missing arguments : ./detect_rectangles [path to image] [width of the rectangles] [height of the rectangles] [total number of rectangles]" << endl;
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

void assignArgs(char *argv[])
{
	sscanf(argv[2],"%d",&width);
	sscanf(argv[3],"%d",&height);
	sscanf(argv[4],"%d",&thin);
	sscanf(argv[5],"%d",&total_rectangles);
}
