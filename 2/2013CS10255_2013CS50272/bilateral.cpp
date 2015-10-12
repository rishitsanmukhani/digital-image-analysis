#include "image.h"
#include "histogram.h"
#include "util.h"

Image img;
Mat img_copy;
int sigma1=4,sigma2=35;
string window_name="Bilateral Filter";
float mask[200][200],gaussian[200][200];

void getBilateralMask(int N,int sigmaSpace,int sigmaColor,int cx,int cy){
  assert(N%2 && "Size should be odd.");
  double x,y;
  int I;
  double sum=0;
  for(int i=0;i<N;i++){
    for(int j=0;j<N;j++){
      x=(i-(N>>1));
      y=(j-(N>>1));
      I = int(img.mat.at<uchar>(cx,cy))-int(img.mat.at<uchar>(cx+x,cy+y));
      mask[i][j]=gaussian[abs(int(x))][abs(int(y))]*exp(-(I*I)/(2.0*sigmaColor*sigmaColor));
      sum += mask[i][j];
    }
  }
  for(int i=0;i<N;i++){
    for(int j=0;j<N;j++){
      mask[i][j]/=sum;
    }
  }
}
void bilateral(){
  puts("Bilateral filter...");
  int sigmaSpace=sigma1,sigmaColor=sigma2;
  int N=(max(sigmaColor,sigmaSpace))+1;
  if(N%2==0)N++;
  assert(N%2 && "Size should be odd.");
  
  for(int i=0;i<200;i++){
    for(int j=0;j<200;j++){
      gaussian[i][j]=exp(-(i*i + j*j)/(2.0*sigmaSpace*sigmaSpace));
    }
  }
  Mat tmp=img.mat.clone();
  for(int i=0;i<img.h;++i){
    for(int j=0;j<img.w;++j){
      int x=i-(N>>1),y=j-(N>>1);
      getBilateralMask(N,sigmaSpace,sigmaColor,i,j);
      double sum=0;
      for(int m=0;m<N;m++){
        for(int n=0;n<N;n++){
          if(x+m<0 || x+m>=img.h)continue;
          if(y+n<0 || y+n>=img.w)continue;
          sum += tmp.at<uchar>(x+m,y+n)*mask[m][n];
        }
      }
      if(sum>MAX_INTENSITY)sum=MAX_INTENSITY;
      img.mat.at<uchar>(i,j)=(uint8_t)sum;
    }
  }
  puts("Done");
}

int main(int argc,char** argv){
  if(argc<4){
    puts("Usage: <application> <image-name> <sigmaSpace> <sigmaColor>");
    exit(1);
  }
  img.name=string(argv[1]);
  img.loadImage(true);

  sigma1=atoi(argv[2]);
  sigma2=atoi(argv[3]);
  
  bilateral();

  img.writeImage("bilateral_out.bmp");
  return 0;
}