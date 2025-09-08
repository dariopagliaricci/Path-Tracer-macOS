#ifndef VECTOR_H
#define VECTOR_H
#include <algorithm>
#include <math.h>
#include <stdint.h>

// Vector 3
struct Vec {
    double x, y, z;

    // Keep only this constructor (also acts as default ctor)
    Vec(double x_=0.0, double y_=0.0, double z_=0.0) : x(x_), y(y_), z(z_) {}

    // Non-const index operator: always returns something
    inline double& operator[](int i) {
        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        static double dummy = 0.0;
        return dummy;
    }

    // Const index operator: always returns something
    inline double operator[](int i) const {
        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        return 0.0;
    }

    // Vector operations
    Vec operator+(const Vec &b) const { return Vec(x+b.x,y+b.y,z+b.z); }        // Addition
    Vec operator-(const Vec &b) const { return Vec(x-b.x,y-b.y,z-b.z); }        // Subtraction
    Vec operator*(double b) const { return Vec(x*b,y*b,z*b); }                  // Multiplaction with scalar
    Vec operator/(float b) const { return Vec(x/b,y/b,z/b); }                   // Division with scalar
    Vec mult(const Vec &b) const { return Vec(x*b.x,y*b.y,z*b.z); }             // Multiplication
    Vec& norm(){ return *this = *this * (1/sqrt(x*x+y*y+z*z)); }                // Normalise vector
    double dot(const Vec &b) const { return x*b.x+y*b.y+z*b.z; }                // Dot product
    Vec operator%(Vec&b){return Vec(y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x);}      // Cross product
    Vec cross(const Vec&b){return Vec(y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x);}    // Cross product
    double mag() const{return sqrt(x*x+y*y+z*z);}                               // Calculate vector magnitude
};

#endif // VECTOR_H