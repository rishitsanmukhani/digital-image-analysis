#pragma once
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include <iostream>

using namespace std;
using namespace cv;

class Image{
public:
	string name;
	Mat mat;
	int h,w;
	Image(string _name=""){
		name=_name;
	}
	void load(bool gray_scale=false){
		if(gray_scale)
			mat=imread(name,CV_LOAD_IMAGE_GRAYSCALE);
		else
			mat=imread(name,CV_LOAD_IMAGE_COLOR);
		if(mat.empty()){
			puts("Error loading image...");
			exit(1);
		}
		Mat tmp;
		cvtColor(mat,tmp,COLOR_BGR2YUV);
		mat=tmp.clone();
		h=mat.rows;w=mat.cols;
	}
	void write(string _name){
		Mat tmp;
		cvtColor(mat,tmp,COLOR_YUV2BGR);
		if(!imwrite(_name,tmp)){
			puts("Error writing image...");
			exit(1);
		}
	}
	~Image(){
		mat.release();
	}
};
