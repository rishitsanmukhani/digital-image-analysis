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


Mat energyIndivisual(Mat im, string name, bool displayTranspose, bool toDisplay){
	Mat M(im.rows,im.cols, CV_32SC1);
	int maxima = 0;

	for (int i = 1; i < im.rows-1; ++i){
		for (int j = 1; j < im.cols-1; ++j){
			int bi = im.at<Vec3b>(i-1,j)[0] - im.at<Vec3b>(i+1,j)[0];
			int gi = im.at<Vec3b>(i-1,j)[1] - im.at<Vec3b>(i+1,j)[1];
			int ri = im.at<Vec3b>(i-1,j)[2] - im.at<Vec3b>(i+1,j)[2];
			int bj = im.at<Vec3b>(i,j-1)[0] - im.at<Vec3b>(i,j+1)[0];
			int gj = im.at<Vec3b>(i,j-1)[1] - im.at<Vec3b>(i,j+1)[1];
			int rj = im.at<Vec3b>(i,j-1)[2] - im.at<Vec3b>(i,j+1)[2];
			M.at<int>(i,j) = sqrt(bi*bi + gi*gi + ri*ri + bj*bj + gj*gj + rj*rj);
			maxima = max(maxima,M.at<int>(i,j));
		}
	}

	if(toDisplay){
		Mat energyimage(im.rows,im.cols, CV_8UC1);
		maxima /= 256;
		if(maxima==0) maxima=1;
		for(int i = 1; i < im.rows-1; ++i){
			for (int j = 1; j < im.cols-1; ++j){
					energyimage.at<char>(i,j) = M.at<int>(i,j)/maxima;
			}
		}
		if(displayTranspose)
			imshow(name.c_str(),energyimage.t());
		else
			imshow(name.c_str(),energyimage);	
		waitKey(1);
	}

	return M;
}

Mat energyCumVer(Mat energyInd, string name, bool displayTranspose, bool toDisplay){
	Mat M(energyInd.rows,energyInd.cols, CV_32SC1); // actual
	int maxima = 0;

	for(int j = 1; j < energyInd.cols-1; ++j )
		M.at<int>(0,j) = 0;

	for (int i = 1; i < energyInd.rows-1; ++i){
		M.at<int>(i,1) = energyInd.at<int>(i,1) + min(M.at<int>(i-1,1),M.at<int>(i-1,1),M.at<int>(i-1,2));
		maxima = max(maxima,M.at<int>(i,1));
		M.at<int>(i, energyInd.cols-2) = energyInd.at<int>(i, energyInd.cols-2) + min(M.at<int>(i-1, energyInd.cols-3),M.at<int>(i-1, energyInd.cols-2),M.at<int>(i-1, energyInd.cols-2));
		maxima = max(maxima,M.at<int>(i, energyInd.cols-2));
		for (int j = 2; j < energyInd.cols-2; ++j){
			M.at<int>(i,j) = energyInd.at<int>(i,j) + min(M.at<int>(i-1,j-1),M.at<int>(i-1,j),M.at<int>(i-1,j+1));
			maxima = max(maxima,M.at<int>(i,j));
		}
	}
	if(toDisplay){
		Mat cumEnergy(energyInd.rows,energyInd.cols, CV_8UC1); // just for display
		maxima/=256;
		if(maxima==0) maxima=1;
		for(int i = 1; i < energyInd.rows-1; ++i){
			for (int j = 1; j < energyInd.cols-1; ++j){
					cumEnergy.at<char>(i,j) = M.at<int>(i,j)/maxima;
			}
		}
		if(displayTranspose)
			imshow(name.c_str(),cumEnergy.t());
		else
			imshow(name.c_str(),cumEnergy);	
		waitKey(1);
	}

	return M;
}

Mat getVerSeam(Mat energyCumVer_){

	Mat ret = Mat::zeros(Size(energyCumVer_.cols,energyCumVer_.rows),CV_8UC1);

	int min = INT_MAX;
	int minInd = 0;
	for(int j = 1; j < energyCumVer_.cols-1; ++j){
		if(min>energyCumVer_.at<int>(energyCumVer_.rows-2,j)){
			min = energyCumVer_.at<int>(energyCumVer_.rows-2,j);
			minInd = j;
		}
	}

	Point curr(energyCumVer_.rows-2,minInd);

	ret.at<uchar>(curr.x,curr.y) = 255;
	ret.at<uchar>(curr.x+1,curr.y) = 255;

	for (int i = energyCumVer_.rows-3; i >= 0; --i){
		int j = curr.y;
		curr.x = i;
		int min = INT_MAX;
		if(j!=1){
			if(energyCumVer_.at<int>(i,j-1) < min){
				min = energyCumVer_.at<int>(i,j-1);
				curr.y = j-1;	
			}
		}
		if(j!=energyCumVer_.cols-2){
			if(energyCumVer_.at<int>(i,j+1) < min){
				min = energyCumVer_.at<int>(i,j+1);
				curr.y = j+1;
			}
		}
		if(energyCumVer_.at<int>(i,j) < min)
			curr.y = j;

		ret.at<uchar>(curr.x,curr.y) = 255;
	}

	
	return ret;
}

Mat indexMat(Mat seams){
	Mat ret = Mat::zeros(Size(seams.cols,seams.rows), CV_32SC1);
	// vvi ret(seams.rows,vi(seams.cols,0));
	for(int i = 0; i < seams.rows; ++i){
		int cnt = 0;
		for(int j = 0; j < seams.cols; ++j){
			if(seams.at<uchar>(i,j)!=255){
				ret.at<int>(i,cnt) = j;
				// ret[i][cnt]=j;
				++cnt;
			}
		}
	}
	return ret;
}

Vec3b avg(Vec3b a, Vec3b b){
	uchar B = (int(a[0])+int(b[0]))/2;
	uchar G = (int(a[1])+int(b[1]))/2;
	uchar R = (int(a[2])+int(b[2]))/2;
	return Vec3b(B,G,R);
}

Mat replicateSeam(Mat im, Mat seam, int numSeams = 1){
	Mat ret = Mat::zeros(Size(im.cols+numSeams,im.rows),CV_8UC3);
	for(int i = 0; i < im.rows; ++i){
		int toAdd = 0;
		for(int j = 0; j < im.cols; ++j){
			ret.at<Vec3b>(i,j+toAdd) = im.at<Vec3b>(i,j);
			if(seam.at<uchar>(i,j)==255){
				toAdd+=1;
				if(j+1<im.cols)
					ret.at<Vec3b>(i,j+toAdd) = avg(im.at<Vec3b>(i,j),im.at<Vec3b>(i,j+1));
				else
					ret.at<Vec3b>(i,j+toAdd) = im.at<Vec3b>(i,j);
			}
		}
	}
	return ret;
}

Mat removeVerSeams_3channel_uchar(Mat im, Mat seams, int numSeams){
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

Mat removeVerSeams_1channel_uchar(Mat im, Mat seams, int numSeams){
	Mat ret = Mat::zeros(Size(im.cols-numSeams,im.rows),CV_8UC1);

	for(int i = 0; i < im.rows; ++i){
		int cnt = 0;
		for(int j = 0; j < im.cols; ++j){
			if(seams.at<uchar>(i,j)!=255){
				ret.at<uchar>(i,cnt) = im.at<uchar>(i,j);
				++cnt;
			}
		}
	}
	return ret;
}

Mat removeVerSeams_1channel_int(Mat im, Mat seams, int numSeams){
	Mat ret = Mat::zeros(Size(im.cols-numSeams,im.rows),CV_32SC1);

	for(int i = 0; i < im.rows; ++i){
		int cnt = 0;
		for(int j = 0; j < im.cols; ++j){
			if(seams.at<uchar>(i,j)!=255){
				ret.at<int>(i,cnt) = im.at<int>(i,j);
				++cnt;
			}
		}
	}
	return ret;
}

//returns the Mat with seams removed and the number of seams removed to cover all the required points
pair<Mat,int> removeVerSeams(Mat im, Mat energyCumVer_, Mat pointsToRem, int numPointsToRem){

	Mat ret = im.clone();
	int numSeams = 0;

	while(numPointsToRem!=0){

		numSeams++;
		Mat seam = Mat::zeros(Size(ret.cols,ret.rows),CV_8UC1);

		int min = INT_MAX;
		int minInd = 0;
		for(int j = 1; j < energyCumVer_.cols-1; ++j){
			if(min>energyCumVer_.at<int>(energyCumVer_.rows-2,j)){
				min = energyCumVer_.at<int>(energyCumVer_.rows-2,j);
				minInd = j;
			}
		}

		Point curr(energyCumVer_.rows-2,minInd);

		seam.at<uchar>(curr.x,curr.y) = 255;
		if(pointsToRem.at<uchar>(curr.x,curr.y)==255) numPointsToRem--;
		seam.at<uchar>(curr.x+1,curr.y) = 255;
		if(pointsToRem.at<uchar>(curr.x+1,curr.y)==255) numPointsToRem--;

		for (int i = energyCumVer_.rows-3; i >= 0; --i){
			int j = curr.y;
			curr.x = i;
			int min = INT_MAX;
			if(j!=1){
				if(energyCumVer_.at<int>(i,j-1) < min){
					min = energyCumVer_.at<int>(i,j-1);
					curr.y = j-1;	
				}
			}
			if(j!=energyCumVer_.cols-2){
				if(energyCumVer_.at<int>(i,j+1) < min){
					min = energyCumVer_.at<int>(i,j+1);
					curr.y = j+1;
				}
			}
			if(energyCumVer_.at<int>(i,j) < min)
				curr.y = j;

			seam.at<uchar>(curr.x,curr.y) = 255;
			if(pointsToRem.at<uchar>(curr.x,curr.y)==255) numPointsToRem--;
		}
		ret = removeVerSeams_3channel_uchar(ret, seam, 1);
		energyCumVer_ = removeVerSeams_1channel_int(energyCumVer_, seam, 1);
		pointsToRem = removeVerSeams_1channel_uchar(pointsToRem, seam, 1);
	}

	return make_pair(ret,numSeams);
}

Mat showSeams(Mat im, Mat seams){
	Mat displayIm = im.clone();
	for(int i = 0; i < seams.rows; ++i){
		for(int j = 0; j < seams.cols; ++j){
			if(seams.at<uchar>(i,j)==255){
				displayIm.at<Vec3b>(i,j) = Vec3b(0,0,255);
			}
		}
	}
	imshow("seams",displayIm);
	waitKey(1);
	return displayIm;
}

void mergeSeams(Mat seamBig, Mat seamSmall){
	Mat indMat = indexMat(seamBig);
	for(int i = 0; i < seamSmall.rows; ++i){
		for(int j = 0; j < seamSmall.cols; ++j){
			seamBig.at<uchar>(i,indMat.at<int>(i,j)) = seamSmall.at<uchar>(i,j);
		}
	}
}

Mat getkSeams(Mat im, int k){
	Mat small = im.clone();
	Mat seams = Mat::zeros(Size(small.cols,small.rows),CV_8UC1);
	for(int i = 0; i < k; ++i){
		Mat energyInd = energyIndivisual(small,"energyInd",false,false);
		Mat energyCum = energyCumVer(energyInd,"energyCum",false,false);
		Mat newSeam = getVerSeam(energyCum);
		mergeSeams(seams, newSeam);
		small = removeVerSeams_3channel_uchar(small,newSeam,1);
	}
	return seams;
}

Mat expandCol(Mat im, int numCol){
	Mat seams = getkSeams(im,numCol);
	imshow("3. Seams expanded",seams);
	waitKey(1);
	Mat newIm = replicateSeam(im,seams,numCol);
	return newIm;
}