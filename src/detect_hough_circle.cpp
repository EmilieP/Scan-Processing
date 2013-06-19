#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace cv;


Point getCenterCoordinate( vector<Vec3f> circles )
{
  Point center;
  if( circles.size() <= 0 )
  {
    center.x = 0;
    center.y = 0;
  }
  else
  {
    for( size_t i = 0; i < circles.size(); i++ )
    {
      int radius = cvRound(circles[i][2]);

      if( radius >= 10 && radius <= 30 )
        center.x = cvRound(circles[i][0]);
        center.y = cvRound(circles[i][1]);
        break;
    }
  }
  return center;
}

Mat applyFilters( Mat& img)
{
  Mat var_img;
  cvtColor( img, var_img, CV_BGR2GRAY );
  Canny( var_img, var_img, 5, 70, 3);
  GaussianBlur( var_img, var_img, Size(9, 9), 2, 2 );

  return var_img;
}

vector<Vec3f> getCircles( Mat& img )
{
  vector<Vec3f> circles;
  HoughCircles( img, circles, CV_HOUGH_GRADIENT, 1, img.rows/8, 2, 32.0, 10, 30 );
  return circles;
}



void detectRightCircle( Mat& img )
{
  int width = img.cols;
  Rect roi( width - 150 , 0 , 150, 150 );
  Mat sub_img = img( roi );
  Mat gray = applyFilters( sub_img );
  vector<Vec3f> circles = getCircles( gray );
  Point center = getCenterCoordinate( circles );
  center.x +=  width - 150;

  circle( img, center, 1, Scalar(0,0,255), -1, 8, 0 );
}



void detectLeftCircle( Mat& img )
{
  Rect roi( 0 , 0 , 150, 150 );
  Mat sub_img = img( roi );
  Mat gray = applyFilters( sub_img );
  vector<Vec3f> circles = getCircles( gray );
  Point center = getCenterCoordinate( circles );

  printf("1 cercle de coordonnées LEFT:\n");
  printf("    x : %d\n", center.x );
  printf("    y : %d\n", center.y );
  circle( img, center, 1, Scalar(0,255,0), -1, 8, 0 );
}


/** @function main */
int main(int argc, char** argv)
{
  Mat src = imread( argv[1], 1 );

  if( !src.data )
    return -1;

  detectLeftCircle( src );
  detectRightCircle( src );

  namedWindow( "img");
  moveWindow("img", 1500, 600);
  imshow( "img", src );
  waitKey(0);

  return 0;
}