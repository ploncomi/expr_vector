// NOTE: In g++, compile with -O3
// NOTE: In msvc, compile with /std:c++14 /O2 /EHsc. Not using /EHsc will cause the code to crash

#include "expr_vector.h"
#include <iostream>
#include <chrono>
#include <valarray>


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
  size_t n = 10000;
  size_t n2 = 3;
  std::vector<double> a0(n), b0(n), c0(n);

  for (size_t i = 0; i < n; i++)
    a0[i] = rand();    // = M_PI/2
  for (size_t i = 0; i < n; i++)
    b0[i] = 2*rand();  // = 0
  
  double t_valarray;
  double t_exprvector;
  double t_rawfor;
  double t_vector;


//#define FORMULA     c = a + (b*a+b)*a + (a*b) + (a*b*b*a) + (a*a*a) + b;
#define FORMULA     c = a + 0.5*a + 0.5*a;
//#define FORMULA     c = a;
//#define FORMULA     c = sin( (a+b) ) + sin(a);
//#define FORMULA     c = atan2(a, b);

//#define FORMULA_FOR     c[i] = a[i] + (b[i]*a[i]+b[i])*a[i] + (a[i]*b[i]) + (a[i]*b[i]*b[i]*a[i]) + (a[i]*a[i]*a[i]) + b[i];
#define FORMULA_FOR     c[i] = a[i] + 0.5*a[i] + 0.5*a[i];
//#define FORMULA_FOR     c[i] = a[i];
//#define FORMULA_FOR     c[i] = sin( (a[i]+b[i]) ) + sin(a[i]);
//#define FORMULA_FOR     c[i] = atan2(a[i], b[i]);

  // Warm up...
  {
    std::vector<double> a(n,0), b(n,1), c(n,2);
    c = a+b+a+b+a+b;
  }  

  // valarray
  {
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t u=0; u<n2; u++)
    {
      std::valarray<double> a(n), b(n), c(n);
      for (size_t i=0; i<n; i++)
        a[i] = a0[i];
      for (size_t i=0; i<n; i++)
        b[i] = b0[i];

      FORMULA
    }

    auto stop = std::chrono::high_resolution_clock::now();

    t_valarray = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
  }

  // ExprVector
  {
    auto start = std::chrono::high_resolution_clock::now();
    if (c0.size() == 0)
      c0.resize(n);
    for (size_t u=0; u<n2; u++)
    {
      ExprVector<double, BuffDataExt<double>> a, b, c;
      a.setBuffer(a0.data(), a0.size());
      b.setBuffer(b0.data(), b0.size());
      c.setBuffer(c0.data(), c0.size());

      FORMULA
    }
    auto stop = std::chrono::high_resolution_clock::now();

    t_exprvector = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
  }

  // raw for
  {
    std::vector<double> a=a0, b=b0;
    std::vector<double> c(n);
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t u=0; u<n2; u++)
    {
      for (size_t i=0; i<n; i++)
      {
        FORMULA_FOR
      }
    }
    auto stop = std::chrono::high_resolution_clock::now();

    t_rawfor = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
  }

  // Using vector
  {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<double> a, b, c(n);
    a = std::move(a0);
    b = std::move(b0);
    for (size_t u=0; u<n2; u++)
    {
      FORMULA
    }
    c0 = std::move(c);
    auto stop = std::chrono::high_resolution_clock::now();

    t_vector = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
  }

  std::cout << "Processing time respect to raw for:" << std::endl;  
  std::cout << "raw for:      " << t_rawfor / t_rawfor <<std::endl;
  std::cout << "ExprVector:   " << t_exprvector / t_rawfor <<std::endl;
  std::cout << "valarray:     " << t_valarray / t_rawfor <<std::endl;
  std::cout << "vector(move): " << t_vector / t_rawfor <<std::endl;

  return 0;
}
