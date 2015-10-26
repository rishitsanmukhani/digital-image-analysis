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
/*
Mat energyIndivisual(Mat im, Mat seam, bool toDisplay){
	Mat M(im.rows,im.cols, CV_32SC1);
	int maxima = 0;

	for (int i = 1; i < im.rows-1; ++i){
		for (int j = 1; j < im.cols-1; ++j){
			if(seam.at<char>(i,j)==255){
				M.at<int>(i,j)=INT_MAX;
				continue;
			}

			int nextUp = i-1;
			while(seam.at<char>(nextUp,j)==255 && nextUp>=0){
				nextUp--;
			}
			if(nextUp==-1){
				M.at<int>(i,j)=INT_MAX;
				continue;
			}
			int nextDown = i+1;
			while(seam.at<char>(nextDown,j)==255 && nextDown<=im.rows-1){
				nextDown++;
			}
			if(nextDown==im.rows){
				M.at<int>(i,j)=INT_MAX;
				continue;
			}
			int nextLeft = j-1;
			while(seam.at<char>(i,nextLeft)==255 && nextLeft>=0){
				nextLeft--;
			}
			if(nextLeft==-1){
				M.at<int>(i,j)=INT_MAX;
				continue;
			}
			int nextRight = j+1;
			while(seam.at<char>(i,nextRight)==255 && nextRight<=im.cols-1){
				nextRight++;
			}
			if(nextRight==im.cols){
				M.at<int>(i,j)=INT_MAX;
				continue;
			}
			int bi = im.at<Vec3b>(nextUp,j)[0] - im.at<Vec3b>(nextDown,j)[0];
			int gi = im.at<Vec3b>(nextUp,j)[1] - im.at<Vec3b>(nextDown,j)[1];
			int ri = im.at<Vec3b>(nextUp,j)[2] - im.at<Vec3b>(nextDown,j)[2];
			int bj = im.at<Vec3b>(i,nextLeft)[0] - im.at<Vec3b>(i,nextRight)[0];
			int gj = im.at<Vec3b>(i,nextLeft)[1] - im.at<Vec3b>(i,nextRight)[1];
			int rj = im.at<Vec3b>(i,nextLeft)[2] - im.at<Vec3b>(i,nextRight)[2];
			M.at<int>(i,j) = sqrt(bi*bi + gi*gi + ri*ri + bj*bj + gj*gj + rj*rj);
			maxima = max(maxima,M.at<int>(i,j));
		}
	}

	if(toDisplay){
		Mat energyimage(im.rows,im.cols, CV_8UC1);
		maxima /= 255;
		if(maxima==0) maxima=1;
		for(int i = 1; i < im.rows-1; ++i){
			for (int j = 1; j < im.cols-1; ++j){
					energyimage.at<char>(i,j) = M.at<int>(i,j)/maxima;
			}
		}
		imshow("energyInd",energyimage);
		waitKey(1);
	}

	return M;
}
*/

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

void energyIndivisual(Mat im, bool toDisplay,Mat& M){

	int maxima = 0;
	puts("energyIndivisual");
	for (int i = 1; i < im.rows-1; ++i){
		for (int j = 1; j < im.cols-1; ++j){
			int bi = im.at<Vec3b>(i-1,j)[0] - im.at<Vec3b>(i+1,j)[0];
			int gi = im.at<Vec3b>(i-1,j)[1] - im.at<Vec3b>(i+1,j)[1];
			int ri = im.at<Vec3b>(i-1,j)[2] - im.at<Vec3b>(i+1,j)[2];
			int bj = im.at<Vec3b>(i,j-1)[0] - im.at<Vec3b>(i,j+1)[0];
			int gj = im.at<Vec3b>(i,j-1)[1] - im.at<Vec3b>(i,j+1)[1];
			int rj = im.at<Vec3b>(i,j-1)[2] - im.at<Vec3b>(i,j+1)[2];
			M.at<int>(i,j) = int(sqrt(bi*bi + gi*gi + ri*ri + bj*bj + gj*gj + rj*rj));
			maxima = max(maxima,M.at<int>(i,j));
		}
	}

	if(toDisplay){
		Mat energyimage(im.rows,im.cols, CV_8UC1);
		maxima /= 255;
		if(maxima==0) maxima=1;
		for(int i = 1; i < im.rows-1; ++i){
			for (int j = 1; j < im.cols-1; ++j){
					energyimage.at<char>(i,j) = M.at<int>(i,j)/maxima;
			}
		}
		imshow("energyInd",energyimage);
		waitKey(1);
	}
	puts("energyIndivisual end");
}

void energyCumVer(Mat energyInd, string name, bool displayTranspose, bool toDisplay,Mat& M){
	cout << "energyCumVer" << M.size() << endl;
	int maxima = 0;

	cout <<"start"<<endl;

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
		maxima/=255;
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
	}
	cout <<"end"<<endl;

}

Mat indexMat(Mat m){
	Mat ret(m.rows,m.cols, CV_32SC1);
	for(int i = 0; i < m.rows; ++i){
		ret.at<int>(i,0) = 0;
		int counter = 1;
		for(int j = 1; j < m.cols; ++j){
			if(m.at<char>(i,j)!=255){
				ret.at<int>(i,counter) = j;
				++counter;
			}
		}
	}
	return ret;
}

void mergeSeams(Mat m1, Mat m2){
	Mat cum = indexMat(m1);
	for(int i = 0; i < m2.rows; ++i){
		for(int j = 0; j < m2.cols; ++j){
			if(m2.at<char>(i,j)==255){
				m1.at<char>(i,cum.at<int>(i,j)) = 255;
			}
		}
	}
}

Mat removeVerSeams(Mat im, Mat seams, int numSeams){
	Mat ret(im.rows,im.cols-numSeams,CV_8UC3);

	for(int i = 0; i < im.rows; ++i){
		int counter = 0;
		for(int j = 0; j < im.cols; ++j){
			if(seams.at<char>(i,j)!=255){
				ret.at<Vec3b>(i,counter) = im.at<Vec3b>(i,j);
				++counter;
			}
		}
	}
	return ret;
}

Mat markSeamVer(Mat im, int numSeams){

	Mat ret(im.rows,im.cols,CV_8UC1);

	for (int cnt = 0; cnt < numSeams; ++cnt,cout<<cnt<<endl){
		cout<<cnt<<" " <<numSeams<<endl;

		Mat smaller = removeVerSeams(im,ret,cnt);
		puts("Allocating..");
		Mat energyInd = Mat::zeros(Size(smaller.cols,smaller.rows),CV_32SC1);
		puts("done..");
		energyIndivisual(smaller,false,energyInd);
		puts("Allocating..");
		Mat energyCum = Mat::zeros(Size(energyInd.cols,energyInd.rows), CV_32SC1);
		puts("Done");
		energyCumVer(energyInd,"energyCum",false,false,energyCum);
		puts("Allocating..");
		Mat retLocal(im.rows,im.cols-cnt,CV_8UC1);
		puts("Done.");

		int min = INT_MAX;
		int minInd = 0;
		for(int j = 1; j < smaller.cols-1; ++j){
			if(min>energyCum.at<int>(smaller.rows-2,j)){
				min = energyCum.at<int>(smaller.rows-2,j);
				minInd = j;
			}
		}
		puts("this");
		Point curr(smaller.rows-2,minInd);

		retLocal.at<char>(curr.x,curr.y) = 255;

		for (int i = energyCum.rows-3; i >= 1; --i){
			int j = curr.y;
			curr.x = i;
			int min = INT_MAX;
			if(j!=1){
				if(energyCum.at<int>(i,j-1) < min){
					min = energyCum.at<int>(i,j-1);
					curr.y = j-1;	
				}
			}
			if(j!=energyCum.cols-2){
				if(energyCum.at<int>(i,j+1) < min){
					min = energyCum.at<int>(i,j+1);
					curr.y = j+1;
				}
			}
			if(energyCum.at<int>(i,j) < min)
				curr.y = j;

			retLocal.at<char>(curr.x,curr.y) = 255;
		}
		puts("calling mergeSeams");
		mergeSeams(ret,retLocal);
		puts("returned");
	}
	
	return ret;
}

void seam(Mat im, Mat oldSeamsHor, Mat oldSeamsVer, int numOldSeamsHor, int numOldSeamsVer, int numNewSeamsHor, int numNewSeamsVer){

	Mat newIm1 = removeVerSeams(im.t(),oldSeamsHor,numOldSeamsHor);
	Mat newIm2 = removeVerSeams(im,oldSeamsVer,numOldSeamsVer);

	Mat newSeamsHor = markSeamVer(newIm1, numNewSeamsHor);

// 	Mat newSeamsVer = markSeamVer(newIm2, numNewSeamsVer);
// cout << "asdas" << endl;

// 	// imshow("new1",newSeamsHor);
// 	// imshow("new2",newSeamsVer);

// 	mergeSeams(oldSeamsHor, newSeamsHor);
// 	mergeSeams(oldSeamsVer, newSeamsVer);
// cout << "asdas" << endl;

}

void showSeams(Mat im, Mat seamsHor, Mat seamsVer){
	seamsHor = seamsHor.t();
	Mat displayIm = im.clone();
	for(int i = 0; i < displayIm.rows; ++i){
		for(int j = 0; j < displayIm.cols; ++j){
			if(seamsHor.at<char>(i,j)==255 || seamsVer.at<char>(i,j)==255){
				displayIm.at<Vec3b>(i,j) = Vec3b(0,0,255);
			}
		}
	}
	imshow("seams",displayIm);
	waitKey(1);
}

Mat removeBothSeamsVerFirst(Mat im, Mat seamsHor, Mat seamsVer, int numSeamsHor, int numSeamsVer){
	Mat ret(im.rows,im.cols-numSeamsVer,CV_8UC3);
	Mat newSeamsHor(im.rows,im.cols-numSeamsVer,CV_8UC1);

	for(int i = 0; i < im.rows; ++i){
		int counter = 0;
		for(int j = 0; j < im.cols; ++j){
			if(seamsVer.at<char>(i,j)!=255){
				ret.at<Vec3b>(i,counter) = im.at<Vec3b>(i,j);
				newSeamsHor.at<char>(i,counter) = seamsHor.at<char>(i,j);
				++counter;
			}
		}
	}

	Mat retFinal(im.rows-numSeamsHor,im.cols-numSeamsVer,CV_8UC3);

	for(int j = 0; j < ret.cols; ++j){
		int counter = 0;
		for(int i = 0; i < ret.rows; ++i){
			if(newSeamsHor.at<char>(i,j)!=255){
				retFinal.at<Vec3b>(counter,j) = ret.at<Vec3b>(i,j);
				++counter;
			}
		}
	}

	return retFinal;
}

Mat removeBothSeamsHorFirst(Mat im, Mat seamsHor, Mat seamsVer, int numSeamsHor, int numSeamsVer){
	Mat ret(im.rows-numSeamsHor,im.cols,CV_8UC3);
	Mat newSeamsVer(im.rows-numSeamsHor,im.cols,CV_8UC1);

	for(int j = 0; j < im.cols; ++j){
		int counter = 0;
		for(int i = 0; i < im.rows; ++i){
			if(seamsHor.at<char>(i,j)!=255){
				ret.at<Vec3b>(counter,j) = im.at<Vec3b>(i,j);
				newSeamsVer.at<char>(counter,j) = seamsVer.at<char>(i,j);
				++counter;
			}
		}
	}

	Mat retFinal(im.rows-numSeamsHor,im.cols-numSeamsVer,CV_8UC3);

	for(int i = 0; i < ret.rows; ++i){
		int counter = 0;
		for(int j = 0; j < ret.cols; ++j){
			if(newSeamsVer.at<char>(i,j)!=255){
				retFinal.at<Vec3b>(i,counter) = ret.at<Vec3b>(i,j);
				++counter;
			}
		}
	}

	return retFinal;
}

int main(int argc, char** argv){
	Mat im = imread(argv[1]);
	Mat empty1(im.rows,im.cols,CV_8UC1,0);
	Mat empty2(im.rows,im.cols,CV_8UC1,0);
	seam(im,empty1,empty2,0,0,10,10);
	showSeams(im,empty1,empty2);
	// imshow("compressed",removeBothSeamsVerFirst(im,empty1,empty2,10,10));
	waitKey(0);
}