#include "util.h"

class Seam{
public:
  Mat input,seam; // N*N size
  Mat curr_image,curr_seam;
  vvi map,energy,cumulative_energy; // m*n size
  int rows,cols;
  int rows_removed,cols_removed; 
  Seam(Mat& _input,int _rows=0,int _cols=0){
    assert(_input.empty()==false);
    rows=_rows;cols=_cols;
    rows_removed=0;cols_removed=0;
    input = _input;
    seam = Mat::zeros(Size(input.cols,input.rows),CV_8UC1);
    initializeCurrentState();
  }
  ~Seam(){
  }
  void initializeCurrentState(){
    curr_image = Mat::zeros(Size(input.cols-cols_removed,input.rows-rows_removed),CV_8UC3);
    int r = curr_image.rows,c=curr_image.cols;
    curr_seam = Mat::zeros(Size(c,r),CV_8UC1);
    
    map.resize(r,vi(c,0));
    for(int i=0;i<seam.rows;i++){
      int cntr=0;
      for(int j=0;j<seam.cols;j++){
        if(seam.at<uchar>(i,j)==0){
          map[i][cntr]=j;
          curr_image.at<Vec3b>(i,cntr) = input.at<Vec3b>(i,j);
          cntr++;
          continue;
        }
        assert(seam.at<uchar>(i,j)==MAX_INTENSITY);
      }
      assert(cntr==map[i].size());
    }
    // imshow("curr_image",curr_image);
    // imshow("curr_seam",curr_seam);
    calculateEnergy();
  }
  void calculateEnergy(){
    Mat tmp_x,tmp_y,tmp;
    Sobel(curr_image,tmp_x,CV_16S,1,0);
    convertScaleAbs(tmp_x,tmp_x);
    // imshow("x",tmp_x);
    Sobel(curr_image,tmp_y,CV_16S,0,1);
    convertScaleAbs(tmp_y,tmp_y);
    // imshow("y",tmp_y);
    addWeighted(tmp_x,0.5,tmp_y,0.5,0,tmp);
    imshow("tmp",tmp);
    int r = curr_image.rows,c=curr_image.cols;
    energy.resize(r,vi(c,0));
    cumulative_energy.resize(r,vi(c,0));

    Mat e;
    cout<<tmp.type()<<endl;
    cout<<tmp_x.type()<<endl;
    cout<<tmp_y.type()<<endl;
    cout<<curr_image.type()<<endl;
    Mat temp = Mat::zeros(Size(c,r),CV_8UC1);
    for(int i=0;i<temp.rows;i++){
      for(int j=0;j<temp.cols;j++){
        temp.at<uchar>(i,j) = tmp.at<Vec3s>(i,j)[0];
      }
    }
    Mat temp1 = Mat::zeros(Size(c,r),CV_8UC1);
    for(int i=0;i<temp1.rows;i++){
      for(int j=0;j<temp1.cols;j++){
        temp1.at<uchar>(i,j) = tmp.at<Vec3s>(i,j)[1];
      }
    }
    imshow("temp",temp);
    imshow("temp1",temp1);
    
    e = Mat::zeros(Size(c,r),CV_8UC1);
    for(int i=0;i<r;i++){
      for(int j=0;j<c;j++){
        energy[i][j]=0;
        // for(int k=0;k<=2;k++)
          energy[i][j]=int(tmp.at<Vec3s>(i,j)[2]);
        // e.at<uchar>(i,j)=(energy[i][j]*255)/(3.0*(1<<16));
        // e.at<uchar>(i,j)=100;
        e.at<uchar>(i,j)=energy[i][j];
      }
    }
    imshow("energy",e);
    cout<<e.size()<<endl;
  }
};

int main(){
  Mat im=imread("sample1.jpg");
  Seam s(im,1,1);
  waitKey(0);
}