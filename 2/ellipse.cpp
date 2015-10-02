#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "image.h"
#include "util.h"
using namespace cv;

Mat src;
Mat src_gray;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

void thresh_callback(int, void* );

int main( int argc, char** argv ){
	src = imread( argv[1], 1 );

	cvtColor( src, src_gray, CV_BGR2GRAY );
	blur( src_gray, src_gray, Size(3,3) );

	string source_window("Source");
	namedWindow( source_window.c_str(),CV_WINDOW_AUTOSIZE);
	imshow( source_window, src );

	createTrackbar( " Threshold:", "Source", &thresh, max_thresh, thresh_callback );
	thresh_callback(0,0);
	waitKey(0);
	return(0);
}

void thresh_callback(int, void* ){
	Mat threshold_output,crop_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY);
	findContours(threshold_output,contours, hierarchy, CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE,Point(0, 0));

	vector<RotatedRect> minRect(contours.size() );
	vector<RotatedRect> minEllipse(contours.size());
	
	int idx=-1,max_area=0;
	for( int i = 0; i < contours.size(); i++ ){
		minRect[i] = minAreaRect(Mat(contours[i]));
		if(contours[i].size()>5){
			minEllipse[i] = fitEllipse(Mat(contours[i]));
			if(abs(contourArea(contours[i]))>max_area)max_area=abs(contourArea(contours[i])),idx=i;
		}
	}
	Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3);
	Mat drawing_edge = Mat::zeros(threshold_output.size(), CV_8UC3);
	
	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
	ellipse( drawing, minEllipse[idx], color,-1, 8 );
	for(int i=0;i<contours.size();i++){
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
		drawContours(drawing_edge,contours,i,color,1,8);
		if(i!=idx && contours[i].size()>5)ellipse( drawing_edge, minEllipse[i], color,-1, 8 );
	}
	
	Image img("median.bmp");
	img.loadImage(false);
	for(int i=0;i<img.h;i++){
		for(int j=0;j<img.w;j++){
			Vec3b& bgr=img.mat.at<Vec3b>(i,j);
			Vec3b _c=drawing.at<Vec3b>(i,j);
			if(_c[0]==0 && _c[1]==0 && _c[2]==0){
				bgr[0]=0;bgr[1]=0;bgr[2]=0;
			}
		}
	}
	img.writeImage("face.bmp");

	imshow( "Contours", drawing );
	imshow( "Contours1", drawing_edge);
	imwrite("controus.bmp",drawing);
	imwrite( "Contours1.bmp", drawing_edge);
}
