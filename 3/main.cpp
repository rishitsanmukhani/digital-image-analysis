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

int numRowsToDelete, numColToDelete, currEnergy=0;


bool** choiceDP;
pair<int,Mat>** DP;


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

Mat energyIndivisual(Mat im, bool toDisplay){
	Mat M(im.rows,im.cols, CV_32SC1);
	Mat energyimage(im.rows,im.cols, CV_8UC1);
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


	maxima /= 256;
	if(maxima==0) maxima=1;
	for(int i = 1; i < im.rows-1; ++i){
		for (int j = 1; j < im.cols-1; ++j){
				energyimage.at<char>(i,j) = M.at<int>(i,j)/maxima;
		}
	}

	if(toDisplay){
		imshow("energyInd",energyimage);
		waitKey(1);
	}

	return M;
}

Mat energyCumRow(Mat energyInd, string name, bool displayTranspose, bool toDisplay){
	Mat M(energyInd.rows,energyInd.cols, CV_32SC1); // actual
	Mat cumEnergy(energyInd.rows,energyInd.cols, CV_8UC1); // just for display
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

	maxima/=256;
	if(maxima==0) maxima=1;
	for(int i = 1; i < energyInd.rows-1; ++i){
		for (int j = 1; j < energyInd.cols-1; ++j){
				cumEnergy.at<char>(i,j) = M.at<int>(i,j)/maxima;
		}
	}

	if(toDisplay){
		if(displayTranspose)
			imshow(name.c_str(),cumEnergy.t());
		else
			imshow(name.c_str(),cumEnergy);	
	}

	return M;
}

bool comp(pair<int,int> p1, pair<int,int> p2){
	return(p1.first < p2.first);
}

Mat markSeamRow(Mat im, Mat energyCum){

	vector<pair<int,int> > lastRow; //pair - (energy,columnNumber)
	for(int j = 1; j < im.cols-1; ++j){
		lastRow.push_back(make_pair(energyCum.at<int>(im.rows-2,j),j));
	}
	sort(lastRow.begin(),lastRow.end(),comp);

	for (int count = 0; count < lastRow.size()/2; ++count){
		Point curr(im.rows-2,lastRow[count].second);
		
		im.at<Vec3b>(curr.x,curr.y) = Vec3b(0,0,255);
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

			im.at<Vec3b>(curr.x,curr.y) = Vec3b(0,0,255);
		}
	}
	
	return im;
}

void showEnergyAndSeams(Mat im){
	Mat energyInd = energyIndivisual(im,true);
	Mat energyCumRow_ = energyCumRow(energyInd,"energyCumRow_",false,true);
	Mat energyCumCol_ = energyCumRow(energyInd.t(),"energyCumCol_",true,true);

	Mat markedRow = markSeamRow(im.clone(),energyCumRow_);
	imshow("markedRow",markedRow);
	Mat markedCol = markSeamRow(im.clone().t(),energyCumCol_);
	imshow("markedCol",markedCol.t());
	Mat markedAll = markSeamRow(markedRow.t(),energyCumCol_);
	imshow("markedAll",markedAll.t());
}

//returns (value of seam, compressed mat)
pair<int,Mat> removeCol(Mat im, string name, bool displayTranspose){

	Mat energyCumRow_ = energyCumRow(energyIndivisual(im,false),name,displayTranspose,false);

	Mat newIm(im.rows,im.cols-1, CV_8UC3);

	Point minPt(im.rows-1,1);
	int min = energyCumRow_.at<int>(im.rows-2,1);
	for(int j = 2; j < im.cols-1; ++j){
		if(min > energyCumRow_.at<int>(im.rows-2,j)){
			minPt.y = j;
			min = energyCumRow_.at<int>(im.rows-2,j);
		}
	}
	int energySeam = min;

	int currCol = 0;
	for(int j = 0; j < im.cols; ++j){
		if(j!=minPt.y){
			newIm.at<Vec3b>(im.rows-1,currCol) = im.at<Vec3b>(im.rows-1,j);
			++currCol;
		}
	}

	for (int i = energyCumRow_.rows-2; i >= 0; --i){
		int j = minPt.y;
		minPt.x = i;
		int min = INT_MAX;
		if(j!=1){
			if(energyCumRow_.at<int>(i,j-1) < min){
				min = energyCumRow_.at<int>(i,j-1);
				minPt.y = j-1;	
			}
		}
		if(j!=energyCumRow_.cols-2){
			if(energyCumRow_.at<int>(i,j+1) < min){
				min = energyCumRow_.at<int>(i,j+1);
				minPt.y = j+1;
			}
		}
		if(energyCumRow_.at<int>(i,j) < min)
			minPt.y = j;

		int currCol = 0;
		for(int col = 0; col < im.cols; ++col){
			if(col!=minPt.y){
				newIm.at<Vec3b>(i,currCol) = im.at<Vec3b>(i,col);
				++currCol;
			}
		}
	}

	return make_pair(energySeam,newIm);
}

void findPath(Mat im){
	choiceDP = new bool*[numRowsToDelete+1];
	for(int i = 0; i < numRowsToDelete+1; ++i){
		choiceDP[i] = new bool[numColToDelete+1];
	}
	DP = new pair<int,Mat>*[2];
	for(int i = 0; i < 2; ++i){
		DP[i] = new pair<int,Mat>[numColToDelete+1];
	}

	//intializing DP
	DP[0][0] = make_pair(0,im);
	for(int j = 1; j < numColToDelete+1; ++j){
		pair<int,Mat> M = removeCol(DP[0][j-1].second.clone(),"DP",false);
		DP[0][j] = make_pair(DP[0][j-1].first+M.first, M.second);
		choiceDP[0][j] = 0;
	}
	for(int i = 1; i < numRowsToDelete+1; ++i){
		choiceDP[i][0] = 1;
	}
	pair<int,Mat> M = removeCol(DP[0][0].second.clone().t(),"DP",true);

	M.second = M.second.t();
	DP[1][0] = make_pair(DP[0][0].first+M.first, M.second);

	int currRow = 1;
	int predRow = 0;
	for (int i = 1; i < numRowsToDelete+1; ++i){
		currRow = i%2;
		predRow = (currRow+1)%2;
		for(int j = 1; j < numColToDelete+1; ++j){
			pair<int,Mat> M0 = removeCol(DP[currRow][j-1].second.clone(),"DP",false);
			pair<int,Mat> M1 = removeCol(DP[predRow][j].second.clone().t(),"DP",true);
			M1.second = M1.second.t();

			if(M0.first < M1.first){
				DP[currRow][j] = make_pair(DP[currRow][j-1].first+M0.first, M0.second);
				choiceDP[i][j] = 0;
			}
			else{
				DP[currRow][j] = make_pair(DP[predRow][j].first+M1.first, M1.second);
				choiceDP[i][j] = 1;
			}
		}
	}
}

void compressWithDP(Mat im){
	findPath(im);
	/*
	for(int i = 0; i < numRowsToDelete+1; ++i){
		for(int j = 0; j < numColToDelete+1; ++j){
			cout << choiceDP[i][j] << " ";
		}
		cout << endl;
	}
	*/
	int currRow = numRowsToDelete;
	int currCol = numColToDelete;
	Mat compressedIm = im.clone();
	for(int count = 0; count < numRowsToDelete+numColToDelete; ++count){
		if(choiceDP[currRow][currCol] == 0){
			compressedIm = removeCol(compressedIm,"compressing",false).second;
			currCol--;
		}
		else{
			compressedIm = removeCol(compressedIm.t(),"compressing",true).second;
			compressedIm = compressedIm.t();
			currRow--;
		}
		imshow("Compressing image",compressedIm);
		waitKey(10);
	}
}

void comressWithoutDP(Mat im){

	//ROW FIRST
	Mat compRowFirst = im.clone().t();
	for(int count = 0; count < numRowsToDelete; ++count){
		compRowFirst = removeCol(compRowFirst,"energyCumCol_",true).second;
		imshow("compRowFirst",compRowFirst.t());
		waitKey(1);
	}

	compRowFirst = compRowFirst.t();
	for(int count = 0; count < numColToDelete; ++count){
		compRowFirst = removeCol(compRowFirst,"energyCumRow_",false).second;
		imshow("compRowFirst",compRowFirst);
		waitKey(1);
	}

	//COLUMN FIRST
	Mat compColFirst = im.clone();
	for(int count = 0; count < numColToDelete; ++count){
		compColFirst = removeCol(compColFirst,"energyCumRow_",false).second;
		imshow("compColFirst",compColFirst);
		waitKey(1);
	}

	compColFirst = compColFirst.t();
	for(int count = 0; count < numRowsToDelete; ++count){
		compColFirst = removeCol(compColFirst,"energyCumCol_",true).second;
		imshow("compColFirst",compColFirst.t());
		waitKey(1);
	}
}

int main(int argc, char *argv[]){
	if(argc!=4){
		cout << "Usage: img.exe <imgName> <numRowsToDelete> <numColToDelete>" << endl;
        exit(0);
	}
	Mat im = imread(argv[1]);
	istringstream ss1(argv[2]);
	ss1 >> numRowsToDelete;
	istringstream ss2(argv[3]);
	ss2 >> numColToDelete;

	imshow("original image",im);

	cout << "showing energy and seams..." << endl;
	showEnergyAndSeams(im);

	cout << "Compressing without DP..." << endl;
	comressWithoutDP(im);

	cout << "Compressing with DP..." << endl;
	compressWithDP(im);

	cout << "Done!" << endl;

	waitKey(0);
}