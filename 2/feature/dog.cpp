#include "image.h"
#include "histogram.h"
#include "util.h"

Image img;
Mat img_copy;
int size1=1,size2=5;
int threshold1=5,threshold2=25;
string window_name="Edge Detection";

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
void hysteresis(int,void*);
void dogOpenCV(int, void*){
	img.mat=img_copy.clone();
	int ksize1=2*size1+1;
	int ksize2=2*size2+1;
	Mat m1=img.mat.clone(),m2=img.mat.clone();
	GaussianBlur(m1,m1,Size(ksize1,ksize1),0,0,BORDER_DEFAULT);
	GaussianBlur(m2,m2,Size(ksize2,ksize2),0,0,BORDER_DEFAULT);	
	img.mat=m1-m2;
	hysteresis(0,0);
}
void hysteresis(int, void*){
	int lower=threshold1;
	int upper=threshold2;
	Mat tmp=img.mat.clone();
	int mini=MAX_INTENSITY,maxi=0;
	for(int i=0;i<img.h;++i)
		for(int j=0;j<img.w;++j){
			if(tmp.at<uchar>(i,j)>maxi)
				maxi=tmp.at<uchar>(i,j);
			if(tmp.at<uchar>(i,j)<mini)
				mini=tmp.at<uchar>(i,j);
		}
	lower=(lower*maxi)/100;
	upper=(upper*maxi)/100;
	
	for(int i=0;i<img.h;++i){
		for(int j=0;j<img.w;++j){
			uchar& c=img.mat.at<uchar>(i,j);
			if(c<lower){
				c=0;continue;
			}
			if(c>upper){
				c=MAX_INTENSITY;continue;
			}
			int flag=0;
			for(int m=-1;m<=1;m++){
				for(int n=-1;n<=1;n++){
					if(i+m<0 || i+m>=img.h)continue;
					if(j+n<0 || j+n>=img.w)continue;
					if(img.mat.at<uchar>(i+m,j+n)>upper){
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
	imshow(window_name, img.mat);
	img.writeImage("edge.bmp");
}

int main(int argc,char** argv){
	if(argc<2){
		puts("Usage: <application> <image-name>");
		exit(1);
	}
	img.name=string(argv[1]);
	img.loadImage(true);
	img_copy=img.mat.clone();
	namedWindow( window_name,CV_WINDOW_AUTOSIZE);

  createTrackbar( "Kernel_small:", window_name, &size1,50,dogOpenCV);
  createTrackbar( "Kernel_big:", window_name, &size2,50,dogOpenCV);
  createTrackbar( "Threshold_small:", window_name, &threshold1,100,dogOpenCV);
  createTrackbar( "Threshold_big:", window_name, &threshold2,100, dogOpenCV);
  
	dogOpenCV(0,0);
	hysteresis(0,0);
		
	waitKey(0);
	return 0;
}