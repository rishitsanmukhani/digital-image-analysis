#pragma once
#include <iostream>
using namespace std;
class Vector{
public:
  int x,y;
  Vector(int _x=0,int _y=0){
    x=_x;
    y=_y;
  }
  bool operator==(const Vector& _v)const{
    return (x==_v.x && y==_v.y); 
  }
  int operator*(const Vector& _v)const{
    return (x*_v.x + y*_v.y);
  }
  Vector operator+(const Vector& _v)const{
    return Vector(x+_v.x,y+_v.y);
  }
  Vector operator-(const Vector& _v)const{
    return Vector(x-_v.x,y-_v.y);
  }
  Vector operator*(const int k)const{
    return Vector(x*k,y*k);
  }
  Vector operator/(const int k)const{
    assert(k!=0);
    return Vector(x/k,y/k);
  }
  int norm()const{
    return (x*x + y*y);
  }
  Vector normalized()const{
    if(norm()>0)
      return Vector(x,y)/norm();
    else
      return Vector(x,y);
  }
  Vector normal()const{
    return Vector(-y,x);
  }
};