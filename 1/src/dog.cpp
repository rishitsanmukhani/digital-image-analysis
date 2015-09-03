#include "image.cpp"
#include "histogram.cpp"
#include <string>
#include <stack>
#include <sstream>
#define RED 0
#define GREEN 1
#define BLUE 2
#define MAX_INTENSITY 255
#define PI 3.14159265
template <typename T> string to_string(const T& n){
	ostringstream stm;
	stm<<n;
	return stm.str();
}
float** getGaussianMask(int N,double sigma=-1){
	assert(N%2 && "Size should be odd.");
	if(sigma<=0)sigma=0.3*((N-1)*0.5-1)+0.8;
	float** m=new float*[N];
	for(int i=0;i<N;i++)
		m[i]=new float[N];
	double x,y;
	double sum=0;
	for(int i=0;i<N;i++){
		for(int j=0;j<N;j++){
			x=(i-(N>>1));
			y=(j-(N>>1));
			m[i][j]=exp(-(x*x + y*y)/(2*sigma*sigma));
			sum += m[i][j];
		}
	}
	for(int i=0;i<N;i++){
		for(int j=0;j<N;j++){
			m[i][j]/=sum;
		}
	}
	return m;
}
void applyMask(float** mask,int N,Image* img){
	assert(N%2 && "Size should be odd.");
	assert(img!=NULL && mask!=NULL);
	Mat tmp=img->mat.clone();
	for(int i=0;i<img->h;++i){
		for(int j=0;j<img->w;++j){
			int x=i-(N>>1),y=j-(N>>1);
			double sum=0;
			for(int m=0;m<N;m++){
				for(int n=0;n<N;n++){
					if(x+m<0 || x+m>=img->h)continue;
					if(y+n<0 || y+n>=img->w)continue;
					sum += tmp.at<uchar>(x+m,y+n)*mask[m][n];
				}
			}
			img->mat.at<uchar>(i,j)=(uint8_t)sum;
		}
	}
}
void dogOpenCV(Image& in,int ksize1,int ksize2){
	Mat m1=in.mat.clone(),m2=in.mat.clone();
	GaussianBlur(m1,m1,Size(ksize1,ksize1),0,0,BORDER_DEFAULT);
	GaussianBlur(m2,m2,Size(ksize2,ksize2),0,0,BORDER_DEFAULT);	
	in.mat=m1-m2;
}
void dogSelf(int N=11,double sigma=11,double sigma1=1){
	puts("Self implemented Difference of Gaussian...");
	float** mask=getGaussianMask(N,sigma);
	float** mask1=getGaussianMask(N,sigma1);
	Image img("quantize.bmp");
	Image img1("quantize.bmp");
	img.loadImage(true);
	img1.loadImage(true);
	applyMask(mask,N,&img);
	applyMask(mask1,N,&img1);
	imwrite("dog.bmp",img.mat-img1.mat);
	puts("Done.");
}
void canneyEdgePreprocessing(Image& in,int ksize=3){
	puts("Gaussian blur...");
	GaussianBlur( in.mat, in.mat, Size(ksize,ksize), 0, 0, BORDER_DEFAULT );
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	puts("Sobel gradient...");
	Sobel(in.mat, grad_x, CV_32F,1,0,3);
	convertScaleAbs(grad_x,abs_grad_x);
	Sobel(in.mat, grad_y, CV_32F,0,1,3);
	convertScaleAbs(grad_y,abs_grad_y);
	puts("Adding two gradients...");
	addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, in.mat);
	puts("Non maximum suppressing...");
	for(int i=1;i<in.h-1;i++){
		for(int j=1;j<in.w-1;j++){
			float deg;
			if(abs(grad_x.at<float>(i,j)-1e5)>0)
			deg=atan(abs_grad_y.at<float>(i,j)/abs_grad_x.at<float>(i,j))*180/PI;
			else deg=90;
			deg = deg/45;
			int flag=int(round(deg));
			if(flag==-2)flag=2;
			switch(flag){
				case 0:{
					if(in.mat.at<float>(i,j)<in.mat.at<float>(i,j-1) || in.mat.at<float>(i,j)<in.mat.at<float>(i,j+1)){
						// in.mat.at<float>(i,j)=0;
					}
				}
				case 1:{
					if(in.mat.at<float>(i,j)<in.mat.at<float>(i-1,j+1) || in.mat.at<float>(i,j)<in.mat.at<float>(i+1,j-1)){
						// in.mat.at<float>(i,j)=0;
					}
				}
				case -1:{
					if(in.mat.at<float>(i,j)<in.mat.at<float>(i-1,j-1) || in.mat.at<float>(i,j)<in.mat.at<float>(i+1,j+1)){
						// in.mat.at<float>(i,j)=0;
					}
				}
				case 2:{
					if(in.mat.at<float>(i,j)<in.mat.at<float>(i-1,j) || in.mat.at<float>(i,j)<in.mat.at<float>(i+1,j)){
						// in.mat.at<float>(i,j)=0;
					}
				}
			}
		}
	}
}
void hysteresis(Image &in,uint8_t lower,uint8_t upper){
	puts("Removing weak edges...");
	Mat tmp=in.mat.clone();
	int mini=MAX_INTENSITY,maxi=0;
	for(int i=0;i<in.h;++i)
		for(int j=0;j<in.w;++j){
			if(tmp.at<uchar>(i,j)>maxi)
				maxi=tmp.at<uchar>(i,j);
			if(tmp.at<uchar>(i,j)<mini)
				mini=tmp.at<uchar>(i,j);
		}
	lower=uint32_t(lower*maxi)/100;
	upper=uint32_t(upper*maxi)/100;
	
	for(int i=0;i<in.h;++i){
		for(int j=0;j<in.w;++j){
			uchar& c=in.mat.at<uchar>(i,j);
			// cout<<int(c)<<endl;
			if(c<lower){
				c=0;continue;
			}
			if(c>upper){
				c=MAX_INTENSITY;continue;
			}
			int flag=0;
			for(int m=-1;m<=1;m++){
				for(int n=-1;n<=1;n++){
					if(i+m<0 || i+m>=in.h)continue;
					if(j+n<0 || j+n>=in.w)continue;
					if(in.mat.at<uchar>(i+m,j+n)>upper){
						flag=1;
						break;
					}
				}
				if(flag)break;
			}
			if(flag)c=MAX_INTENSITY;
			else c=0;
		}
	}
	puts("Done.");
}
int main(int argc,char** argv){
	if(argc<6){
		puts("Invalid arguments!");
		exit(1);
	}
	Image img1(argv[1]);
	img1.loadImage(true);
	// dogOpenCV(img1,atoi(argv[2]),atoi(argv[3]));
	canneyEdgePreprocessing(img1);
	hysteresis(img1,atoi(argv[4]),atoi(argv[5]));
	img1.writeImage("edge.bmp");
	return 0;
}