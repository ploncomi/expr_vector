// NOTE: In g++, compile with -O3
// NOTE: In msvc, compile with /std:c++14 /O2 /EHsc. Not using /EHsc will cause the code to crash

#include "expr_vector.h"

class PointXYZ
{
public:
  PointXYZ() : x(0), y(0), z(0) {}
  PointXYZ(double x, double y, double z) : x(x), y(y), z(z) {}
  double x,y,z;
};

PointXYZ operator+(PointXYZ p1, PointXYZ p2)
{
  return PointXYZ{p1.x+p2.x, p1.y+p2.y, p1.z+p2.z};
}

PointXYZ operator*(double d, PointXYZ p)
{
  return PointXYZ{d*p.x, d*p.y, d*p.z};
}

PointXYZ operator*(PointXYZ p, double d)
{
  return PointXYZ{p.x*d, p.y*d, p.z*d};
}

PointXYZ operator/(PointXYZ p, double d)
{
  return PointXYZ{p.x/d, p.y/d, p.z/d};
}

double operator*(PointXYZ p1, PointXYZ p2)
{
  return p1.x*p2.x + p1.y*p2.y + p1.z*p2.z;
}

std::ostream& operator<<(std::ostream& os, const PointXYZ& p) {os << "(" << p.x << "," << p.y << "," << p.z << ")"; return os;}


int main()
{
    ExprVector<PointXYZ> a(10, PointXYZ(1,2,3)), b;
    b = 2 * a / sqrt(a*a);
    std::cout << b << std::endl;

    return 0;
}