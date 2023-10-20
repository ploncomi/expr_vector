# expr_vector
Class for using operations based on expression templates on std::vector, or on a provided external buffer.

It is provided inside a single header file.

It aims to be similar to std::valarray, but the option for providing an external buffer makes the computations faster, as initialization doesn't require to copy all of the elements:

```
std::vector<double> v(n);
...
ExprVector<double, BuffDataExt<double>> a;  // BuffDataExt<double> for using external memory
a.setBuffer(v.data(), v.size());
```

It supports using of python-like slices `{start,end,step}`:

```
using namespace expr_vector_default_index;

// c[0:-1:2] = a[0:-1:2] + b[1::2]
c[{0,-1,2}] = a[{0,-1,2}] + b[{1,_,2}];
```

Also, if python/matplotlib is present, the arrays can be plotted (however, the plot must have few points):

```
  ExprVector<double> x = ExprVector<double>::arange(0, 40, 0.1);
  ExprVector<double>::plot(x, sin(x) + 0.5*sin(0.5*x));

```
