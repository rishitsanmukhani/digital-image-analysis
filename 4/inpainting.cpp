#include "util.h"
#include "Vector.h"

enum Pos{OUT=0,BOUNDARY=1,IN=2};

float lambda = 0.25;
float kappa = 0.2; 
class Inpaint{
public:
  Image img;
  Mat original;
  vector<vector<Vector> > gra;

  Inpaint(string _s):img(_s){
    img.load();
    img.mat.convertTo(img.mat,CV_32S);
    original=img.mat.clone();
  }
  ~Inpaint(){
  }
  inline Vector gradient(int r,int c,int k)const{
    return Vector(gradE(r,c,k)-gradW(r,c,k),gradN(r,c,k)-gradS(r,c,k));
  }
  inline int gradN(int r,int c,int k)const{
    return img.mat.at<Vec3i>(r-1,c)[k]-img.mat.at<Vec3i>(r,c)[k];
  }
  inline int gradS(int r,int c,int k)const{
    return img.mat.at<Vec3i>(r+1,c)[k]-img.mat.at<Vec3i>(r,c)[k];
  }
  inline int gradE(int r,int c,int k)const{
    return img.mat.at<Vec3i>(r,c+1)[k]-img.mat.at<Vec3i>(r,c)[k];
  }
  inline int gradW(int r,int c,int k)const{
    return img.mat.at<Vec3i>(r,c-1)[k]-img.mat.at<Vec3i>(r,c)[k];
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
            m.at<Vec3i>(r,c)[k] = img.mat.at<Vec3i>(r,c)[k] + lambda*(gradE(r,c,k)+gradW(r,c,k)+gradN(r,c,k)+gradS(r,c,k));
          }
        }
      }
      img.mat=m.clone();
    }
  }
  float dce(float _grad_mag,float _kappa=kappa){
    return exp((_grad_mag/_kappa)*(_grad_mag/_kappa));
  }
  float dcq(float _grad_mag,float _kappa=kappa){
    return (1.0f/(1.0f + (_grad_mag/_kappa)*(_grad_mag/_kappa)));
  }
  void anisotropicDiffusion(int itr,float _kappa=kappa,int option=0){
    assert(_kappa>0);
    for(int i=0;i<itr;i++){
      printf("Iteration:%d\n",i+1);
      Mat m=img.mat.clone();
      for(int r=1;r<img.mat.rows-1;r++){
        for(int c=1;c<img.mat.cols-1;c++){
          for(int k=0;k<=2;k++){
            float ge=gradE(r,c,k),gw=gradW(r,c,k),gn=gradN(r,c,k),gs=gradS(r,c,k);
            if(option==0)
              m.at<Vec3i>(r,c)[k] = img.mat.at<Vec3i>(r,c)[k] + lambda*(dce(ge)*ge + dce(gw)*gw +dce(gn)*gn +dce(gs)*gs);
            else
              m.at<Vec3i>(r,c)[k] = img.mat.at<Vec3i>(r,c)[k] + lambda*(dcq(ge)*ge + dcq(gw)*gw +dcq(gn)*gn +dcq(gs)*gs);
          }
        }
      }
      img.mat=m.clone();
    }
  }
};
int main(int argc,char** argv){
  if(argc<4){
    puts("Invalid arguments!");
    return 1;
  }
  Inpaint p("test.bmp");
  p.isotropicDiffusion(atoi(argv[1]));
  p.img.write("out.bmp");

  p.img.mat = p.original.clone();
  kappa = atof(argv[2]);
  lambda = atof(argv[3]);
  p.anisotropicDiffusion(atoi(argv[1]),kappa);
  p.img.write("aniso.bmp");
  p.img.mat.convertTo(p.img.mat,CV_8U);
  imshow("aniso",p.img.mat);
  waitKey(0);
  return 0;
}