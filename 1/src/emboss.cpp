#include "image.cpp"
#include "histogram.cpp"
#include "util.cpp"
#include <string>
#include <stack>
#include <sstream>
#define RED 0
#define GREEN 1
#define BLUE 2
#define PI 3.14159265
template <typename T> string to_string(const T& n){
	ostringstream stm;
	stm<<n;
	return stm.str();
}
int main(int argc,char** argv){
	if(argc<2){
		puts("Invalid arguments!");
		exit(1);
	}
	float** mask;
	mask=new float*[3];
	for(int i=0;i<3;i++)
		mask[i]=new float[3];
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			mask[i][j]=0;
	mask[0][0]=-1;
	mask[1][0]=-1;
	mask[0][1]=-1;
	mask[2][2]=1;
	mask[1][2]=1;
	mask[2][1]=1;
	Image img1(argv[1]);
	img1.loadImage(true);
	applyMask(mask,3,&img1,MAX_INTENSITY>>1);
	img1.writeImage("emboss.bmp");
	
	return 0;
}