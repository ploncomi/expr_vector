// Compile with -O3

#include "expr_vector.h"
#include <iostream>

int main()
{
  size_t n = 100000;
  std::vector<double> a0(n), b0(n), c0(n);

  srand(42);
  for (size_t i = 0; i < n; i++)
    a0[i] = rand();
  for (size_t i = 0; i < n; i++)
    b0[i] = 2*rand();
  
  // Providing external buffer
  ExprVector<double> a, b, c;
  a.setBuffer(a0.data(), a0.size());
  b.setBuffer(b0.data(), b0.size());
  c.setBuffer(c0.data(), c0.size());

  c = a + 0.5*a + 0.5*b;

  std::cout << c.sum() << std::endl;


  // Not providing external buffer
  ExprVector<double> d(n), e(n), f;

  srand(42);
  for (size_t i = 0; i < n; i++)
    d[i] = rand();
  for (size_t i = 0; i < n; i++)
    e[i] = 2*rand();

  f = d + 0.5*d + 0.5*e;

  std::cout << f.sum() << std::endl;

  return 0;
}
