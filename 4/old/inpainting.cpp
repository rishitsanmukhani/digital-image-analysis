#include "util.h"
#include "Vector.h"
#include <set>

enum Pos{OUT=0,BOUNDARY=1,IN=2};

float lambda = 0.25;
float kappa = 20; 
float delta_t = 0.1;
int Rmin=20,Rmax=50,Cmin=20,Cmax=30;
set<pair<int,int> > pointsToRem;
bool clicked=false;
Mat imInput;
class Inpaint{
public:
  Image img;
  Mat original;
  Inpaint(string _s):img(_s){
    img.load();
    original=img.mat.clone();
  }
  ~Inpaint(){
  }
  inline float gradN(int r,int c,int k)const{
    return img.mat.at<Vec3f>(r-1,c)[k]-img.mat.at<Vec3f>(r,c)[k];
  }
  inline float gradS(int r,int c,int k)const{
    return img.mat.at<Vec3f>(r+1,c)[k]-img.mat.at<Vec3f>(r,c)[k];
  }
  inline float gradE(int r,int c,int k)const{
    return img.mat.at<Vec3f>(r,c+1)[k]-img.mat.at<Vec3f>(r,c)[k];
  }
  inline float gradW(int r,int c,int k)const{
    return img.mat.at<Vec3f>(r,c-1)[k]-img.mat.at<Vec3f>(r,c)[k];
  }
  inline Vector gradient(int r,int c,int k)const{
    return Vector((gradE(r,c,k)-gradW(r,c,k))/2,(gradS(r,c,k)-gradN(r,c,k))/2);
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
    cout<<itr<<endl;
    for(int i=0;i<itr;i++){
      Mat m=img.mat.clone();
      for(int r=1;r<img.mat.rows-1;r++){
        for(int c=1;c<img.mat.cols-1;c++){
          for(int k=0;k<=2;k++){
            m.at<Vec3f>(r,c)[k] = img.mat.at<Vec3f>(r,c)[k] + lambda*(gradE(r,c,k)+gradW(r,c,k)+gradN(r,c,k)+gradS(r,c,k));
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
    for(int i=0;i<itr;i++){
      Mat m=img.mat.clone();
      for(auto p:pointsToRem){
         int r=p.first,c=p.second;
          for(int k=0;k<=2;k++){
            float ge=gradE(r,c,k),gw=gradW(r,c,k),gn=gradN(r,c,k),gs=gradS(r,c,k);
            if(option==0)
              m.at<Vec3f>(r,c)[k] = img.mat.at<Vec3f>(r,c)[k] + lambda*(dce(ge)*ge + dce(gw)*gw +dce(gn)*gn +dce(gs)*gs);
            else
              m.at<Vec3f>(r,c)[k] = img.mat.at<Vec3f>(r,c)[k] + lambda*(dcq(ge)*ge + dcq(gw)*gw +dcq(gn)*gn +dcq(gs)*gs);
          }
      }
      img.mat=m.clone();
    }
  }
  inline float sq(float x)const{
    return (x*x);
  }
  float slopeLimiter(int r,int c,int k,float beta)const{
    float xf=gradE(r,c,k),xb=-gradW(r,c,k),yf=gradS(r,c,k),yb=-gradN(r,c,k);
    if(beta>0) return sqrt(sq(min(xb,0.0f))+sq(max(xf,0.0f))+sq(min(yb,0.0f))+sq(max(yf,0.0f)));
    else return sqrt(sq(max(xb,0.0f))+sq(min(xf,0.0f))+sq(max(yb,0.0f))+sq(min(yf,0.0f)));
  }
  void clamp(float& c){
    if(c>1.0f)c=1.0f;
    if(c<0)c=0;
  }
  void isotopicDiffusion(int itr){
    int cnt=0;
    for(int i=0;i<itr;i++){
      cnt++;
      Mat m=img.mat.clone();
      // for(int r=Rmin;r<Rmax;r++){
      //   for(int c=Cmin;c<Cmax;c++){
      for(auto p:pointsToRem){
         int r=p.first,c=p.second;
            for(int k=0;k<=2;k++){
              Vector grad = gradient(r,c,k);
              Vector N = grad.normal();
              if(N.norm()==0)continue;
              Vector deltaL = laplacianDiff(r,c,k);
              float beta = (deltaL*N)*1.0f/float(N.norm());
              float slope_limiter = slopeLimiter(r,c,k,beta);
              float val=delta_t*beta*slope_limiter;
              float color = float(img.mat.at<Vec3f>(r,c)[k]) + val;
              clamp(color);
              m.at<Vec3f>(r,c)[k] = float(color);
            }
          }
        
        img.mat=m.clone();
        if(cnt==15){
          cnt=0;
          anisotropicDiffusion(2);
        }
      }
    } 
    // }
  // }
};

void CallBackFunc(int event, int x, int y, int flags, void* userdata){
  if  ( event == EVENT_LBUTTONDOWN )
  { 
    if(x>=1 && x<=imInput.cols-2 && y>=1 && y<=imInput.rows-2){
    // cout <<  x << " " << y << endl;
    pair<int,int> p(y,x);
    pointsToRem.insert(p);
    Scalar color(0, 0, 255);
    circle( imInput, Point(x,y), 1, color, FILLED, LINE_8, 0 );
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
        circle( imInput, Point(x,y), 1, color, FILLED, LINE_8, 0 );
        imshow("Input", imInput);     
      }
    }
  }
}
int main(int argc,char** argv){
  if(argc<6){
    puts("Usage: <Rmin> <Rmax> <Cmin> <Cmax> <itr> <image>");
    return 1;
  }
  // Rmin=atoi(argv[1]);
  // Rmax=atoi(argv[2]);
  // Cmin=atoi(argv[3]);
  // Cmax=atoi(argv[4]);
  Inpaint p(argv[6]);
  imInput = p.img.mat.clone();
  imshow("Input",imInput);
  setMouseCallback("Input", CallBackFunc, NULL);
  waitKey(0);
  puts("Normalizing...");
  p.img.mat.convertTo(p.img.mat,CV_32F,1/255.0f);
  puts("Blacking out...");
  // for(int r=Rmin;r<Rmax;r++){
  //   for(int c=Cmin;c<Cmax;c++){
  //     for(int k=0;k<=2;k++){
  //       p.img.mat.at<Vec3f>(r,c)[k]=0;
  //     }
  //   }
  // }
  for(auto _p:pointsToRem){
    int r=_p.first,c=_p.second;
    for(int k=0;k<=2;k++)
      p.img.mat.at<Vec3f>(r,c)[k]=0;
  }
  puts("Diffusion...");
  p.isotopicDiffusion(atoi(argv[5]));
  p.img.mat.convertTo(p.img.mat,CV_8U,255.0f);
  p.img.write("output.bmp");

}