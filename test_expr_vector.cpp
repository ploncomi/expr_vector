// NOTE: In g++, compile with -O3
// NOTE: In msvc, compile with /std:c++14 /O2 /EHsc. Not using /EHsc will cause the code to crash

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
  // Use the symbol _ for a missing index. Example: [::2] transforms into {_,_,2}
  d = 0;
  e = 0;

  using namespace expr_vector_default_index;

  f[{0,-1,2}] = d[{0,-1,2}] + e[{1,_,2}];

  std::cout << "Number of zeros: " << f.count(0) << std::endl;

  // Vector slices can work also with other datatypes (they are general)

  ExprVector<std::string> s1(10), s2(5);

  for (int i=0; i<10; i++)
    s1[i] = "string " + std::to_string(i);

  s2[{_,_,_}] = s1[{_,_,2}];

  std::cout << s2 << std::endl;

  return 0;
}
