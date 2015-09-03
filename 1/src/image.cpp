#pragma once
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <cstdlib>
#include <cassert>

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
	void loadImage(bool gray_scale=true){
		if(gray_scale)mat=imread(name,CV_LOAD_IMAGE_GRAYSCALE);
		else mat=imread(name,CV_LOAD_IMAGE_COLOR);
		if(mat.empty()){
			puts("Error loading image...");
			exit(1);
		}
		h=mat.rows;w=mat.cols;
	}
	void writeImage(string _name){
		if(!imwrite(_name,mat)){
			puts("Error writing image...");
			exit(1);
		}
	}
	~Image(){
		mat.release();
	}
};
