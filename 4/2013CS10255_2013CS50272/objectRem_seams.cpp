#include "seam.cpp"
#include <set>

Mat imInput;
bool clicked = false;
set<pair<int,int> > pointsToRem;
int numRowRem = 0, numColRem = 0;


void getPointsToRem(){
	//update pointsToRem
	//dont't take points on the boundary
	waitKey(0);
}

Mat getPointsToRem_Mat(int rows, int cols){
	Mat ret = Mat::zeros(Size(cols,rows), CV_8UC1);
	set<pair<int,int> >::iterator it;
	for(it=pointsToRem.begin(); it!=pointsToRem.end(); ++it){
		ret.at<uchar>((*it).first, (*it).second) = 255;
	}
	imshow("points to remove", ret);
	waitKey(1);
	return ret;
}

void modifyEnergy(Mat& energyInd){
	//use pointsToRem
	set<pair<int,int> >::iterator it;
	for(it=pointsToRem.begin(); it!=pointsToRem.end(); ++it){
		pair<int,int> p = *it;
		energyInd.at<int>(p.first,p.second) = -10000;
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

void CallBackFunc(int event, int x, int y, int flags, void* userdata){
	if  ( event == EVENT_LBUTTONDOWN )
	{ 
		if(x>=1 && x<=imInput.cols-2 && y>=1 && y<=imInput.rows-2){
			// cout <<  x << " " << y << endl;
		pair<int,int> p(y,x);
		pointsToRem.insert(p);
		Scalar color(0, 0, 255);
		circle( imInput, Point(x,y), 2, color, FILLED, LINE_8, 0 );
		imshow("Input", imInput);

		}
		clicked = true;
	}
	else if ( event == EVENT_LBUTTONUP){
    	clicked = false;
    }
	else if (event == EVENT_MOUSEMOVE){
		if(clicked){
			if(x>=1 && x<=imInput.cols-2 && y>=1 && y<=imInput.rows-2){
				pair<int,int> p(y,x);
				pointsToRem.insert(p);
				Scalar color(0, 0, 255);
				circle( imInput, Point(x,y), 2, color, FILLED, LINE_8, 0 );
				imshow("Input", imInput);			
			}
		}
	}
}

int main(int argc, char *argv[]){
	if(argc!=2){
		cout << "Usage: img.exe <imgName>" << endl;
        exit(0);
	}
	Mat im = imread(argv[1]);
	imshow("1. Original",im);
	imInput = im.clone();
	cout << "Size " << im.rows << " " << im.cols << endl;
	imshow("Input", imInput);
	setMouseCallback("Input", CallBackFunc, NULL);
	process(im);

	waitKey(0);
}