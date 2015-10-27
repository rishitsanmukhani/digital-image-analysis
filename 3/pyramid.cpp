#include "util.h"
#include "main_new.cpp"
#define KERNEL_SIZE 5
#define LEVELS 3
int R,C;
class Pyramid{
public:
  Mat original;
  vector<Mat> gaussian_pyramid;
  vector<Mat> laplacian_pyramid;
  vector<int> nseams_rows;
  vector<int> nseams_cols;
  int levels;
  int curr_level;
  double kernel[5][5];
  Pyramid(Mat& _original,int _l=1){
    assert(_l>0);
    original=_original;
    levels=_l;
    gaussian_pyramid.resize(levels,Mat());
    laplacian_pyramid.resize(levels,Mat());
    initGaussianKernel();
    createGaussianPyramid();
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
  void seamDistribution(int rows,int cols){
    nseams_rows.clear();nseams_cols.clear();
    nseams_rows.resize(levels,0);
    nseams_cols.resize(levels,0);
    int idx=levels-1;
    while(rows!=0){
      nseams_rows[idx] = rows/(1<<idx);
      rows = rows - (1<<idx)*nseams_rows[idx];
      idx--;
    }
    idx=levels-1;
    while(cols!=0){
      nseams_cols[idx] = cols/(1<<idx);
      cols = cols - (1<<idx)*nseams_cols[idx];
      idx--;
    }
  }
  void seamEnlargement(Mat& m_rows_tmp,Mat& m_cols_tmp,Mat& m_rows,Mat& m_cols){
    m_rows = Mat::zeros(Size(m_rows_tmp.cols<<1,m_rows_tmp.rows<<1),CV_8UC1);
    m_cols = Mat::zeros(Size(m_cols_tmp.cols<<1,m_cols_tmp.rows<<1),CV_8UC1);
    for(int i=0;i<m_rows_tmp.rows;i++){
      for(int j=0;j<m_rows_tmp.cols;j++){
        if(m_rows_tmp.at<uchar>(i,j)==MAX_INTENSITY){
          m_rows.at<uchar>(i<<1,j<<1)=MAX_INTENSITY;
          m_rows.at<uchar>((i<<1)+1,j<<1)=MAX_INTENSITY;
          m_rows.at<uchar>(i<<1,(j<<1)+1)=MAX_INTENSITY;
          m_rows.at<uchar>((i<<1)+1,(j<<1)+1)=MAX_INTENSITY;
        }
      }
    }
    for(int i=0;i<m_cols_tmp.rows;i++){
      for(int j=0;j<m_cols_tmp.cols;j++){
        if(m_cols_tmp.at<uchar>(i,j)==MAX_INTENSITY){
          m_cols.at<uchar>(i<<1,j<<1)=MAX_INTENSITY;
          m_cols.at<uchar>((i<<1)+1,j<<1)=MAX_INTENSITY;
          m_cols.at<uchar>(i<<1,(j<<1)+1)=MAX_INTENSITY;
          m_cols.at<uchar>((i<<1)+1,(j<<1)+1)=MAX_INTENSITY;
        }
      }
    }
    curr_level--;
  }
  Mat seamCarving(){
    seamDistribution(R,C);
    curr_level=levels-1;
    Mat output= gaussian_pyramid[curr_level];
    Mat m_rows =Mat::zeros(Size(output.rows,output.cols),CV_8UC1);
    Mat m_cols =Mat::zeros(Size(output.cols,output.rows),CV_8UC1);
    Mat m_rows_tmp =Mat::zeros(Size(output.rows,output.cols),CV_8UC1);
    Mat m_cols_tmp =Mat::zeros(Size(output.cols,output.rows),CV_8UC1);
    seam(gaussian_pyramid[curr_level],m_rows,m_cols,0,0,nseams_rows[curr_level],nseams_cols[curr_level]);
    while(curr_level>0){
      m_rows_tmp = m_rows.clone();
      m_cols_tmp = m_cols.clone();
      seamEnlargement(m_rows_tmp,m_cols_tmp,m_rows,m_cols);
      seam(gaussian_pyramid[curr_level],m_rows,m_cols,nseams_rows[curr_level+1],nseams_cols[curr_level+1],nseams_rows[curr_level],nseams_cols[curr_level]);
      imshow("gaussian",gaussian_pyramid[curr_level]);
      imshow("seams",m_cols);
    }
    return removeBothSeamsVerFirst(gaussian_pyramid[curr_level],m_rows.t(),m_cols,R,C);
  }
};

int main(int argc,char** argv){
  R=atoi(argv[2]);
  C=atoi(argv[3]);
  int r=R,c=C;
  
  R=0,C=c;
  Mat m1=imread(argv[1]);
  Pyramid p(m1,LEVELS);
  m1=p.seamCarving();

  R=r,C=0;
  Pyramid p1(m1,LEVELS);
  imshow("Compressed",p1.seamCarving());

  waitKey(0);
  return 0;
}