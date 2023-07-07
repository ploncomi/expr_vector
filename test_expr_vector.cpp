// Compile with -O3

#include "expr_vector.h"
#include <iostream>

int main()
{
  size_t n = 10000;
  std::vector<double> a0(n), b0(n), c0(n);

  srand(42);
  for (size_t i = 0; i < n; i++)
    a0[i] = rand()+1;
  for (size_t i = 0; i < n; i++)
    b0[i] = 2*rand()+1;
  
  // Providing external buffer
  ExprVector<double> a, b, c;

  a.setBuffer(a0.data(), a0.size());
  b.setBuffer(b0.data(), b0.size());
  c.setBuffer(c0.data(), c0.size());

  c = a + 0.5*a + 0.5*b;

  std::cout << "Sum (external buffer):    " << c.sum() << std::endl;

  // Not providing external buffer
  ExprVector<double> d(n), e(n), f;

  srand(42);
  for (size_t i = 0; i < n; i++)
    d[i] = rand()+1;
  for (size_t i = 0; i < n; i++)
    e[i] = 2*rand()+1;

  f = d + 0.5*d + 0.5*e;

  std::cout << "Sum (no external buffer): " << f.sum() << std::endl;

  // Using strides (python-like format: {start, end, stride})
  // If "start" or "end" are lesser than 0, they count back from the array's ending
  d = 0;
  e = 0;

  f[{0,-1,2}] = d[{0,-1,2}] + e[{1,long(n),2}];

  std::cout << "Number of zeros: " << f.count(0) << std::endl;

  return 0;
}
