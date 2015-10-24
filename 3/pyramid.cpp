#include "util.h"

#define KERNEL_SIZE 5

class Pyramid{
public:
  Mat original;
  vector<Mat> gaussian_pyramid;
  vector<Mat> laplacian_pyramid;
  int levels;
  double kernel[5][5];
  Pyramid(Mat& _original,int _l=1){
    assert(_l>0);
    original=_original;
    levels=_l;
    gaussian_pyramid.resize(levels,Mat());
    laplacian_pyramid.resize(levels,Mat());
    initGaussianKernel();
    createGaussianPyramid();
    createLaplacianPyramid();
  }
  void initGaussianKernel(){
    Mat m;
    m=16*getGaussianKernel(KERNEL_SIZE,1.1);
    double sum=0;
    for(int i=0;i<KERNEL_SIZE;i++){
      for(int j=0;j<KERNEL_SIZE;j++){
        kernel[i][j]=round(m.at<double>(i))*round(m.at<double>(j));
        sum += kernel[i][j];
      }
    }
    for(int i=0;i<KERNEL_SIZE;i++){
      for(int j=0;j<KERNEL_SIZE;j++){
        kernel[i][j]/=sum;
      }
    }
  }
  void createGaussianPyramid(){
    gaussian_pyramid[0]=original;
    for(int i=0;i<levels-1;i++){
      downsample(gaussian_pyramid[i],gaussian_pyramid[i+1]);
    }
  }
  void createLaplacianPyramid(){
    for(int i=0;i<levels-1;i++){
      Mat tmp;
      upsample(gaussian_pyramid[i+1],tmp);
      laplacian_pyramid[i]=gaussian_pyramid[i]-tmp;
    }
  }
  void convolve(Mat& src,Mat& dst,const double mask[5][5],double factor=1.0){
    Mat tmp=src.clone();
    dst = src.clone();
    const int N=5; //Kernel size

    for(int i=0;i<src.rows;++i){
      for(int j=0;j<src.cols;++j){
        int x=i-(N>>1),y=j-(N>>1);
        Vec3b sum=0;
        for(int m=0;m<N;m++){
          for(int n=0;n<N;n++){
            if(x+m<0 || x+m>=src.rows)continue;
            if(y+n<0 || y+n>=src.cols)continue;
            sum += tmp.at<Vec3b>(x+m,y+n)*mask[m][n];
          }
        }
        sum *= factor;
        for(int k=0;k<3;k++)
          if(sum[k]>MAX_INTENSITY)sum[k]=MAX_INTENSITY;
        dst.at<Vec3b>(i,j)=(Vec3b)sum;
      }
    }
  }
  void downsample(Mat& src,Mat& dst){
    Mat tmp;
    convolve(src,tmp,kernel);
    dst = Mat::zeros(Size((tmp.cols+1)>>1,(tmp.rows+1)>>1),CV_8UC3);
    for(int i=0;i<dst.rows;i++){
      for(int j=0;j<dst.cols;j++){
        dst.at<Vec3b>(i,j)=tmp.at<Vec3b>(i<<1,j<<1);
      }
    }
  }
  void upsample(Mat& src,Mat& dst){
    dst = Mat::zeros(Size(src.cols<<1,src.rows<<1),CV_8UC3);
    for(int i=0;i<src.rows;i++){
      for(int j=0;j<src.cols;j++){
        dst.at<Vec3b>(i<<1,j<<1)=src.at<Vec3b>(i,j);
      }
    } 
    convolve(dst,dst,kernel,4.0);
  }
};

int main(int argc,char** argv){
  Mat m1,m2;
  m1=imread("test.bmp");
  Pyramid p(m1,3);
  imshow("original",p.gaussian_pyramid[0]);
  imshow("laplace",p.laplacian_pyramid[0]);
  p.upsample(p.gaussian_pyramid[1],m2);
  imshow("test",m2+p.laplacian_pyramid[0]);

  waitKey(0);
  return 0;
}