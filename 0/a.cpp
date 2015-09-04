#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <algorithm>

using namespace cv;
using namespace std;

void scaleDownDecimation(Mat& in,Mat& out){
	puts("Scaling down...");
	for(int i=0;i<out.rows;i++){
		for(int j=0;j<out.cols;j++){
			Vec3b& rgba= out.at<Vec3b>(i,j);
			rgba=(Vec3i(in.at<Vec3b>(i<<1,j<<1))+Vec3i(in.at<Vec3b>((i<<1) + 1,j<<1))+Vec3i(in.at<Vec3b>(i<<1,(j<<1) + 1))+Vec3i(in.at<Vec3b>((i<<1) + 1,(j<<1) + 1)))/4;
		}
	}
	puts("Scaling down done");
}

void scaleDownSkip(Mat& in,Mat& out){
	puts("Scaling down...");
	for(int i=0;i<out.rows;i++){
		for(int j=0;j<out.cols;j++){
			Vec3b& rgba= out.at<Vec3b>(i,j);
			rgba=Vec3i(in.at<Vec3b>(i<<1,j<<1));
		}
	}
	puts("Scaling down done");
}

void scaleUpInterpolation(Mat& in,Mat& out){
	puts("Scaling up...");

	puts("Interpolating one dimension...");
	for(int i=0;i<out.rows;i+=2){
		for(int j=0;j<out.cols;j++){
			out.at<Vec3b>(i,j) = in.at<Vec3b>(i>>1,j>>1);
			j++;
			if(j==out.cols-1)
				out.at<Vec3b>(i,j)=out.at<Vec3b>(i,j-1);
			else
				out.at<Vec3b>(i,j)=(Vec3i(in.at<Vec3b>(i>>1,j>>1))+Vec3i(in.at<Vec3b>(i>>1,(j>>1)+1)))/2;
		}
	}
	puts("Done");
	puts("Interpolating second dimension...");
	for(int i=1;i<out.rows-1;i+=2){
		for(int j=0;j<out.cols;j++){
			out.at<Vec3b>(i,j)=(Vec3i(out.at<Vec3b>(i-1,j))+Vec3i(out.at<Vec3b>(i+1,j)))/2;
		}
	}
	for(int j=0,i=out.rows-1;j<out.cols;j++){
		out.at<Vec3b>(i,j)=out.at<Vec3b>(i-1,j);
	}
	puts("Done");
	puts("Scaling up done");
}

void scaleUpReplication(Mat& in,Mat& out){
	puts("Scaling up...");

	puts("Replicating one dimension...");
	for(int i=0;i<out.rows;i+=2){
		for(int j=0;j<out.cols;j+=2){
			out.at<Vec3b>(i,j) = Vec3i(in.at<Vec3b>(i>>1,j>>1));
			out.at<Vec3b>(i,j+1) = Vec3i(in.at<Vec3b>(i>>1,j>>1));			
		}
	}
	puts("Done");
	puts("Replicating second dimension...");
	for(int i=1;i<out.rows-1;i+=2){
		for(int j=0;j<out.cols;j++){
			out.at<Vec3b>(i,j)=Vec3i(out.at<Vec3b>(i-1,j));
		}
	}
	for(int j=0,i=out.rows-1;j<out.cols;j++){
		out.at<Vec3b>(i,j)=out.at<Vec3b>(i-1,j);
	}
	puts("Done");
	puts("Scaling up done");
}

int main(int argc, char** argv){
	if(argc<2){
		puts("Usage: a <name-of-image>");
		return 1;
	}
	Mat in = imread(argv[1],CV_LOAD_IMAGE_COLOR);
	if(in.empty()){
		puts("Cannot open image!");
		return -1;
	}
	Mat out1(in.rows/2,in.cols/2,CV_8UC3,Scalar(0,0,0));
	Mat out2(in.rows/2,in.cols/2,CV_8UC3,Scalar(0,0,0));
	Mat out3(in.rows<<1,in.cols<<1,CV_8UC3,Scalar(0,0,0));
	Mat out4(in.rows<<1,in.cols<<1,CV_8UC3,Scalar(0,0,0));

	scaleDownDecimation(in,out1);
	scaleDownSkip(in,out2);

	scaleUpInterpolation(in,out3);
	scaleUpReplication(in,out4);

	
	imwrite("smallDecimation.bmp",out1);
	imwrite("smallSkip.bmp",out2);

	imwrite("bigInterpolation.bmp",out3);
	imwrite("bigReplication.bmp",out4);

	puts("Images saved");
	waitKey(0);
	return 0;
}