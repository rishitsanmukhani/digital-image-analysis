#include "C:\opencv\build\include\opencv2\highgui\highgui.hpp"
#include "C:\opencv\build\include\opencv2\imgproc\imgproc_c.h"
#include "C:\opencv\build\include\opencv2\imgproc\imgproc.hpp"
#include "C:\opencv\build\include\opencv2\core\core_c.h"

#include <iostream>
#include <cmath>        // std::abs
#include <string>
#include <vector>
#include <utility> 

using namespace cv;
using namespace std;

int numClusters1D = 4;
int numClusterTotal;
int** clusterAvg;
int sizex,sizey;
int* clusterFreq;
pair<int,int>** pixelCluster;
int numClustersTotalFinal;

vector<pair<int,int> > noisePoints;
int noiseAreaParam = 40;

class UF    {
public:
	int *id, cnt, *sz, total;
	// Create an empty union find data structure with N isolated sets.
    UF(int N)   {
    	total = N;
        cnt = N;
		id = new int[N];
		sz = new int[N];
		for(int i=0; i<N; i++){
		    id[i] = i;
		    sz[i] = 1;
		}
    }
    ~UF()	{
	delete [] id;
	delete [] sz;
    }
	// Return the id of component corresponding to object p.
    int find(int p)	{
        int root = p;
        while (root != id[root])
            root = id[root];
        while (p != root) {
            int newp = id[p];
            id[p] = root;
            p = newp;
        }
        return root;
    }
	// Replace sets containing x and y with their union.
    void merge(int x, int y)	{
        int i = find(x);
        int j = find(y);
        if (i == j) return;
		
		// make smaller root point to larger one
        if   (sz[i] < sz[j])	{ 
		id[i] = j; 
		sz[j] += sz[i]; 
	} else	{ 
		id[j] = i; 
		sz[i] += sz[j]; 
	}
        cnt--;
    }
	// Are objects x and y in the same set?
    bool connected(int x, int y)    {
        return find(x) == find(y);
    }
	// Return the number of disjoint sets.
    int count() {
        return cnt;
    }
};

bool xOp(const pair<int,int> &left, const pair<int,int> &right) {
    return left.first < right.first;
}

bool yOp(const pair<int,int> &left, const pair<int,int> &right) {
    return left.second < right.second;
}

bool xyOp45(const pair<int,int> &left, const pair<int,int> &right) {
    return (left.first + left.second) < (right.first + right.second);
}

bool xyOp135(const pair<int,int> &left, const pair<int,int> &right) {
    return (left.first - left.second) < (right.first - right.second);
}

class region{
public:
	vector<pair<int,int> > points;
	void ploth(Mat &im, int gap){
		sort(points.begin(),points.end(),xOp);
		if (points.size()>0){
			int xCoord = points[0].first;
			int pos = 1;
			while(pos < points.size()){
				while(xCoord==points[pos].first){
					im.at<unsigned char>(xCoord,points[pos].second) = 255;
					// im.at<cv::Vec3b>(xCoord,points[pos].second)[1] = 255;
					// im.at<cv::Vec3b>(xCoord,points[pos].second)[2] = 255;
					++pos;
					if (pos==points.size()) break;
				}
				if (pos==points.size()) break;
				while(points[pos].first-xCoord < gap){
					++pos;
					if (pos==points.size()) break;
				}
				if (pos < points.size()) xCoord = points[pos].first;
			}
		}
	}
	void plotv(Mat &im, int gap){
		sort(points.begin(),points.end(),yOp);
		if (points.size()>0){
			int yCoord = points[0].second;
			int pos = 1;
			while(pos < points.size()){
				while(yCoord==points[pos].second){
					im.at<unsigned char>(points[pos].first,yCoord) = 255;
					// im.at<cv::Vec3b>(points[pos].first,yCoord)[1] = 255;
					// im.at<cv::Vec3b>(points[pos].first,yCoord)[2] = 255;;
					++pos;
					if (pos==points.size()) break;
				}
				if (pos==points.size()) break;
				while(points[pos].second-yCoord < gap){
					++pos;
					if (pos==points.size()) break;
				}
				if (pos < points.size()) yCoord = points[pos].second;
			}
		}
	}
	void plotd45(Mat &im, int gap){
		sort(points.begin(),points.end(),xyOp45);
		if (points.size()>0){
			int xCoord = points[0].first;
			int yCoord = points[0].second;
			int pos = 1;
			while(pos < points.size()){
				while(xCoord+yCoord==points[pos].first+points[pos].second){
					im.at<unsigned char>(points[pos].first,points[pos].second) = 255;
					// im.at<cv::Vec3b>(points[pos].first,points[pos].second)[1] = 255;
					// im.at<cv::Vec3b>(points[pos].first,points[pos].second)[2] = 255;
					++pos;
					if (pos==points.size()) break;
				}
				if (pos==points.size()) break;
				while((points[pos].first+points[pos].second)-(xCoord+yCoord) < gap){
					++pos;
					if (pos==points.size()) break;
				}
				if (pos < points.size()){
					xCoord = points[pos].first;
					yCoord = points[pos].second;
				}
			}
		}
	}
	void plotd135(Mat &im, int gap){
		sort(points.begin(),points.end(),xyOp135);
		if (points.size()>0){
			int xCoord = points[0].first;
			int yCoord = points[0].second;
			int pos = 1;
			while(pos < points.size()){
				while(xCoord-yCoord==points[pos].first-points[pos].second){
					im.at<unsigned char>(points[pos].first,points[pos].second) = 255;
					// im.at<cv::Vec3b>(points[pos].first,points[pos].second)[1] = 255;
					// im.at<cv::Vec3b>(points[pos].first,points[pos].second)[2] = 255;
					++pos;
					if (pos==points.size()) break;
				}
				if (pos==points.size()) break;
				while((points[pos].first-points[pos].second)-(xCoord-yCoord) < gap){
					++pos;
					if (pos==points.size()) break;
				}
				if (pos < points.size()){
					xCoord = points[pos].first;
					yCoord = points[pos].second;
				}
			}
		}
	}
	void plotPattern(Mat& im, int i){
		if (i==0){
			ploth(im, 2);
		}
		else if (i==1){
			plotv(im, 2);
		}
		else if (i==2){
			plotd45(im, 2);
		}
		else if (i==3){
			plotd135(im, 2);
		}
		else if (i==4){
			ploth(im, 2);
			plotv(im, 2);
		}
		else if (i==5){
			plotd45(im, 2);
			plotd135(im, 2);
		}
		else if (i==6){
			ploth(im, 4);
		}
		else if (i==7){
			plotv(im, 4);
		}
		else if (i==8){
			plotd45(im, 4);
		}
		else if (i==9){
			plotd135(im, 4);
		}
	}

	void plotSilhoutte(Mat& im){
		for(int i = 0; i < points.size(); ++i){
			im.at<unsigned char>(points[i].first,points[i].second) = 255;
		}
	}
};

region* regionsAll;

bool regionOp(const region &left, const region &right) {
    return left.points.size() > right.points.size();
}

int nearestInArray(int r, int g, int b){
	int bestIndex = 0;
	int bestDev = 1000;
	for (int i = 0; i < numClusterTotal; i++){
		int dev = 0;
		dev += abs(r-clusterAvg[i][0]);
		dev += abs(g-clusterAvg[i][1]);
		dev += abs(b-clusterAvg[i][2]);
		if (dev<bestDev){
			bestDev = dev;
			bestIndex = i;
		}
	}
	return bestIndex;
}

void initializeClusters(Mat& im){
	for (int k = 0; k < numClusters1D*numClusters1D; k++){
	  
	  int iStart = ((k/numClusters1D)*sizex)/numClusters1D;
	  int jStart = ((k%numClusters1D)*sizey)/numClusters1D;
	  
	 //cout << iStart << endl;
	 //cout << jStart << endl;
	  
	  long long rSum = 0;
	  long long gSum = 0;
	  long long bSum = 0;
	  for (int i = iStart; i < iStart+(sizex/numClusters1D); i++){
		for (int j= jStart; j < jStart+(sizey/numClusters1D); j++){
			rSum += im.at<cv::Vec3b>(i,j)[0];
			gSum += im.at<cv::Vec3b>(i,j)[1];
			bSum += im.at<cv::Vec3b>(i,j)[2];
		}
	  }
	  clusterAvg[k][0] = numClusterTotal*rSum/(sizex*sizey);
	  clusterAvg[k][1] = numClusterTotal*gSum/(sizex*sizey);
	  clusterAvg[k][2] = numClusterTotal*bSum/(sizex*sizey);
	  
	  //cout << clusterAvg[k][0] << endl;
	  //cout << clusterAvg[k][1] << endl;
	  //cout << clusterAvg[k][2] << endl << endl;
  }
}

void mergeClusters(){
	int count = 0;
	for (int i = 0; i < numClusterTotal; i++){
		for (int j = 0; j < i; j++){
			if(clusterAvg[i][0] != -1 && clusterAvg[j][0] != -1){
				// float v1 = 0.21*clusterAvg[i][0] + 0.72*clusterAvg[i][1] + 0.07*clusterAvg[i][2];
				// float v2 = 0.21*clusterAvg[j][0] + 0.72*clusterAvg[j][1] + 0.07*clusterAvg[j][2];
				// int dev = abs(v1-v2);
				int dev = abs(clusterAvg[i][0]-clusterAvg[j][0]);
				dev += abs(clusterAvg[i][1]-clusterAvg[j][1]);
				dev += abs(clusterAvg[i][2]-clusterAvg[j][2]);
				if (dev < 120){
				// if (dev < (2*255/numClusterTotal)){
					clusterAvg[i][0] = (clusterFreq[i]*clusterAvg[i][0] + clusterFreq[j]*clusterAvg[j][0])/(clusterFreq[i]+clusterFreq[j]);
					clusterAvg[i][1] = (clusterFreq[i]*clusterAvg[i][1] + clusterFreq[j]*clusterAvg[j][1])/(clusterFreq[i]+clusterFreq[j]);
					clusterAvg[i][2] = (clusterFreq[i]*clusterAvg[i][2] + clusterFreq[j]*clusterAvg[j][2])/(clusterFreq[i]+clusterFreq[j]);
					clusterFreq[i]+=clusterFreq[j];
					clusterAvg[j][0] = clusterAvg[numClusterTotal-count-1][0];
					clusterAvg[j][1] = clusterAvg[numClusterTotal-count-1][1];
					clusterAvg[j][2] = clusterAvg[numClusterTotal-count-1][2];
					clusterFreq[j] = clusterFreq[numClusterTotal-count-1];
					--j;		
					clusterAvg[numClusterTotal-count-1][0] = -1;
					count++;
				}
			}
			else break;
		}
	}
	numClusterTotal-=count;
}

//Splits cluster if atleast 2 parts of it are unconnected, also identifies noises
void splitClusters(region* p){
	UF uf(p->points.size());
	// sort(p->points.begin(),p->points.end(),xOp);
	for (int i = 0; i < p->points.size(); i++){
		int currentx = p->points[i].first;
		int currenty = p->points[i].second;
		int currentCluster = pixelCluster[currentx][currenty].first;
		int indInCluster = pixelCluster[currentx][currenty].second;
		if (currentx+1<sizex){
			if (currenty>0)
				if (pixelCluster[currentx+1][currenty-1].first==currentCluster) uf.merge(indInCluster,pixelCluster[currentx+1][currenty-1].second);
			if (pixelCluster[currentx+1][currenty].first==currentCluster) uf.merge(indInCluster,pixelCluster[currentx+1][currenty].second);
			
		}
		if (currenty+1 < sizey){
			for(int j = -1; j != 2; ++j){
				if (currentx+j>=0 && currentx+j<sizex)
					if (pixelCluster[currentx+j][currenty+1].first==currentCluster) uf.merge(indInCluster,pixelCluster[currentx+j][currenty+1].second);
			}
		}
	}
	// cout << "union of " << p->points.size() << " to " << uf.count() << endl;

	if(uf.count()>1){
		int indMax1 = 0, indMax2;
		int max1 = uf.sz[0], max2;
		if(uf.sz[1]>max1){
			max1 = uf.sz[1];
			indMax1 = 1;
			max2 = uf.sz[0];
			indMax2 = 0;
		}
		else{
			max2 = uf.sz[1];
			indMax2 = 1;
		}
		for (int i = 2; i < uf.total; i++){
			if(uf.sz[i]>=max1){
				max2 = max1;
				indMax2 = indMax1;
				max1 = uf.sz[i];
				indMax1 = i;
			}
			else if(uf.sz[i]<=max2){;}
			else{
				max2 = uf.sz[i];
				indMax2 = i;
			}
		}

		if (max2 > uf.total/10){
			// cout << "Splitting..........." << endl;
			numClustersTotalFinal+=2;
			vector<pair<int,int> > v;
			for(int i = 0; i < uf.total; ++i){
				if(uf.sz[uf.find(i)]<noiseAreaParam) noisePoints.push_back(p->points[i]);
				else if(uf.connected(i,indMax1)) regionsAll[numClustersTotalFinal-2].points.push_back(p->points[i]);
				else if(uf.connected(i,indMax2)) regionsAll[numClustersTotalFinal-1].points.push_back(p->points[i]);
				else v.push_back(p->points[i]);
			}
			p->points.clear();
			p->points = v;
		} 
		else{
			for(int i = 0; i < uf.total; ++i)
				if(uf.sz[uf.find(i)]<noiseAreaParam) noisePoints.push_back(p->points[i]);
		}
	}
}

void noiseRemoval(Mat& segmentedImage){
	sort(noisePoints.begin(),noisePoints.end(),xOp);
	for(int l = 0; l < noisePoints.size(); l++){
		for(int i = -1; i < 2; i++){
			if(noisePoints[l].first+i < sizex && noisePoints[l].first+i>=0){
				for(int j = -1; j < 2; j++){
					if(noisePoints[l].second+j < sizey && noisePoints[l].second+j>=0){
						if(segmentedImage.at<cv::Vec3b>(noisePoints[l].first+i,noisePoints[l].second+j)[0]!=0 ||
							segmentedImage.at<cv::Vec3b>(noisePoints[l].first+i,noisePoints[l].second+j)[1]!=0 ||
							segmentedImage.at<cv::Vec3b>(noisePoints[l].first+i,noisePoints[l].second+j)[2]!=0){
							segmentedImage.at<cv::Vec3b>(noisePoints[l].first,noisePoints[l].second)=
							segmentedImage.at<cv::Vec3b>(noisePoints[l].first+i,noisePoints[l].second+j);
						}
					}
				}
			}
		}
	}
}

void iterate(Mat& im, Mat& segmentedImage, Mat& patternImage, Mat& silhoutteImage, bool toPrint){

	int** colors;

	if(toPrint){
		cout << "Meging Clusters" << endl;
		mergeClusters();
		regionsAll = new region[3*numClusters1D*numClusters1D+1];
		numClustersTotalFinal = numClusterTotal;
	}

	for (int i = 0; i < numClusterTotal; i++){
		clusterFreq[i] = 0;
	}
	int **clusterTotalNew = new int*[numClusterTotal];
	for (int i = 0; i < numClusterTotal; i++){
	  clusterTotalNew[i] = new int[3];
	  clusterTotalNew[i][0] = 0;
	  clusterTotalNew[i][1] = 0;
	  clusterTotalNew[i][2] = 0;
	  clusterFreq[i] = 0;
	}
	
	int r,g,b,nearestIndex;
	
	for (int i = 0; i < sizex; i++){
		for (int j = 0; j < sizey; j++){
			r = im.at<cv::Vec3b>(i,j)[0];
			g = im.at<cv::Vec3b>(i,j)[1];
			b = im.at<cv::Vec3b>(i,j)[2];
			nearestIndex = nearestInArray(r,g,b);
			clusterTotalNew[nearestIndex][0] += r;
			clusterTotalNew[nearestIndex][1] += g;
			clusterTotalNew[nearestIndex][2] += b;
			clusterFreq[nearestIndex]++;
			if (toPrint){
				regionsAll[nearestIndex].points.push_back(pair<int,int>(i,j));
				pixelCluster[i][j].first = nearestIndex;
				pixelCluster[i][j].second = regionsAll[nearestIndex].points.size()-1;
				
				// im.at<cv::Vec3b>(i,j)[2] = colors[nearestIndex][0];
				// im.at<cv::Vec3b>(i,j)[1] = colors[nearestIndex][1];
				// im.at<cv::Vec3b>(i,j)[0] = colors[nearestIndex][2];
			
			// im.at<cv::Vec3b>(i,j)[0] = (255*nearestIndex)/numClusterTotal;
			// im.at<cv::Vec3b>(i,j)[1] = (255*nearestIndex)/numClusterTotal;
			// im.at<cv::Vec3b>(i,j)[2] = (255*nearestIndex)/numClusterTotal;
			}
		}
	}

	if(toPrint){
		cout << "Splitting Clusters" << endl;
		for(int i = 0; i < numClusterTotal; ++i)
			splitClusters(&regionsAll[i]);

		//Finding the background for Silhoutte and preparing the Pattern Image
		int backgroundClusterIndex = 0;
		int max = regionsAll[0].points.size();
		sort(regionsAll,regionsAll+3*numClusters1D*numClusters1D+1,regionOp);
		for (int i = 0; i < numClustersTotalFinal; i++){
			regionsAll[i].plotPattern(patternImage,i);
			if(regionsAll[i].points.size()>max){
				backgroundClusterIndex = i;
				max = regionsAll[i].points.size();
			}
			cout << "Cluster " << i << " size " << regionsAll[i].points.size() << endl;
		}

		//Defining the Mapping colors
		colors = new int*[numClustersTotalFinal];
		for (int i = 0; i < numClustersTotalFinal; ++i){
			colors[i] = new int[3];
			int numRG = numClustersTotalFinal/3;
			int gradRG = 255/numRG;
			int numB = numClustersTotalFinal-2*numRG;
			int gradB = 255/numB;
			if(i<numRG){
				colors[i][0] = (i+1)*gradRG;
				colors[i][1] = 0;
				colors[i][2] = 0;
			}
			else if(i<2*numRG){
				colors[i][1] = (i-numRG+1)*gradRG;
				colors[i][0] = 0;
				colors[i][2] = 0;
			}
			else{
				colors[i][2] = (i-2*numRG+1)*gradB;
				colors[i][0] = 0;
				colors[i][1] = 0;
			}
		}

		//Mapping the colors
		for (int i = 0; i < numClustersTotalFinal; i++){
			for (int j = 0; j < regionsAll[i].points.size(); ++j){
				segmentedImage.at<cv::Vec3b>(regionsAll[i].points[j].first, regionsAll[i].points[j].second)[0] = colors[i][0];
				segmentedImage.at<cv::Vec3b>(regionsAll[i].points[j].first, regionsAll[i].points[j].second)[1] = colors[i][1];
				segmentedImage.at<cv::Vec3b>(regionsAll[i].points[j].first, regionsAll[i].points[j].second)[2] = colors[i][2];
			}
			
			if (i!=backgroundClusterIndex)
				regionsAll[i].plotSilhoutte(silhoutteImage);
		}

		cout << "Removing Noise" << endl;
		noiseRemoval(segmentedImage);
	}


	
	for (int i = 0; i < numClusterTotal; i++){
		if (clusterFreq[i]!=0){
			// cout << "cluster no " << i << endl;
			clusterAvg[i][0] = clusterTotalNew[i][0]/clusterFreq[i];
			clusterAvg[i][1] = clusterTotalNew[i][1]/clusterFreq[i];
			clusterAvg[i][2] = clusterTotalNew[i][2]/clusterFreq[i];
			// if(toPrint){
			// 	cout << clusterFreq[i] << endl;
			// 	cout << clusterAvg[i][2] << endl;
			// 	cout << clusterAvg[i][1] << endl;
			// 	cout << clusterAvg[i][0] << endl;
				// cout << "Mapped to b=" << colors[i][0] << " g=" << colors[i][1] << " r=" << colors[i][2] << endl<< endl;
			// }
		}
	}

	// if(toPrint){
	// 	cout << "after splitting " << endl;
	// 	for (int i = 0; i < numClustersTotalFinal; i++){
	// 		cout << i <<" mapped to b=" << colors[i][0] << " g=" << colors[i][1] << " r=" << colors[i][2] << endl<< endl;
	// 	}
	// }
}



int main(int argc, char** argv)
{
	Mat im1, edgeCanny;
	if (argc>1){
		im1 = imread(argv[1],CV_LOAD_IMAGE_UNCHANGED);
	}
	else{
		cout << "enter image name" << endl;
	}

	// cout << im1.rows << " " << im1.cols << endl;
	//resize(im, im, Size(im.cols/4, im.rows/4));
	if (im1.empty())
	{
		cout << "Cannot open image!" << endl;
		return -1;
	}
	Mat im;
	blur(im1,im,Size(3,3));
	imshow("Original Image",im1);
	imshow("Blurred Image",im);

	sizex = im.rows;
	sizey = im.cols;

	Mat segmentedImage(sizex, sizey, CV_8UC3);
	Mat patternImage(sizex, sizey, CV_8UC1);
	Mat silhoutteImage(sizex, sizey, CV_8UC1);

	numClusterTotal = numClusters1D*numClusters1D;
	clusterAvg = new int*[numClusterTotal];
	for (int i = 0; i < numClusterTotal; i++){
		clusterAvg[i] = new int[3];
	}

	clusterFreq = new int[numClusterTotal];
	pixelCluster = new pair<int,int>*[sizex];
	for(int i = 0; i < sizex; ++i){
		pixelCluster[i] = new pair<int,int>[sizey];
	}

	initializeClusters(im);
	cout << "Clusters Initialised" << endl;
	iterate(im,segmentedImage,patternImage,silhoutteImage,false);
	iterate(im,segmentedImage,patternImage,silhoutteImage,false);
	iterate(im,segmentedImage,patternImage,silhoutteImage,false);
	iterate(im,segmentedImage,patternImage,silhoutteImage,false);
	iterate(im,segmentedImage,patternImage,silhoutteImage,false);
	iterate(im,segmentedImage,patternImage,silhoutteImage,false);
	iterate(im,segmentedImage,patternImage,silhoutteImage,false);

	iterate(im,segmentedImage,patternImage,silhoutteImage,true);

	// cv::Canny(im,edgeCanny,35,90);
	imshow("Segmented Image", segmentedImage);
	imshow("Pattern Mapping", patternImage);
	imshow("Silhoutte", silhoutteImage);
	imwrite("pattern.bmp",patternImage);
	imwrite("segments.bmp",segmentedImage);
	imwrite("silhoutte.bmp",silhoutteImage);
	// cout << noisePoints.size() << endl;

	// imshow("4",edgeCanny);

	waitKey(0);

	return 0;
}
