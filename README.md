# expr_vector
Class for using operations based on expression templates on std::vector, or on a provided external buffer.

It is provided inside a single header file.

It aims to be similar to std::valarray, but the option for providing an external buffer makes the computations faster, as initialization doesn't require to copy all of the elements:

```
std::vector<double> v(n);
...
ExprVector<double> a;
a.setBuffer(v.data(), v.size());
```

Also it supports using of python-like slices `{start,end,step}`:

```
using namespace expr_vector_default_index;

// c[0:-1:2] = a[0:-1:2] + b[1::2]
c[{0,-1,2}] = a[{0,-1,2}] + b[{1,_,2}];
```
