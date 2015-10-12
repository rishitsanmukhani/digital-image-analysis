#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc_c.h"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\core\core_c.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace cv;
using namespace std;

class triplet{
public:
    int a,b,c;
    triplet(int A, int B, int C){
        a=A; b=B; c=C;
    }
};

vector<string> files;
Mat imgMorphed;
Mat* imgPath;
int numFaces;
Size size(300,300);

vector<Mat> allImages;

vector<vector<Point> > allFeatures;
vector<Point> featuresAllMorphed;

vector<triplet> trianglesInd;

vector<Point> path;

bool clicked = false;


int min(int a, int b){
	if(a>b) return b;
	else return a;
}

int distance(Point p1,Point p2){
    return(abs(p1.x-p2.x)+abs(p1.y-p2.y));
}

bool inBounds(Point pt){
    return (pt.x < allImages[0].cols && pt.x >= 0 && pt.y < allImages[0].rows && pt.y >= 0);
}

int getIndFrom1(Point p){
    for(int i = 0; i  < allFeatures[0].size(); ++i){
        if(p.x == allFeatures[0][i].x && p.y == allFeatures[0][i].y)
            return i;
    }
}

int getIndFromMorphed(Point p, vector<Point>* featuresMorphed){
    for(int i = 0; i  < featuresMorphed->size(); ++i){
        if(p.x == (*featuresMorphed)[i].x && p.y == (*featuresMorphed)[i].y)
            return i;
    }
    // cout << "getIndFromMorphed couln't find any match " << p.x << " " << p.y << endl;
    return -1;
}

void getTriangleList(Subdiv2D& subdiv){
    vector<Vec6f> triangleList;
    subdiv.getTriangleList(triangleList);
    vector<Point> pt(3);

    for( size_t i = 0; i < triangleList.size(); i++ )
    {
        triplet triInd(-1,-1,-1);
        Vec6f t = triangleList[i];
        pt[0] = Point(cvRound(t[0]), cvRound(t[1]));
        pt[1] = Point(cvRound(t[2]), cvRound(t[3]));
        pt[2] = Point(cvRound(t[4]), cvRound(t[5]));
        if(inBounds(pt[0]))
            triInd.a = getIndFrom1(pt[0]);
        else
            continue;
        if(inBounds(pt[1]))
            triInd.b = getIndFrom1(pt[1]);
        else
            continue;
        if(inBounds(pt[2]))
            triInd.c = getIndFrom1(pt[2]);
        else
            continue;
        trianglesInd.push_back(triInd);
    }
}

int dot2(Point& A, Point& B){
    return (A.x*B.x + A.y*B.y);
}

void CallBackFunc1(int event, int x, int y, int flags, void* userdata){
     if  ( event == EVENT_LBUTTONDOWN )
     { 
     	// cout <<  x << " " << y << endl;
		Point fp(x,y);
		allFeatures[0].push_back(fp);
		Scalar color(0, 0, 255);
		circle( allImages[0], fp, 2, color, FILLED, LINE_8, 0 );
		imshow(files[0].c_str(), allImages[0]);

     }
}

void CallBackFunc2(int event, int x, int y, int flags, void* userdata){
    if  ( event == EVENT_LBUTTONDOWN )
    {
    	// cout <<  x << ", " << y << endl;
		Point fp(x,y);
		allFeatures[1].push_back(fp);
		Scalar color(0, 0, 255);
		circle( allImages[1], fp, 2, color, FILLED, LINE_8, 0 );
		imshow(files[1].c_str(), allImages[1]);
    }
}

void CallBackFunc3(int event, int x, int y, int flags, void* userdata){
    if  ( event == EVENT_LBUTTONDOWN )
    {
        // cout <<  x << ", " << y << endl;
        Point fp(x,y);
        allFeatures[2].push_back(fp);
        Scalar color(0, 0, 255);
        circle( allImages[2], fp, 2, color, FILLED, LINE_8, 0 );
        imshow(files[2].c_str(), allImages[2]);
    }
}

void CallBackFunc4(int event, int x, int y, int flags, void* userdata){
    if  ( event == EVENT_LBUTTONDOWN ){
      circle( *imgPath, Point(x,y), 1, Scalar(0, 0, 255), FILLED, LINE_8, 0 );
      imshow("imgPath", *imgPath);
      clicked = true;
      path.push_back(Point(x,y));
    }
    else if ( event == EVENT_LBUTTONUP){
    	clicked = false;
    }
    else if (event == EVENT_MOUSEMOVE){
    	if(clicked && distance(path[path.size()-1],Point(x,y)) > 5){
			circle( *imgPath, Point(x,y), 1, Scalar(0, 0, 255), FILLED, LINE_8, 0 );
			imshow("imgPath", *imgPath);
            path.push_back(Point(x,y));			
    	}
    }
}

void draw_delaunay1(Scalar delaunay_color){
    vector<Point> pt(3);
    for( int i = 0; i < trianglesInd.size(); i++ )
    {
        pt[0] = allFeatures[0][trianglesInd[i].a];
        pt[1] = allFeatures[0][trianglesInd[i].b];
        pt[2] = allFeatures[0][trianglesInd[i].c];
        line(allImages[0], pt[0], pt[1], delaunay_color, 1, LINE_AA, 0);
        line(allImages[0], pt[1], pt[2], delaunay_color, 1, LINE_AA, 0);
        line(allImages[0], pt[2], pt[0], delaunay_color, 1, LINE_AA, 0);
    }
}

void draw_delaunay2(Scalar delaunay_color){
    vector<Point> pt(3);
    for( int i = 0; i < trianglesInd.size(); i++ )
    {
        pt[0] = allFeatures[1][trianglesInd[i].a];
        pt[1] = allFeatures[1][trianglesInd[i].b];
        pt[2] = allFeatures[1][trianglesInd[i].c];
        line(allImages[1], pt[0], pt[1], delaunay_color, 1, LINE_AA, 0);
        line(allImages[1], pt[1], pt[2], delaunay_color, 1, LINE_AA, 0);
        line(allImages[1], pt[2], pt[0], delaunay_color, 1, LINE_AA, 0);
    }
}

pair<double,double> getBaryCoord(Point& A, Point& B, Point& C, Point& P){
    // src: http://adrianboeing.blogspot.in/2010/01/barycentric-coordinates.html
    // Compute vectors        
    Point v0 (C.x-A.x, C.y-A.y);
    Point v1 (B.x-A.x, B.y-A.y);
    Point v2 (P.x-A.x, P.y-A.y);

    // Compute dot products
    long long dot00 = dot2(v0, v0);
    long long dot01 = dot2(v0, v1);
    long long dot02 = dot2(v0, v2);
    long long dot11 = dot2(v1, v1);
    long long dot12 = dot2(v1, v2);

    // Compute barycentric coordinates
    long long denom = (dot00 * dot11 - dot01 * dot01);
    if(denom==0){ //colinear points
        cout << "getBaryCoord called for 3 collinear points: " << A.x << "," << A.y << " " << B.x << "," << B.y << " " << C.x << "," << C.y << endl;
        exit(0);
    }
    double u = double(dot00 * dot12 - dot01 * dot02) / denom;
    double v = double(dot11 * dot02 - dot01 * dot12) / denom;
    pair<double,double> tmp(u,v);
    return tmp;
}

Point getPointFromBaryCoord(Point& A, Point& B, Point& C, pair<double,double> bc){
    Point pt;
    pt.x = cvRound(double(A.x) + bc.first*(B.x-A.x) + bc.second*(C.x-A.x));
    pt.y = cvRound(double(A.y) + bc.first*(B.y-A.y) + bc.second*(C.y-A.y));
    return pt;
}

Mat morph(vector<Mat> img, vector<vector<Point> >& features, vector<float>& wt, bool toWarp, bool isAllMorph){

	Mat imgMorphed(img[0].rows,img[0].cols, CV_8UC3, Scalar(0,0,0));
	Rect rectMorphed(0, 0, img[0].cols, img[0].rows);
	Subdiv2D subdivMorphed(rectMorphed);
	Scalar red(0,0,255);
    vector<Point> featuresMorphed;

    for(int i = 0; i < features[0].size(); ++i){
    	float x = 0, y = 0;
    	for(int i1 = 0; i1 < features.size(); ++i1){
    		x += wt[i1] * features[i1][i].x;
    		y += wt[i1] * features[i1][i].y;
    	}
        Point pt(x,y);
        featuresMorphed.push_back(pt);
        subdivMorphed.insert(pt);
        circle( imgMorphed, pt, 2, red, FILLED, LINE_8, 0 );
    }

    for(int x = 1; x < imgMorphed.cols-1; ++x){
        for(int y = 1; y < imgMorphed.rows-1; ++y){
            Point fp(x,y);
            int e0=0, vertex=0;
            int loc = subdivMorphed.locate(fp, e0, vertex);

            if(loc==Subdiv2D::PTLOC_INSIDE || loc==Subdiv2D::PTLOC_ON_EDGE){
                if( e0 > 0 ){
                    Point2f v1, v2, v3;
                    if( !(subdivMorphed.edgeOrg(e0, &v1) > 0 && subdivMorphed.edgeDst(e0, &v2) > 0) ){
                        cout << "error 1" << endl;
                    }
                    e0 = subdivMorphed.getEdge(e0, Subdiv2D::NEXT_AROUND_LEFT);
                    if( !(subdivMorphed.edgeOrg(e0, &v2) > 0 && subdivMorphed.edgeDst(e0, &v3) > 0) ){
                        cout << "error 2" << endl;
                    }

                    // cout << "triangle given " << v1.x << " " << v1.y << " " << v2.x << " " << v2.y << " " << v3.x << " " << v3.y << endl;

                    Point v1_int(cvRound(v1.x),cvRound(v1.y));
                    Point v2_int(cvRound(v2.x),cvRound(v2.y));
                    Point v3_int(cvRound(v3.x),cvRound(v3.y));

                    // cout << "triangle rounded " << v1_int.x << " " << v1_int.y << " " << v2_int.x << " " << v2_int.y << " " << v3_int.x << " " << v3_int.y << endl;
                    int ind1 = getIndFromMorphed(v1_int, &featuresMorphed);
                    int ind2 = getIndFromMorphed(v2_int, &featuresMorphed);
                    int ind3 = getIndFromMorphed(v3_int, &featuresMorphed);

                    if(ind1==-1 || ind2==-1 || ind3==-1){
                        // cout << "exiting" << endl;
                        continue;
                    }

                    pair<double,double> bc = getBaryCoord(v1_int,v2_int,v3_int,fp);

                    if(!toWarp){
                        for(int k = 0; k < features.size(); ++k){
                            Point img_v1 = features[k][ind1];
                            Point img_v2 = features[k][ind2];
                            Point img_v3 = features[k][ind3];

                            Point img_v = getPointFromBaryCoord(img_v1,img_v2,img_v3,bc);

                            imgMorphed.at<Vec3b>(y,x)[0] += wt[k]*img[k].at<Vec3b>(img_v.y,img_v.x)[0];
                            imgMorphed.at<Vec3b>(y,x)[1] += wt[k]*img[k].at<Vec3b>(img_v.y,img_v.x)[1];
                            imgMorphed.at<Vec3b>(y,x)[2] += wt[k]*img[k].at<Vec3b>(img_v.y,img_v.x)[2];
                        }
                    }
                    else{
                        Point img_v1 = features[0][ind1];
                        Point img_v2 = features[0][ind2];
                        Point img_v3 = features[0][ind3];

                        Point img_v = getPointFromBaryCoord(img_v1,img_v2,img_v3,bc);

                        imgMorphed.at<Vec3b>(y,x)[0] += img[0].at<Vec3b>(img_v.y,img_v.x)[0];
                        imgMorphed.at<Vec3b>(y,x)[1] += img[0].at<Vec3b>(img_v.y,img_v.x)[1];
                        imgMorphed.at<Vec3b>(y,x)[2] += img[0].at<Vec3b>(img_v.y,img_v.x)[2];
                    }
                }
            }
            else if(loc==Subdiv2D::PTLOC_VERTEX){
                // cout << "on vertex" << endl;
            }
        }
    }

    imshow("imgMorphed",imgMorphed);
	waitKey(30);
    if(isAllMorph)
        featuresAllMorphed = featuresMorphed;
	return imgMorphed;
}

void traversePath(bool traveseAll, bool toWarp){
    if(traveseAll){
    	for(int i = 0; i < numFaces; ++i){
    		int j = i+1;
    		if(i==numFaces-1)
    			j=0;
       		vector<Mat> imagesToMorph;
				imagesToMorph.push_back(allImages[i]);
				imagesToMorph.push_back(allImages[j]);
			vector<vector<Point> > featuresToMorph;
		    	featuresToMorph.push_back(allFeatures[i]);
		    	featuresToMorph.push_back(allFeatures[j]);
    		for(float wt1 = 1; wt1 >= 0; wt1-=0.05){
		    	vector<float> wt; 
			    	wt.push_back(wt1);
			    	wt.push_back(1-wt1);
                morph(imagesToMorph,featuresToMorph,wt,toWarp,false);
		    }
    	}
    }
    else if(numFaces==2){
		cout << "can't traversePath for less than 3 images!" << endl;
	}
    else{
    	Scalar black(0,0,0);
	    int radius = min(size.width,size.height)/2 - 5;
	    Point center(size.height/2,size.width/2);
	    imgPath = new Mat(size.height,size.width,CV_8UC3,black);
	    circle( *imgPath, center, radius, Scalar( 0, 0, 255 ), 2, LINE_8 );
	    vector<Point> locOfImgOnPath;

	    for(int i = 0; i < numFaces; ++i){
	    	double angle = 6.28*i/numFaces;
	    	int x = int(center.x + radius*cos(angle));
	    	int y = int(center.x + radius*sin(angle));
	    	locOfImgOnPath.push_back(Point(x,y));
	    	circle( *imgPath, Point(x,y), 5, Scalar( 255, 255, 255 ), -1, LINE_8 );
	    	line( *imgPath, Point(x,y), center, Scalar(255,0,0), 1, LINE_AA, 0);
	    }
	    circle( *imgPath, center, 5, Scalar( 255, 255, 255 ), -1, LINE_8 );
	    imshow("imgPath",*imgPath);

	    vector<float> wt; 
		for(int i = 0; i < numFaces; ++i)
			wt.push_back(1.0/numFaces);

        Mat allMorphed = morph(allImages, allFeatures, wt, toWarp, true);
 
	    // imshow("allMorphed", allMorphed);

    	//Waiting for path input through mouse
    	waitKey(0);

	    for(int i = 0; i < path.size(); ++i){
	    	double angle = atan2(path[i].y-center.y, path[i].x-center.x);
	    	if(angle<0)
	    		angle = 6.28 + angle;
	    	int imgOnPath1 = angle/(6.28/numFaces);
	    	int imgOnPath2 = imgOnPath1+1;
	    	if(imgOnPath1==numFaces-1)
	    		imgOnPath2=0;
	    	vector<float> wtToMorph;
	    	if(numFaces>2){
	    		pair<double,double> bc = getBaryCoord(center, locOfImgOnPath[imgOnPath1], locOfImgOnPath[imgOnPath2], path[i]);
				wtToMorph.push_back(1-bc.first-bc.second);
				wtToMorph.push_back(bc.first);
				wtToMorph.push_back(bc.second);
	    	}
			vector<Mat> imagesToMorph;
				imagesToMorph.push_back(allMorphed);
				imagesToMorph.push_back(allImages[imgOnPath1]);
				imagesToMorph.push_back(allImages[imgOnPath2]);
			vector<vector<Point> > featuresToMorph;
				featuresToMorph.push_back(featuresAllMorphed);
		    	featuresToMorph.push_back(allFeatures[imgOnPath1]);
		    	featuresToMorph.push_back(allFeatures[imgOnPath2]);
	    	morph(imagesToMorph,featuresToMorph,wtToMorph,toWarp,false);
	    }
	}
}

void fillFeatures(string filename){
	ifstream in(filename.c_str());
	vector<Point> v;
	while(!in.eof()){
		int x,y;
		in >> x >> y;
		v.push_back(Point(x,y));
	}
	allFeatures.push_back(v);
}

int main(int argc, char *argv[]){
    if(argc != 4){
        // Best results for img.exe 0 0 0
        cout << "Usage: img.exe <toTraverseAll:0or1> <toWarp:0or1> <manualPts:0or1>" << endl;
        exit(0);
    }

    istringstream ss1(argv[1]);
    istringstream ss2(argv[2]);
    istringstream ss3(argv[3]);
    bool toTraverseAll, toWarp, manualPts;
    ss1 >> toTraverseAll;
    ss2 >> toWarp;
    ss3 >> manualPts;

	files.push_back("emma1");
	files.push_back("mark");
	files.push_back("old2");

    if(files.size()<3){
        cout << "Can't traverse path for less than 3 images!" << endl;
        exit(0);
    }

	string jpg = ".jpg";
	string txt = ".txt";
    
     //Create a window
	numFaces = files.size();
	for(int i = 0; i < numFaces; ++i){
		namedWindow(files[i].c_str(),1);
	}
     
	namedWindow("imgPath", 1);

	setMouseCallback(files[0].c_str(), CallBackFunc1, NULL);
	setMouseCallback(files[1].c_str(), CallBackFunc2, NULL);
    if(files.size()==3){
        setMouseCallback(files[2].c_str(), CallBackFunc3, NULL);
    }
	setMouseCallback("imgPath", CallBackFunc4, NULL);

    Scalar blue(255,0,0);
 
    Point tl(0,0);
    Point tr(size.width-1,0);
    Point bl(0,size.height-1);
    Point br(size.width-1,size.height-1);

    for(int i = 0; i < numFaces; ++i){
    	Mat img = imread(files[i]+jpg);
    	resize(img,img,size);
    	allImages.push_back(img);
        if(!manualPts)
    	   fillFeatures(files[i]+txt);
        else{
            vector<Point> v;
            allFeatures.push_back(v);
        }
    	allFeatures[i].push_back(tl);
    	allFeatures[i].push_back(tr);
    	allFeatures[i].push_back(bl);
    	allFeatures[i].push_back(br);
    	imshow(files[i].c_str(),img);
    }
    waitKey(30);

    /*
    float aspect1 = float(img1.rows)/img1.cols;
    float aspect2 = float(img2.rows)/img2.cols;
    float aspect = (aspect1+aspect2)/2;
    int minCol = min(img1.cols,img2.cols);
    Size size(int(aspect*minCol),minCol);
    */

    Rect rect(0, 0, size.width, size.height); // first 2 are start points, next 2 are width and height, 
    //the top and left boundary of the rectangle are inclusive, while the right and bottom boundaries are not
    Subdiv2D subdiv(rect);

    if(manualPts)
        waitKey(0);

    for( int i = 0; i < allFeatures[0].size(); i++ )
        subdiv.insert(allFeatures[0][i]);

    getTriangleList(subdiv);

    // draw_delaunay1(blue);
    // draw_delaunay2(blue);
    // imshow( "img1", img1 );
    // imshow( "img2", img2 );

    /*
    vector<Mat*> images;
		images.push_back(&img1);
		images.push_back(&img2);
	vector<vector<Point>*> features;
    	features.push_back(&allFeatures[0]);
    	features.push_back(&allFeatures[1]);
    
    for(float wt1 = 0; wt1 <= 1; wt1+=0.05){
    	vector<float> wt; 
    	wt.push_back(wt1);
    	wt.push_back(1-wt1);
		morph(images,features,wt);
    }
	*/
    
    traversePath(toTraverseAll,toWarp);

    waitKey(0);

    return 0;
}
