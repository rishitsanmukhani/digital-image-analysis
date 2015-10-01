#include <stack>
#include "color.h"
#include "util.h"

int max_intensity=0;
int max_colors=1;
vector<bgr> color3;
vector<bgr> final3;
bgr color_map3[MAX_INTENSITY+1][MAX_INTENSITY+1][MAX_INTENSITY+1];

class Node;
void cut3d(Node *n);
class Node{
public:
	int Split,split_val;
	Node *l,*r,*p;
	int a,b,level;
	bgr c1;
	Node(int _a,int _b,int _level){
		l=r=p=NULL;
		level=_level;
		a=_a;b=_b;
		if(a>b)a=b;
		medianCut();
	}
	void medianCut(){
		if(level==max_colors){
			int sum_r=0,sum_g=0,sum_b=0;
			for(int i=a;i<=b;i++){
				sum_r += int(color3[i].r);
				sum_g += int(color3[i].g);
				sum_b += int(color3[i].b);
			}
			c1.r=sum_r/(b-a+1);
			c1.g=sum_g/(b-a+1);
			c1.b=sum_b/(b-a+1);
			cout<<a<<" "<<b<<" ";
			c1.print();
			final3.push_back(c1);
			return;
		}
		cut3d(this);
	}	
};
void cut3d(Node* n){
	bool R[max_intensity];
	bool G[max_intensity];
	bool B[max_intensity];
	for(int i=0;i<max_intensity;++i)R[i]=G[i]=B[i]=0;
	for(int i=n->a;i<=n->b;i++){
		R[color3[i].r]=1;G[color3[i].g]=1;B[color3[i].b]=1;
	}
	int rc=0,gc=0,bc=0;
	for(int i=0;i<max_intensity;i++){
		if(R[i])rc++;
		if(G[i])gc++;
		if(B[i])bc++;
	}
	int median = (n->a+n->b)>>1;
	int co=max(rc,max(gc,bc));
	if(rc==co){
		sort(color3.begin()+n->a,color3.begin()+n->b+1,comp_r);
		n->Split=RED;
		n->split_val = color3[median].r;
	}
	else if(gc==co){
		sort(color3.begin()+n->a,color3.begin()+n->b+1,comp_g);
		n->Split=GREEN;
		n->split_val = color3[median].g;
	}
	else{
		sort(color3.begin()+n->a,color3.begin()+n->b+1,comp_b);
		n->Split=BLUE;
		n->split_val = color3[median].b;
	}
	n->l=new Node(n->a,median,n->level+1);
	n->r=new Node(median+1,n->b,n->level+1);
	n->l->p=n;n->r->p=n;
}
bgr findNearestNeighbour(bgr c){
	int dist=max_intensity*3;
	int idx=-1;
	for(int i=0;i<final3.size();i++){
		if(c-final3[i]<dist){
			dist=c-final3[i];
			idx=i;
		}
	}
	return final3[idx];
}
void interpolate(){
	for(int i=0;i<=max_intensity-1;i++)
		for(int j=0;j<=max_intensity-1;j++)
			for(int k=0;k<=max_intensity-1;k++)
				if(color_map3[i][j][k]==bgr(0,0,0))
					color_map3[i][j][k]=findNearestNeighbour(bgr(i,j,k));
}
void generateColorMap(Node& tree){
	puts("Generating color map...");
	memset(color_map3,0,sizeof color_map3);
	stack<Node*> s;
	s.push(&tree);
	Node *n;
	while(!s.empty()){
		n=s.top();
		s.pop();
		if(n->l)s.push(n->r),s.push(n->l);
		if(n->level==max_colors){
			color_map3[n->c1.b][n->c1.g][n->c1.r]=n->c1;
			for(int i=n->a;i<=n->b;i++){
				color_map3[color3[i].b][color3[i].g][color3[i].r]=n->c1;
			}
		}
	}
	interpolate();
	puts("Done.");
}
void quantize(Image &in){
	Histogram in_hist(&in);
	puts("Calcuate histogram...");
	in_hist.calcuate();
	max_intensity=in_hist.max_intensity;

	puts("Extracting color from histogram...");
	for(int i=0;i<max_intensity;i++){
		for(int j=0;j<max_intensity;j++){
			for(int k=0;k<max_intensity;k++){
				color3.insert(color3.end(),in_hist.hist[i][j][k],bgr(i,j,k));
			}
		}
	}
	cout<<"Different colors:"<<color3.size()<<endl;

	puts("Applying median cut algorithm...");
	Node tree(0,color3.size()-1,0);

	generateColorMap(tree);

	puts("Generating quantized image...");
	Mat out = in.mat.clone();
	for(int i=0;i<out.rows;i++){
		for(int j=0;j<out.cols;j++){
			Vec3b& bgr= out.at<Vec3b>(i,j);
			Vec3b bgr1;
			bgr1[0]=color_map3[bgr[0]][bgr[1]][bgr[2]].b;
			bgr1[1]=color_map3[bgr[0]][bgr[1]][bgr[2]].g;
			bgr1[2]=color_map3[bgr[0]][bgr[1]][bgr[2]].r;
			bgr=bgr1;
		}
	}
	imwrite("median.bmp",out);
}
// int main(int argc, char** argv){
// 	if(argc<3){
// 		puts("Usage: a <image_name> <color>");
// 		return 1;
// 	}
// 	max_colors=atoi(argv[2]);
// 	Image in(to_string(argv[1]));
// 	in.loadImage(false);
// 	quantize(in);
// 	return 0;
// }