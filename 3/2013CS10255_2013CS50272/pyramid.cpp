#include "util.h"
#include "main_new.cpp"
#define KERNEL_SIZE 5
#define LEVELS 4
int R,C;

vector<Mat> laplacian_pyramid;
Timer t;
vector<float> times;
float pyramid_time;
float total_time;
vvi indexMat_;
Mat seamIm;

class Pyramid{
public:
  Mat original;
  vector<Mat> gaussian_pyramid;
  vector<int> nseams_rows,nseams_cols;
  int levels;
  int curr_level;
  double kernel[5][5];
  Pyramid(Mat& _original,int _l=1){
    assert(_l>0);
    original=_original;
    levels=_l;
    gaussian_pyramid.resize(levels,Mat());
    initGaussianKernel();
    createGaussianPyramid();
    if(laplacian_pyramid.size()==0){
      laplacian_pyramid.resize(levels-1,Mat());
      createLaplacianPyramid();
    }
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
  void displayGaussianPyramid(){
    string s="G_Level_";
    for(int i=0;i<gaussian_pyramid.size();i++){
      imwrite(s+to_string(i)+".bmp",gaussian_pyramid[i]);
    }
  }
  void displayLaplacianPyramid(){
    string s="L_Level_";
    for(int i=0;i<laplacian_pyramid.size();i++){
      imwrite(s+to_string(i)+".bmp",laplacian_pyramid[i]);
    }
  }
  void convolve(Mat& src,Mat& dst,const double mask[5][5],double factor=1.0){
    Mat tmp=src.clone();
    dst = src.clone();
    const int N=5;

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
    int idx=0;
    while(rows!=0 && idx<levels){
      nseams_rows[idx] = (rows>>1)/(1<<idx);
      rows = rows - (1<<idx)*nseams_rows[idx];
      idx++;
    }
    if(rows)
      nseams_rows[0]+=rows,rows=0;
    idx=0;
    while(cols!=0 && idx<levels){
      nseams_cols[idx] = (cols>>1)/(1<<idx);
      cols = cols - (1<<idx)*nseams_cols[idx];
      idx++;
    }
    if(cols)
      nseams_cols[0]+=cols,cols=0;
    puts("Seam distribution...");
    for(auto r:nseams_rows)cout<<r<<endl;
    for(auto c:nseams_cols)cout<<c<<endl;
    cout<<endl;
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
    Timer tnew;
    tnew.reset();
    curr_level=levels-1;
    Mat output= gaussian_pyramid[curr_level];
    Mat m_rows =Mat::zeros(Size(output.rows,output.cols),CV_8UC1);
    Mat m_cols =Mat::zeros(Size(output.cols,output.rows),CV_8UC1);
    Mat m_rows_tmp =Mat::zeros(Size(output.rows,output.cols),CV_8UC1);
    Mat m_cols_tmp =Mat::zeros(Size(output.cols,output.rows),CV_8UC1);
    seam(gaussian_pyramid[curr_level],m_rows,m_cols,0,0,nseams_rows[curr_level],nseams_cols[curr_level]);
    times.push_back(tnew.elapsed());tnew.reset();
    int accumulated_cols=0,accumulated_rows=0;
    accumulated_rows+=nseams_rows[curr_level];
    accumulated_cols+=nseams_cols[curr_level];
    while(curr_level>0){
      m_rows_tmp = m_rows.clone();
      m_cols_tmp = m_cols.clone();
      seamEnlargement(m_rows_tmp,m_cols_tmp,m_rows,m_cols);
      accumulated_rows<<=1;
      accumulated_cols<<=1;
      seam(gaussian_pyramid[curr_level],m_rows,m_cols,accumulated_rows,accumulated_cols,nseams_rows[curr_level],nseams_cols[curr_level]);
      accumulated_rows+=nseams_rows[curr_level];
      accumulated_cols+=nseams_cols[curr_level];
      vvi waste;
      if(R==0)
        imwrite("vertical_seam"+to_string(curr_level)+".bmp",showSeams(gaussian_pyramid[curr_level],waste,m_cols,false));
      else
        imwrite("horizontal_seam"+to_string(curr_level)+".bmp",showSeams(gaussian_pyramid[curr_level],waste,m_rows.t(),false));
      times.push_back(tnew.elapsed());
      tnew.reset();
    }
    if(R==0){
      vvi waste;
      seamIm = showSeams(gaussian_pyramid[0],waste,m_cols,false);
      indexMat_ = indexMat(m_cols);
    }
    else
      showSeams(seamIm,indexMat_,m_rows.t(),true);
    Mat m = removeBothSeamsVerFirst(gaussian_pyramid[curr_level],m_rows.t(),m_cols,R,C); 
    return m;
  }
};

int main(int argc,char** argv){
  R=atoi(argv[2]);
  C=atoi(argv[3]);
  int r=R,c=C;
  
  R=0,C=c;

  Mat m1=imread(argv[1]);
  resize(m1,m1,Size(512,512));
  
  t.reset();
  Pyramid p(m1,LEVELS);
  pyramid_time=t.elapsed();

  t.reset();
  total_time=0;
  m1=p.seamCarving();
  total_time+=t.elapsed();

  p.displayLaplacianPyramid();
  p.displayGaussianPyramid();

  R=r,C=0;
  Pyramid p1(m1,LEVELS);

  t.reset();
  imshow("Compressed",p1.seamCarving());
  total_time+=t.elapsed();
  
  puts("Time analysis...");
  puts("");
  cout<<"Pyramid construction time: "<<pyramid_time<<" ms"<<endl;
  cout<<"Total Evaluation time: "<<total_time<<" ms"<<endl;
  puts("");
  puts("Time for removing vertical seams:");
  for(int i=0;i<LEVELS;i++){
    cout<<("Level "+to_string(LEVELS-i)+": ")<<times[i]<<" ms"<<endl;
  }
  puts("");
  puts("Time for removing horizontal seams:");
  for(int i=0;i<LEVELS;i++){
    cout<<("Level "+to_string(LEVELS-i)+": ")<<times[i+LEVELS]<<" ms"<<endl;
  }
  puts("");
  puts("Done");
  waitKey(0);
  return 0;
}