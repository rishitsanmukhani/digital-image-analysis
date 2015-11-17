#pragma once
#include <iostream>
#include <cstdio>
using namespace std;
class Vector{
public:
  float x,y;
  Vector(float _x=0,float _y=0){
    x=_x;
    y=_y;
  }
  bool operator==(const Vector& _v)const{
    return (x==_v.x && y==_v.y); 
  }
  float operator*(const Vector& _v)const{
    return (x*_v.x + y*_v.y);
  }
  Vector operator+(const Vector& _v)const{
    return Vector(x+_v.x,y+_v.y);
  }
  Vector operator-(const Vector& _v)const{
    return Vector(x-_v.x,y-_v.y);
  }
  Vector operator*(const float k)const{
    return Vector(x*k,y*k);
  }
  Vector operator/(const float k)const{
    assert(k!=0);
    return Vector(x/k,y/k);
  }
  float norm()const{
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
  void print()const{
    printf("%d %d\n",x,y);
  }
};