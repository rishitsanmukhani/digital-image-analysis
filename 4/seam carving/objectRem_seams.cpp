#include "seam.cpp"

vector<pair<int,int> > pointsToRem;
int numRowRem = 0, numColRem = 0;


void getPointsToRem(){
	//update pointsToRem
	//dont't take points on the boundary
	for(int i = 320; i < 370; ++i){
		for(int j = 40; j < 80; ++j){
			pointsToRem.push_back(make_pair(i,j));
		}
	}
}

Mat getPointsToRem_Mat(int rows, int cols){
	Mat ret = Mat::zeros(Size(cols,rows), CV_8UC1);
	for(int i = 0; i < pointsToRem.size(); ++i){
		ret.at<uchar>(pointsToRem[i].first, pointsToRem[i].second) = 255;
	}
	imshow("points to remove", ret);
	waitKey(1);
	return ret;
}

void modifyEnergy(Mat& energyInd){
	//use pointsToRem
	for(int i = 0; i < pointsToRem.size(); ++i){
		pair<int,int> p = pointsToRem[i];
		energyInd.at<int>(p.first,p.second) = -1000;
	}
}

void process(Mat& im){

	getPointsToRem();
	Mat PointsToRem_Mat_Ver = getPointsToRem_Mat(im.rows,im.cols);
	Mat PointsToRem_Mat_Hor = PointsToRem_Mat_Ver.t();
	Mat energyInd = energyIndivisual(im,"Ind-Energy",false,true);
	modifyEnergy(energyInd);

	//vertical
	Mat energyCumVer_ = energyCumVer(energyInd,"Cum-Energy Vertical",false,true);
	pair<Mat,int> vertical = removeVerSeams(im, energyCumVer_, PointsToRem_Mat_Ver, pointsToRem.size());
	// horizontal
	Mat energyCumHor_ = energyCumVer(energyInd.t(),"Cum-Energy Horizontal",false,true);
	pair<Mat,int> horizontal = removeVerSeams(im.t(), energyCumHor_, PointsToRem_Mat_Hor, pointsToRem.size());

	if(vertical.second < horizontal.second){
		imshow("2. Object removed", vertical.first);
		Mat reconstructed = expandCol(vertical.first,vertical.second);
		imshow("4. Reconstructed",reconstructed);
	}
	else{
		imshow("2. Object removed", horizontal.first.t());
		Mat reconstructed = expandCol(horizontal.first,horizontal.second);
		imshow("3. Reconstructed",reconstructed.t());
	}

	waitKey(1);

}

int main(int argc, char *argv[]){
	if(argc!=2){
		cout << "Usage: img.exe <imgName>" << endl;
        exit(0);
	}
	Mat im = imread(argv[1]);
	imshow("1. Original",im);
	process(im);

	waitKey(0);
}