// NOTE: In g++, compile with -O3
// NOTE: In msvc, compile with /std:c++14 /O2 /EHsc. Not using /EHsc will cause the code to crash

#include "expr_vector.h"
#include <iostream>
#include <chrono>


// Vector operator for benchmarking
std::vector<double> operator+(const std::vector<double>& a, const std::vector<double>& b)
{
  std::vector<double> c(a.size());
  for (size_t i=0; i<a.size(); i++)
    c[i] = a[i] + b[i];
  return c;
}

std::vector<double> operator-(const std::vector<double>& a, const std::vector<double>& b)
{
  std::vector<double> c(a.size());
  for (size_t i=0; i<a.size(); i++)
    c[i] = a[i] - b[i];
  return c;
}

std::vector<double> operator*(const std::vector<double>& a, const std::vector<double>& b)
{
  std::vector<double> c(a.size());
  for (size_t i=0; i<a.size(); i++)
    c[i] = a[i] * b[i];
  return c;
}


std::vector<double> operator*(const double a, const std::vector<double>& b)
{
  std::vector<double> c(b.size());
  for (size_t i=0; i<b.size(); i++)
    c[i] = a * b[i];
  return c;
}

std::vector<double> sin(const std::vector<double>& a)
{
  std::vector<double> b(a.size());
  for (size_t i=0; i<a.size(); i++)
    b[i] = sin(a[i]);
  return b;
}

std::vector<double> atan2(const std::vector<double>& a, const std::vector<double>& b)
{
  std::vector<double> c(a.size());
  for (size_t i=0; i<a.size(); i++)
    c[i] = atan2(a[i], b[i]);
  return c;
}

double sum(const std::vector<double>& a)
{
  double ret = 0;
  for (double x : a)
    ret += x;
  return ret;
}


//////////////////////////////////////
// Benchmark
//////////////////////////////////////


int main()
{
  size_t n = 100000;
  std::vector<double> a0(n), b0(n), c0(n);

  for (size_t i = 0; i < n; i++)
    a0[i] = rand();    // = M_PI/2
  for (size_t i = 0; i < n; i++)
    b0[i] = 2*rand();  // = 0
  
  double t1, t2;


  // Warm up
  {
    std::vector<double> a(n,0), b(n,1), c(n,2);
    c = a+b+a+b;
  }
  
  // Using ExprVector
  {
    std::cout << "ExprVector" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    ExprVector<double> a, b, c;
    if (c0.size() == 0)
      c0.resize(n);
    a.setBuffer(a0.data(), a0.size());  // Optional
    b.setBuffer(b0.data(), b0.size());  // Optional
    c.setBuffer(c0.data(), c0.size());  // Optional

    //c = a + (b*a+b)*a + (a*b) + (a*b*b*a) + (a*a*a) + b;
    c = a + 0.5*a + 0.5*a;
    //c = sin( (a+b) ) + sin(a);
    //c = atan2(a, b);

    auto stop = std::chrono::high_resolution_clock::now();

    t1 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    std::cout << t1 << " " << sum(c0) << std::endl;
  }

  // Using vector
  {
    std::cout << "vector (move)" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<double> a(n), b(n), c(n);
    a = std::move(a0);
    b = std::move(b0);

    //c = a + (b*a+b)*a + (a*b) + (a*b*b*a) + (a*a*a) + b;
    c = a + 0.5*a + 0.5*a;
    //c = sin( (a+b) ) + sin(a);
    //c = atan2(a, b);

    c0 = std::move(c);
    auto stop = std::chrono::high_resolution_clock::now();

    t2 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    
    std::cout << t2 << " " << sum(c0) << std::endl;
  }

  std::cout << "Processing time ratio: " << t1 / t2 * 100 << "%" << std::endl;

  return 0;
}
