#include <iostream>
#include <cstdio>
#include <cmath>
using namespace std;

#define RED 0
#define GREEN 1
#define BLUE 2

class bgr{
public:
	uint8_t b,g,r;
	bgr(){r=g=b=0;}
	bgr(uint8_t _b,uint8_t _g,uint8_t _r){
		b=_b;g=_g;r=_r;
	}
	void print(){
		cout<<int(b)<<" "<<int(g)<<" "<<int(r)<<endl;
	}
	bool operator==(const bgr& c){
		return (b==c.b && g==c.g && r==c.r);
	}
	bool operator<(const bgr& c)const{
		if(b==c.b){
			if(g==c.g)return r<c.r;
			return g<c.g;
		}
		return b<c.b;
	}
	bgr operator+(const bgr& c){
		return bgr(b+c.b,g+c.g,r+c.r);
	}
	int operator-(const bgr& c){
		return abs(b-c.b)+abs(g-c.g)+abs(r-c.r);
	}
	bgr operator/(const int k){
		return bgr(b/k,g/k,r/k);
	}
};
bool comp_r(const bgr& c1,const bgr& c2){
	return c1.r<c2.r;
}
bool comp_g(const bgr& c1,const bgr& c2){
	return c1.g<c2.g;
}
bool comp_b(const bgr& c1,const bgr& c2){
	return c1.b<c2.b;
}