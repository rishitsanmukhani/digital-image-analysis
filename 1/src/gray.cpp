#include "image.cpp"
#include "histogram.cpp"
#include <iostream>
#include <cstring>
#include <algorithm>
using namespace std;
class Node;

void gray(Image &in){
	
	Histogram in_hist(&in);
	in_hist.calcuate();

	cout<<"Type:"<<endl;
	cout<<in.mat.type()<<endl;
	cout<<in_hist.hist.type()<<endl;
	puts("---");

	MAX_INTENSITY=in_hist.ranges[0][1];
	puts("Extracting color from histogram...");
	for(int i=0;i<MAX_INTENSITY;i++){
		if(in_hist.hist.at<int>(i,0))color.push_back(i);
	}
	puts("Done.");
	puts("Applying median cut algorithm...");
	Node tree(1,0,color.size()-1,0);
	puts("Done.");
	
	puts("Generating color_map...");
	generateColorMap(tree);
	puts("Done");
	
	Mat out = in.mat.clone();
	for(int i=0;i<out.rows;i++){
		for(int j=0;j<out.cols;j++){
			out.at<uchar>(i,j)=color_map[out.at<uchar>(i,j)];
		}
	}
	puts("Applying floyd steinberg algorithm...");
	Mat floyd = in.mat.clone();
	floyd.convertTo(floyd,CV_8U);
	for(int i=1;i<floyd.rows-1;i++){
		for(int j=1;j<floyd.cols-1;j++){
			int old_val = floyd.at<uchar>(i,j);
			int new_val=0;
			if(floyd.at<uchar>(i,j)>=MAX_INTENSITY)
				new_val=color_map[MAX_INTENSITY-1];
			else
				new_val = color_map[max(floyd.at<uchar>(i,j),unsigned int(0))];
			
			int err = old_val-new_val;
			floyd.at<uchar>(i,j+1) 		+= (err*7)/16;
			floyd.at<uchar>(i+1,j) 		+= (err*5)/16;
			floyd.at<uchar>(i+1,j-1) 	+= (err*3)/16;
			floyd.at<uchar>(i+1,j+1) 	+= (err)/16;
			floyd.at<uchar>(i,j) = new_val;
		}
	}
	
	puts("Done");
	imwrite("butterfly_gray.bmp",in.mat);
	imwrite("out.bmp",out);
	imwrite("floyd.bmp",floyd);	
}
void generateColorMap(Node& tree){
	color_map.resize(MAX_INTENSITY,-1);
	stack<Node*> s;
	s.push(&tree);
	Node *n;
	while(!s.empty()){
		n=s.top();
		s.pop();
		if(n->l)s.push(n->r),s.push(n->l);
		if(n->level==max_colors){
			cout<<n->a<<" "<<n->b<<endl;
			for(int i=n->a;i<=n->b;i++){
				color_map[color[i]]=n->c;
			}
		}
	}
}
