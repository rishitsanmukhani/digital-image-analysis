#include "util.h"
#include "Vector.h"

enum Pos{OUT=0,BOUNDARY=1,IN=2};

float lambda = 0.25;
float kappa = 20; 
float delta_t = 0.1;
int Rmin=20,Rmax=50,Cmin=20,Cmax=30;
class Inpaint{
public:
  Image img;
  Mat original;
  Inpaint(string _s):img(_s){
    img.load();
    // img.mat.convertTo(img.mat,CV_32S);
    original=img.mat.clone();
  }
  ~Inpaint(){
  }
  inline int gradN(int r,int c,int k)const{
    return img.mat.at<Vec3b>(r-1,c)[k]-img.mat.at<Vec3b>(r,c)[k];
  }
  inline int gradS(int r,int c,int k)const{
    return img.mat.at<Vec3b>(r+1,c)[k]-img.mat.at<Vec3b>(r,c)[k];
  }
  inline int gradE(int r,int c,int k)const{
    return img.mat.at<Vec3b>(r,c+1)[k]-img.mat.at<Vec3b>(r,c)[k];
  }
  inline int gradW(int r,int c,int k)const{
    return img.mat.at<Vec3b>(r,c-1)[k]-img.mat.at<Vec3b>(r,c)[k];
  }
  inline Vector gradient(int r,int c,int k)const{
    return Vector((gradE(r,c,k)-gradW(r,c,k))>>1,(gradS(r,c,k)-gradN(r,c,k))>>1);
  }
  inline Vector laplacian(int r,int c,int k)const{
    return Vector(gradE(r,c,k)+gradW(r,c,k),gradN(r,c,k)+gradS(r,c,k));
  }
  inline Vector laplacianDiff(int r,int c,int k)const{
    Vector v1=laplacian(r,c+1,k)-laplacian(r,c-1,k);
    Vector v2=laplacian(r+1,c,k)-laplacian(r-1,c,k);
    return Vector(v1.x,v2.y);
  }
  void isotropicDiffusion(int itr){
    for(int i=0;i<itr;i++){
      printf("Iteration:%d\n",i+1);
      Mat m=img.mat.clone();
      for(int r=1;r<img.mat.rows-1;r++){
        for(int c=1;c<img.mat.cols-1;c++){
          for(int k=0;k<=2;k++){
            m.at<Vec3b>(r,c)[k] = img.mat.at<Vec3b>(r,c)[k] + lambda*(gradE(r,c,k)+gradW(r,c,k)+gradN(r,c,k)+gradS(r,c,k));
          }
        }
      }
      img.mat=m.clone();
    }
  }
  float dce(float _grad_mag,float _kappa=kappa){
    return exp(-(_grad_mag/_kappa)*(_grad_mag/_kappa));
  }
  float dcq(float _grad_mag,float _kappa=kappa){
    return (1.0f/(1.0f + (_grad_mag/_kappa)*(_grad_mag/_kappa)));
  }
  void anisotropicDiffusion(int itr,float _kappa=kappa,int option=0){
    assert(_kappa>0);
    for(int i=0;i<itr;i++){
      // printf("Iteration:%d\n",i+1);
      Mat m=img.mat.clone();
      for(int r=Rmin;r<Rmax;r++){
        for(int c=Cmin;c<Cmax;c++){
          for(int k=0;k<=2;k++){
            float ge=gradE(r,c,k),gw=gradW(r,c,k),gn=gradN(r,c,k),gs=gradS(r,c,k);
            if(option==0)
              m.at<Vec3b>(r,c)[k] = img.mat.at<Vec3b>(r,c)[k] + lambda*(dce(ge)*ge + dce(gw)*gw +dce(gn)*gn +dce(gs)*gs);
            else
              m.at<Vec3b>(r,c)[k] = img.mat.at<Vec3b>(r,c)[k] + lambda*(dcq(ge)*ge + dcq(gw)*gw +dcq(gn)*gn +dcq(gs)*gs);
          }
        }
      }
      img.mat=m.clone();
    }
  }
  inline float sq(float x)const{
    return (x*x);
  }
  float slopeLimiter(int r,int c,int k,float beta)const{
    int xf=gradE(r,c,k),xb=-gradW(r,c,k),yf=gradS(r,c,k),yb=-gradN(r,c,k);
    if(beta>0) return sqrt(sq(min(xb,0))+sq(max(xf,0))+sq(min(yb,0))+sq(max(yf,0)));
    else return sqrt(sq(max(xb,0))+sq(min(xf,0))+sq(max(yb,0))+sq(min(yf,0)));
  }
  void isotopicDiffusion(int itr){
    for(int r=Rmin;r<Rmax;r++){
      for(int c=Cmin;c<Cmax;c++){
        for(int k=0;k<=2;k++){
          img.mat.at<Vec3b>(r,c)[k]=0;
        }
      }
    }
    int cnt=0;
    for(int i=0;i<itr;i++){
      cnt++;
      // printf("Isotopic iteration: %d\n",i+1);
      Mat m=img.mat.clone();
      for(int r=Rmin;r<Rmax;r++){
        for(int c=Cmin;c<Cmax;c++){
          for(int k=0;k<=2;k++){
            Vector grad = gradient(r,c,k);
            Vector N = grad.normal();
            if(N.norm()==0)continue;
            Vector deltaL = laplacianDiff(r,c,k);
            float beta = (deltaL*N)*1.0f/float(N.norm());
            float slope_limiter = slopeLimiter(r,c,k,beta);
            float val=delta_t*beta*slope_limiter;
            if(val<-255)
              val=-255;
            if(val>255)
              val=255;

            m.at<Vec3b>(r,c)[k] = img.mat.at<Vec3b>(r,c)[k] + val;
          }
        }
      }
      img.mat=m.clone();
      if(cnt==15){
        cnt=0;
        anisotropicDiffusion(2);
      }
    }
  }
};
int main(int argc,char** argv){
  if(argc<6){
    puts("Invalid arguments!");
    return 1;
  }
  Rmin=atoi(argv[1]);
  Rmax=atoi(argv[2]);
  Cmin=atoi(argv[3]);
  Cmax=atoi(argv[4]);
  Inpaint p("test.bmp");
  p.isotopicDiffusion(atoi(argv[5]));
  p.img.write("paint.bmp");

  return 0;
}