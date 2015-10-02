#include <iostream>
#include <string>
#include <sstream>
#include "histogram.h"
using namespace std;

#define MAX_INTENSITY 255
#define PI 3.14159265
#define pb push_back
#define mp make_pair


void applyMask(float** mask,int N,Image* img,double bias=0){
	assert(N%2 && "Size should be odd.");
	assert(img!=NULL && mask!=NULL);
	Mat tmp=img->mat.clone();
	for(int i=0;i<img->h;++i){
		for(int j=0;j<img->w;++j){
			int x=i-(N>>1),y=j-(N>>1);
			double sum=0;
			for(int m=0;m<N;m++){
				for(int n=0;n<N;n++){
					if(x+m<0 || x+m>=img->h)continue;
					if(y+n<0 || y+n>=img->w)continue;
					sum += tmp.at<uchar>(x+m,y+n)*mask[m][n];
				}
			}
			sum+=bias;
			if(sum>MAX_INTENSITY)sum=MAX_INTENSITY;
			img->mat.at<uchar>(i,j)=(uint8_t)sum;
		}
	}
}
template <typename T> string to_string(const T& n){
	ostringstream stm;
	stm<<n;
	return stm.str();
}
