#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc_c.h"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\core\core_c.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace cv;
using namespace std;

class triplet{
public:
    int a,b,c;
    triplet(int A, int B, int C){
        a=A; b=B; c=C;
    }
};

Mat img1,img2,imgMorphed;
Mat* imgPath;

vector<Point> features1;
vector<Point> features2;

vector<triplet> trianglesInd;

vector<Point> path;

bool clicked = false;


int min(int a, int b){
	if(a>b) return b;
	else return a;
}

bool inBounds(Point pt){
    return (pt.x < img1.cols && pt.x >= 0 && pt.y < img1.rows && pt.y >= 0);
}

int getIndFrom1(Point p){
    for(int i = 0; i  < features1.size(); ++i){
        if(p.x == features1[i].x && p.y == features1[i].y)
            return i;
    }
}

int getIndFromMorphed(Point p, vector<Point>* featuresMorphed){
    for(int i = 0; i  < featuresMorphed->size(); ++i){
        if(p.x == (*featuresMorphed)[i].x && p.y == (*featuresMorphed)[i].y)
            return i;
    }
    cout << "getIndFromMorphed couln't find any match " << p.x << " " << p.y << endl;
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
     	cout <<  x << " " << y << endl;
		Point fp(x,y);
		features1.push_back(fp);
		Scalar color(0, 0, 255);
		circle( img1, fp, 2, color, FILLED, LINE_8, 0 );
		imshow("img1", img1);

     }
 }

void CallBackFunc2(int event, int x, int y, int flags, void* userdata){
    if  ( event == EVENT_LBUTTONDOWN )
    {
    	cout <<  x << ", " << y << endl;
		Point fp(x,y);
		features2.push_back(fp);
		Scalar color(0, 0, 255);
		circle( img2, fp, 2, color, FILLED, LINE_8, 0 );
		imshow("img2", img2);
    }
}

void CallBackFunc3(int event, int x, int y, int flags, void* userdata){
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
    	if(clicked){
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
        pt[0] = features1[trianglesInd[i].a];
        pt[1] = features1[trianglesInd[i].b];
        pt[2] = features1[trianglesInd[i].c];
        line(img1, pt[0], pt[1], delaunay_color, 1, LINE_AA, 0);
        line(img1, pt[1], pt[2], delaunay_color, 1, LINE_AA, 0);
        line(img1, pt[2], pt[0], delaunay_color, 1, LINE_AA, 0);
    }
}

void draw_delaunay2(Scalar delaunay_color){
    vector<Point> pt(3);
    for( int i = 0; i < trianglesInd.size(); i++ )
    {
        pt[0] = features2[trianglesInd[i].a];
        pt[1] = features2[trianglesInd[i].b];
        pt[2] = features2[trianglesInd[i].c];
        line(img2, pt[0], pt[1], delaunay_color, 1, LINE_AA, 0);
        line(img2, pt[1], pt[2], delaunay_color, 1, LINE_AA, 0);
        line(img2, pt[2], pt[0], delaunay_color, 1, LINE_AA, 0);
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

pair<Mat,vector<Point> > morph(vector<Mat*> img, vector<vector<Point>*>& features, vector<float>& wt){

	Mat imgMorphed(img[0]->rows,img[0]->cols, CV_8UC3, Scalar(0,0,0));
	Rect rectMorphed(0, 0, img[0]->cols, img[0]->rows);
	Subdiv2D subdivMorphed(rectMorphed);
	vector<Point> featuresMorphed;
	Scalar red(0,0,255);

    for(int i = 0; i < features[0]->size(); ++i){
    	float x = 0, y = 0;
    	for(int i1 = 0; i1 < features.size(); ++i1){
    		x += wt[i1] * (*features[i1])[i].x;
    		y += wt[i1] * (*features[i1])[i].y;
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
                        cout << "exiting" << endl;
                        continue;
                    }

                    pair<double,double> bc = getBaryCoord(v1_int,v2_int,v3_int,fp);

                    for(int k = 0; k < features.size(); ++k){
                    	Point img_v1 = (*features[k])[ind1];
	                    Point img_v2 = (*features[k])[ind2];
	                    Point img_v3 = (*features[k])[ind3];

	                    Point img_v = getPointFromBaryCoord(img_v1,img_v2,img_v3,bc);

	                    imgMorphed.at<Vec3b>(y,x)[0] += wt[k]*img[k]->at<Vec3b>(img_v.y,img_v.x)[0];
	                    imgMorphed.at<Vec3b>(y,x)[1] += wt[k]*img[k]->at<Vec3b>(img_v.y,img_v.x)[1];
	                    imgMorphed.at<Vec3b>(y,x)[2] += wt[k]*img[k]->at<Vec3b>(img_v.y,img_v.x)[2];
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
	return make_pair(imgMorphed, featuresMorphed);
}

void traversePath(vector<Mat*> images, vector<vector<Point>*>& features, bool traveseAll){
	//Initializing
	int numFaces = images.size();

    if(traveseAll){
    	for(int i = 0; i < numFaces-1; ++i){
    		vector<Mat*> imagesToMorph;
				imagesToMorph.push_back(images[i]);
				imagesToMorph.push_back(images[i+1]);
			vector<vector<Point>*> featuresToMorph;
		    	featuresToMorph.push_back(features[i]);
		    	featuresToMorph.push_back(features[i+1]);
    		for(float wt1 = 0; wt1 <= 1; wt1+=0.05){
		    	vector<float> wt; 
			    	wt.push_back(wt1);
			    	wt.push_back(1-wt1);
				morph(images,features,wt);
		    }
    	}
    }
    else if(numFaces==2){
		cout << "can't traversePath for less than 3 images!" << endl;
	}
    else{
    	Scalar black(0,0,0);
	    Point rectSize(400,400);
	    int radius = min(rectSize.x,rectSize.y)/2 - 5;
	    Point center(rectSize.x/2,rectSize.y/2);
	    imgPath = new Mat(rectSize.x,rectSize.y,CV_8UC3,black);
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
			wt.push_back(1.0/images.size());

	    pair<Mat,vector<Point> > m = morph(images, features, wt);
	    Mat allMorphed = m.first;
	    vector<Point> featuresAllMorphed = m.second;
	    imshow("allMorphed", allMorphed);

    	//Waiting for path input through mouse
    	waitKey(0);

	    for(int i = 0; i < path.size(); ++i){
	    	double angle = atan2(path[i].y-center.y, path[i].x-center.x);
	    	if(angle<0)
	    		angle = 6.28 + angle;
	    	int section = angle/(6.28/numFaces);
	    	vector<float> wtToMorph;
	    	if(numFaces>2){
	    		pair<double,double> bc = getBaryCoord(center, locOfImgOnPath[section], locOfImgOnPath[section+1], path[i]);
		    	double totalWt = 1+bc.first+bc.second; 
				wtToMorph.push_back(1-bc.first-bc.second/totalWt);
				wtToMorph.push_back(bc.first/totalWt);
				wtToMorph.push_back(bc.second/totalWt);
	    	}
			vector<Mat*> imagesToMorph;
				imagesToMorph.push_back(&allMorphed);
				imagesToMorph.push_back(images[section]);
				imagesToMorph.push_back(images[section+1]);
			vector<vector<Point>*> featuresToMorph;
				featuresToMorph.push_back(&featuresAllMorphed);
		    	featuresToMorph.push_back(features[section]);
		    	featuresToMorph.push_back(features[section+1]);
	    	morph(imagesToMorph,featuresToMorph,wtToMorph);
	    }
	}
}

void fillFeatures(string filename, vector<Point>* features){
	ifstream in(filename.c_str());
	while(!in.eof()){
		int x,y;
		in >> x >> y;
		features->push_back(Point(x,y));
	}
}

int main( int, char** )
{
	string file1 = "emma1";
	string file2 = "mark";
	string jpg = ".jpg";
	string txt = ".txt";
    
     //Create a window
     namedWindow("img1", 1);
     namedWindow("img2", 1);
     namedWindow("imgPath", 1);
     
     setMouseCallback("img1", CallBackFunc1, NULL);
     setMouseCallback("img2", CallBackFunc2, NULL);
     setMouseCallback("imgPath", CallBackFunc3, NULL);

    Scalar blue(255,0,0);

    img1 = imread(file1+jpg);
    img2 = imread(file2+jpg);
    /*
    float aspect1 = float(img1.rows)/img1.cols;
    float aspect2 = float(img2.rows)/img2.cols;
    float aspect = (aspect1+aspect2)/2;
    int minCol = min(img1.cols,img2.cols);
    Size size(int(aspect*minCol),minCol);
    */
    Size size(300,300);
    resize(img1,img1,size);
    resize(img2,img2,size);

    Rect rect(0, 0, img1.cols, img1.rows); // first 2 are start points, next 2 are width and height, 
    //the top and left boundary of the rectangle are inclusive, while the right and bottom boundaries are not

    Subdiv2D subdiv(rect);
    
    imshow("img1", img1);
    imshow("img2", img2);

    fillFeatures(file1+txt, &features1);
    fillFeatures(file2+txt, &features2);
    // waitKey(0);

    Point tl(0,0);
    Point tr(img1.cols-1,0);
    Point bl(0,img1.rows-1);
    Point br(img1.cols-1,img1.rows-1);
    features1.push_back(tl);
    features1.push_back(tr);
    features1.push_back(bl);
    features1.push_back(br);
    features2.push_back(tl);
    features2.push_back(tr);
    features2.push_back(bl);
    features2.push_back(br);

    for( int i = 0; i < features1.size(); i++ )
        subdiv.insert(features1[i]);

    getTriangleList(subdiv);

    // draw_delaunay1(blue);
    // draw_delaunay2(blue);
    imshow( "img1", img1 );
    imshow( "img2", img2 );

    
    vector<Mat*> images;
		images.push_back(&img1);
		images.push_back(&img2);
	vector<vector<Point>*> features;
    	features.push_back(&features1);
    	features.push_back(&features2);
    /*
    for(float wt1 = 0; wt1 <= 1; wt1+=0.05){
    	vector<float> wt; 
    	wt.push_back(wt1);
    	wt.push_back(1-wt1);
		morph(images,features,wt);
    }
	*/
    traversePath(images,features,true);

    waitKey(0);

    return 0;
}
