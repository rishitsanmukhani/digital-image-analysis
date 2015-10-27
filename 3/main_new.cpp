#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc_c.h"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\core\core_c.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <math.h>
#include <limits.h>

using namespace cv;
using namespace std;

typedef vector<int> vi;
typedef vector<vector<int> > vvi;


int min(int a, int b, int c){
	if(a<b){
		if(a<c) return a;
		else return c;
	}
	else{
		if(b<c) return b;
		else return c;
	}
}

int max(int a, int b){
	if(a<b) return b;
	else return a;
}

void energyIndivisual(Mat im, bool toDisplay,vvi& M){

	int maxima = 0;
	for (int i = 1; i < im.rows-1; ++i){
		for (int j = 1; j < im.cols-1; ++j){
			int bi = im.at<Vec3b>(i-1,j)[0] - im.at<Vec3b>(i+1,j)[0];
			int gi = im.at<Vec3b>(i-1,j)[1] - im.at<Vec3b>(i+1,j)[1];
			int ri = im.at<Vec3b>(i-1,j)[2] - im.at<Vec3b>(i+1,j)[2];
			int bj = im.at<Vec3b>(i,j-1)[0] - im.at<Vec3b>(i,j+1)[0];
			int gj = im.at<Vec3b>(i,j-1)[1] - im.at<Vec3b>(i,j+1)[1];
			int rj = im.at<Vec3b>(i,j-1)[2] - im.at<Vec3b>(i,j+1)[2];
			M[i][j] = int(sqrt(bi*bi + gi*gi + ri*ri + bj*bj + gj*gj + rj*rj));
			maxima = max(maxima,M[i][j]);
		}
	}

	if(toDisplay){
		Mat energyimage = Mat::zeros(Size(im.cols,im.rows), CV_8UC1);
		maxima /= 255;
		if(maxima==0) maxima=1;
		for(int i = 1; i < im.rows-1; ++i){
			for (int j = 1; j < im.cols-1; ++j){
					energyimage.at<uchar>(i,j) = M[i][j]/maxima;
			}
		}
		imshow("energyInd",energyimage);
		waitKey(1);
	}
}

void energyCumVer(vvi& energyInd, string name, bool displayTranspose, bool toDisplay,vvi& M){
	int maxima = 0;
	for(int j = 1; j < energyInd[0].size()-1; ++j )
		M[0][j] = 0;

	for (int i = 1; i < energyInd.size()-1; ++i){
		M[i][1] = energyInd[i][1] + min(M[i-1][1],M[i-1][1],M[i-1][2]);
		maxima = max(maxima,M[i][1]);
		M[i][energyInd[0].size()-2] = energyInd[i][energyInd[0].size()-2] + min(M[i-1][energyInd[0].size()-3],M[i-1][energyInd[0].size()-2],M[i-1][energyInd[0].size()-2]);
		maxima = max(maxima,M[i][energyInd[0].size()-2]);
		for (int j = 2; j < energyInd[0].size()-2; ++j){
			M[i][j] = energyInd[i][j] + min(M[i-1][j-1],M[i-1][j],M[i-1][j+1]);
			maxima = max(maxima,M[i][j]);
		}
	}

	if(toDisplay){
		Mat cumEnergy(energyInd.size(),energyInd[0].size(), CV_8UC1); // just for display
		maxima/=255;
		if(maxima==0) maxima=1;
		for(int i = 1; i < energyInd.size()-1; ++i){
			for (int j = 1; j < energyInd[0].size()-1; ++j){
					cumEnergy.at<uchar>(i,j) = M[i][j]/maxima;
			}
		}
		if(displayTranspose)
			imshow(name.c_str(),cumEnergy.t());
		else
			imshow(name.c_str(),cumEnergy);	
	}
}

vvi indexMat(Mat m){
	// Mat ret = Mat::zeros(Size(m.cols,m.rows), CV_32SC1);
	vvi ret(m.rows,vi(m.cols,0));
	for(int i = 0; i < m.rows; ++i){
		int cnt = 0;
		for(int j = 0; j < m.cols; ++j){
			if(m.at<uchar>(i,j)!=255){
				// ret.at<int>(i,cnt) = j;
				ret[i][cnt]=j;
				++cnt;
			}
		}
	}
	return ret;
}

void mergeSeams(Mat m1, Mat m2){
	vvi cum = indexMat(m1);
	for(int i = 0; i < m2.rows; ++i){
		for(int j = 0; j < m2.cols; ++j){
			if(m2.at<uchar>(i,j)==255){
				m1.at<uchar>(i,cum[i][j]) = 255;
			}
		}
	}
}

Mat removeVerSeams(Mat im, Mat seams, int numSeams){
	Mat ret = Mat::zeros(Size(im.cols-numSeams,im.rows),CV_8UC3);

	for(int i = 0; i < im.rows; ++i){
		int cnt = 0;
		for(int j = 0; j < im.cols; ++j){
			if(seams.at<uchar>(i,j)!=255){
				ret.at<Vec3b>(i,cnt) = im.at<Vec3b>(i,j);
				++cnt;
			}
		}
	}
	return ret;
}

Mat markSeamVer(Mat im, int numSeams){

	Mat ret = Mat::zeros(Size(im.cols,im.rows),CV_8UC1);

	for (int cnt = 0; cnt < numSeams; ++cnt){

		Mat smaller = removeVerSeams(im,ret,cnt);
		// Mat energyInd = Mat::zeros(Size(smaller.cols,smaller.rows),CV_32SC1);
		vvi energyInd(smaller.rows,vi(smaller.cols,0));
		energyIndivisual(smaller,true,energyInd);
		// Mat energyCum = Mat::zeros(Size(energyInd[0].size(),energyInd.rows), CV_32SC1);
		vvi energyCum(smaller.rows,vi(smaller.cols,0));
		energyCumVer(energyInd,"energyCum",false,true,energyCum);
		Mat retLocal = Mat::zeros(Size(smaller.cols,smaller.rows),CV_8UC1);

		int min = INT_MAX;
		int minInd = 0;
		for(int j = 1; j < smaller.cols-1; ++j){
			if(min>energyCum[smaller.rows-2][j]){
				min = energyCum[smaller.rows-2][j];
				minInd = j;
			}
		}

		waitKey(1);
		Point curr(smaller.rows-2,minInd);

		retLocal.at<uchar>(curr.x,curr.y) = 255;
		retLocal.at<uchar>(curr.x+1,curr.y) = 255;

		for (int i = energyCum.size()-3; i >= 0; --i){
			int j = curr.y;
			curr.x = i;
			int min = INT_MAX;
			if(j!=1){
				if(energyCum[i][j-1] < min){
					min = energyCum[i][j-1];
					curr.y = j-1;	
				}
			}
			if(j!=energyCum[0].size()-2){
				if(energyCum[i][j+1] < min){
					min = energyCum[i][j+1];
					curr.y = j+1;
				}
			}
			if(energyCum[i][j] < min)
				curr.y = j;

			retLocal.at<uchar>(curr.x,curr.y) = 255;
		}

		mergeSeams(ret,retLocal);
	}
	
	return ret;
}

void seam(Mat im, Mat oldSeamsHor, Mat oldSeamsVer, int numOldSeamsHor, int numOldSeamsVer, int numNewSeamsHor, int numNewSeamsVer){

	Mat newIm1 = removeVerSeams(im.t(),oldSeamsHor,numOldSeamsHor);
	Mat newIm2 = removeVerSeams(im,oldSeamsVer,numOldSeamsVer);

	Mat newSeamsHor = markSeamVer(newIm1, numNewSeamsHor);
	Mat newSeamsVer = markSeamVer(newIm2, numNewSeamsVer);

	imshow("new1",newSeamsHor);
	imshow("new2",newSeamsVer);

	mergeSeams(oldSeamsHor, newSeamsHor);
	mergeSeams(oldSeamsVer, newSeamsVer);
}

void showSeams(Mat im, Mat seamsHor, Mat seamsVer){
	Mat displayIm = im.clone();
	for(int i = 0; i < displayIm.rows; ++i){
		for(int j = 0; j < displayIm.cols; ++j){
			if(seamsHor.at<uchar>(j,i)==255 || seamsVer.at<uchar>(i,j)==255){
				displayIm.at<Vec3b>(i,j) = Vec3b(0,0,255);
			}
		}
	}
	imshow("seams",displayIm);
	waitKey(1);
}

Mat removeBothSeamsVerFirst(Mat im, Mat seamsHor, Mat seamsVer, int numSeamsHor, int numSeamsVer){
	Mat ret = Mat::zeros(Size(im.cols-numSeamsVer,im.rows),CV_8UC3);
	Mat newSeamsHor = Mat::zeros(Size(im.cols-numSeamsVer,im.rows),CV_8UC1);

	
	for(int j = 0; j < im.cols; ++j){
		int c=0;
		for(int i = 0; i < im.rows; ++i){
			if(seamsHor.at<uchar>(i,j)==255){
				c++;
			}
		}
		// if(c!=numSeamsHor)
		// 	cout << "shouldn't hor" << endl;
	}

	for(int i = 0; i < im.rows; ++i){
		int c=0;
		for(int j = 0; j < im.cols; ++j){
			if(seamsVer.at<uchar>(i,j)==255){
				c++;
			}
		}
		// if(c!=numSeamsVer)
		// 	cout << "shouldn't ver" << endl;
	}

	for(int i = 0; i < im.rows; ++i){
		int cnt = 0;
		for(int j = 0; j < im.cols; ++j){
			if(seamsVer.at<uchar>(i,j)!=255){
				ret.at<Vec3b>(i,cnt) = im.at<Vec3b>(i,j);
				newSeamsHor.at<uchar>(i,cnt) = seamsHor.at<uchar>(i,j);
				++cnt;
			}
		}
		// if(im.cols-cnt!=numSeamsVer)
		// 	cout << "Error!" << endl;
	}

	imshow("newSeamsHor",newSeamsHor);
	imshow("oldSeamsHor",seamsHor);

	waitKey(1);
// cout << "done" << endl;

	Mat retFinal = Mat::zeros(Size(im.cols-numSeamsVer,im.rows-numSeamsHor),CV_8UC3);

	for(int j = 0; j < ret.cols; ++j){
		int cnt = 0;
		for(int i = 0; i < ret.rows; ++i){
			if(newSeamsHor.at<uchar>(i,j)!=255){
				retFinal.at<Vec3b>(cnt,j) = ret.at<Vec3b>(i,j);
				++cnt;
			}
		}
		if(ret.rows-cnt!=numSeamsHor){
			cout <<"here " << (ret.rows-cnt)-numSeamsHor << endl;
			cout << j  << endl;
		}
	}
	
// cout << "done" << endl;
	return retFinal;
}

Mat removeBothSeamsHorFirst(Mat im, Mat seamsHor, Mat seamsVer, int numSeamsHor, int numSeamsVer){
	Mat ret = Mat::zeros(Size(im.cols,im.rows-numSeamsHor),CV_8UC3);
	Mat newSeamsVer = Mat::zeros(Size(im.cols,im.rows-numSeamsHor),CV_8UC1);

	for(int j = 0; j < im.cols; ++j){
		int cnt = 0;
		for(int i = 0; i < im.rows; ++i){
			if(seamsHor.at<uchar>(i,j)!=255){
				ret.at<Vec3b>(cnt,j) = im.at<Vec3b>(i,j);
				newSeamsVer.at<uchar>(cnt,j) = seamsVer.at<uchar>(i,j);
				++cnt;
			}
		}
	}

	Mat retFinal = Mat::zeros(Size(im.cols-numSeamsVer,im.rows-numSeamsHor),CV_8UC3);

	for(int i = 0; i < ret.rows; ++i){
		int cnt = 0;
		for(int j = 0; j < ret.cols; ++j){
			if(newSeamsVer.at<uchar>(i,j)!=255){
				retFinal.at<Vec3b>(i,cnt) = ret.at<Vec3b>(i,j);
				++cnt;
			}
		}
	}

	return retFinal;
}

// int main(int argc, char** argv){
// 	Mat im = imread(argv[1]);
// 	Mat empty1 = Mat::zeros(Size(im.rows,im.cols),CV_8UC1);
// 	Mat empty2 = Mat::zeros(Size(im.cols,im.rows),CV_8UC1);
// 	seam(im,empty1,empty2,0,0,atoi(argv[2]),atoi(argv[3]));
// 	showSeams(im,empty1,empty2);
// 	imshow("orig",im);
// 	imshow("compressed",removeBothSeamsVerFirst(im,empty1.t(),empty2,atoi(argv[2]),atoi(argv[3])));
// 	waitKey(0);
// }