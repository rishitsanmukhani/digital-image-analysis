#pragma once
#include "image.cpp"

class Histogram{
public:
	Image *img;
	int ***hist;
	int max_intensity;
	Histogram(Image *_img=NULL){
		img=_img;
		if(img){
			int bit_length = (img->mat.elemSize1()<<3);
			max_intensity=(1<<bit_length);
			int*** A;
			if(img->mat.channels()==1){
				A=new int**;
				A[0]=new int*;
				A[0][0]=new int[max_intensity];
				for(int k=0;k<max_intensity;k++){
						A[0][0][k]=0;
				}
			}
			else if(img->mat.channels()==3){
				A=new int**[max_intensity];
				for(int i=0;i<max_intensity;i++)
					A[i]=new int*[max_intensity];
				for(int i=0;i<max_intensity;i++)
					for(int j=0;j<max_intensity;j++)
						A[i][j]=new int[max_intensity];
				for(int i=0;i<max_intensity;i++)
					for(int j=0;j<max_intensity;j++)
						for(int k=0;k<max_intensity;k++)
							A[i][j][k]=0;
			}
			else{
				puts("Invalid image format.");
				exit(1);
			}
			hist=A;
		}
	}
	void calcuate(){
		assert(img!=NULL);
		if(img->mat.channels()==3){
			for(int i=0;i<img->mat.rows;i++){
				for(int j=0;j<img->mat.cols;j++){
					Vec3b& bgr = img->mat.at<Vec3b>(i,j);
					hist[bgr[0]][bgr[1]][bgr[2]]++;
				}
			}
		}
		else if(img->mat.channels()==1){
			for(int i=0;i<img->mat.rows;i++){
				for(int j=0;j<img->mat.cols;j++){
					hist[0][0][img->mat.at<uchar>(i,j)]++;
				}
			}
		}
		else{
			puts("Invalid image");
			exit(1);
		}
	}
};