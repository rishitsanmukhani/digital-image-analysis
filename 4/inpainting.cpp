#include "util.h"
#include "Vector.h"

enum Pos{OUT=0,BOUNDARY=1,IN=2};

const float lambda = 0.5;
class Inpaint{
public:
  Image img;
  vector<vector<Vector> > gra;

  Inpaint(string _s):img(_s){
    img.load();
    img.mat.convertTo(img.mat,CV_32S);
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

};
int main(){
  Inpaint p("test.bmp");
  p.isotropicDiffusion(2);
  p.img.write("out.bmp");
  return 0;
}