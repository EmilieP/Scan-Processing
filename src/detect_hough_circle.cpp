#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <dirent.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <fstream>

using namespace cv;

const std::string extensions[] = { "jpg", "png", "jpeg" };
const int extensions_size = sizeof(extensions)/sizeof(std::string);
const int iterator = 1;
std::ofstream text;


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
      int radius = cvRound( circles[i][2] );

      if( radius >= 10 && radius <= 30 ) {
        center.x = cvRound( circles[i][0] );
        center.y = cvRound( circles[i][1] );
        return center;
      }
    }
  }
  return center;
}

void detect_right_circle( Mat& img )
{
  int width = img.cols;
  Rect roi( width - 150 , 0 , 150, 150 );
  Mat sub_img = img( roi );
  Mat gray    = applyFilters( sub_img );
  vector<Vec3f> circles = getCircles( gray );
  if( circles.size() > 0 )
  {
    Point center = getCenterCoordinate( circles );
    center.x += width - 150;
    text << "[ " << center.x << " , " << center.y << " ]";
  }
}

void detect_left_circle( Mat img )
{
  Rect roi( 0, 0, 150, 150 );

  Mat sub_img = img( roi );
  Mat gray = applyFilters( sub_img );
  vector<Vec3f> circles = getCircles( gray );
  if( circles.size() > 0 )
  {
    Point center = getCenterCoordinate( circles );
    text << "[ " << center.x << " , " << center.y << " ]";
  }
}

bool is_graphic_file( std::string filename ) {
  for( int i = 0; i < extensions_size; i++)
  {
    std::size_t found = filename.find(extensions[i]);
    if (found != std::string::npos)
      return 1;
  }
  return 0;
}

bool is_dir(const char* path) {
  struct stat buf;
  stat(path, &buf);
  return S_ISDIR(buf.st_mode);
}

std::string full_path( std::string dir, std::string filename )
{
  return dir + filename;
}

void detect_circles_from_image( Mat& img ){
  detect_left_circle( img );
  detect_right_circle( img );
  text << "\n";
}

int main(int argc, char** argv)
{
  clock_t tic = clock();

  DIR *dir;
  struct dirent *file;

  if ( is_dir( argv[1] ) )
  {
    dir = opendir( argv[1] );

    if( dir != NULL ) {
      std::string folder = argv[1];
      text.open ("coordonnees.txt");
      while(( file = readdir( dir )) )
      {
        for( int j = 0; j < iterator; j++ )
        {
          std::string filename = file->d_name;
          if( is_graphic_file( filename ) )
          {
            std::string img_full_path = full_path( folder, filename );
            Mat img = imread( img_full_path, 1 ) ;
            text << " == " << filename << " ==\n";
            detect_circles_from_image( img );
          }
        }
      }
    }
  }
  else
  {
    if( is_graphic_file( argv[1] ) )
    {
      Mat img = imread( argv[1], 1 );
      detect_circles_from_image( img );
    }
  }
  text.close();

  clock_t toc = clock();
  printf("Elapsed: %f seconds\n", (double)(toc - tic) / CLOCKS_PER_SEC);

  return 0;
}