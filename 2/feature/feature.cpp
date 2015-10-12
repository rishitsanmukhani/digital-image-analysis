#include "median_cut.h"
void cropFace(Image& img){
  quantize(img);
  Image ref("median.bmp");
  ref.loadImage(false); 
  bgr face_color;
  vector<bgr> tmp;
  for(int i=-5,r=ref.h>>1,c=ref.w>>1,y;i<=5;i++){
    for(int j=-5;j<=5;j++){
      Vec3b& _c=ref.mat.at<Vec3b>(r+i,c+j);
      tmp.pb(bgr(_c[0],_c[1],_c[2]));
    }
  }
  sort(tmp.begin(),tmp.end());
  face_color=tmp[tmp.size()>>1];
  for(int i=0;i<img.h;i++){
    for(int j=0;j<img.w;j++){
      Vec3b& _c=ref.mat.at<Vec3b>(i,j);
      if(!(bgr(_c[0],_c[1],_c[2])==face_color)){
        img.mat.at<Vec3b>(i,j)=(0,0,0);
      }
    }
  }
}
int main(int argc,char** argv){
  if(argc<2){
    puts("Usage: <exec name> <image name>");
    exit(1);
  }
  Image img(argv[1]);
  img.loadImage(false);
  cropFace(img);
  detectEyes(img);
  img.writeImage("feature.bmp");

  
  return 0;
}