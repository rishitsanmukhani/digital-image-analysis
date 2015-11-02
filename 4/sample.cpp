#include <bits/stdc++.h>
#include <functional>
using namespace std;
using namespace std::placeholders;

class foo{
public:
  int a;
  void g(int a,float b,int c){
    cout<<a<<" "<<b<<endl;
    cout<<"bind"<<endl;
  }
  void g1(){
    cout<<"bind"<<endl;
  }
  void h(){
    auto fun = bind(&foo::g,this,_1,4,_2);
    // fun = bind(&foo::g1,this);
    fun(1,2);
  }
};
void f(){
  cout<<1<<endl;
}
void g(){
  cout<<2<<endl;
}
int main(){
  auto h=bind(f);
  h=bind(g);
  // foo f;
  // f.h();
}