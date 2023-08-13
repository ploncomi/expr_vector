// Provided using BSD license
// Author: Patricio Loncomilla, year 2023

// NOTE: In g++, compile with -O3
// NOTE: In msvc, compile with /std:c++14 /O2 /EHsc. Not using /EHsc will cause the code to crash

#ifndef EXPR_VECTOR_H_PL_
#define EXPR_VECTOR_H_PL_

#include <vector>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>

// Start of main classes for ExprVector

/** BuffData represents a buffer, which can be a std::vector<T> or a buffer pointer T* */
template<typename T>
class BuffData
{
public:
  std::vector<T> vect;
  T* buffer;
  size_t n;

  BuffData() {n=0;}
  BuffData(const BuffData<T>& other) {vect.resize(other.n); for (size_t i=0; i<vect.size(); i++) vect[i] = other[i]; buffer = vect.data(); this->n = vect.size();}

  BuffData(size_t n) {vect.resize(n); buffer = vect.data(); this->n = vect.size();}
  BuffData(size_t n, const T initialValue) {vect = std::vector<T>(n, initialValue); buffer = vect.data(); this->n = vect.size();}
  
  void resize(size_t n) {vect.resize(n); buffer = vect.data(); this->n = vect.size();}

  inline T operator[](const std::size_t i) const
  {
    return buffer[i];
  }

  inline T& operator[](const std::size_t i)
  {
    return buffer[i];
  }
  
  inline T* data() {return buffer;}
  
  inline std::size_t size() const
  {
    return n;
  }

  bool ownsMem() {return buffer == vect.data();}
};

template<typename T, typename Op1>
class BuffDataStrided
{
public:
  Op1& op1;
  long start;
  long end;
  long step;
  size_t n;

  BuffDataStrided(Op1& a, long start, long end, long step) : op1(a), start(start), end(end), step(step)
  {
    n = (std::abs(end-start) + abs(step)-1) / abs(step);

    //std::cout << start << ":" << end << ":" << step << " " << " n = " << n << std::endl;
  }

  void resize(size_t n) {std::cerr << "Error: BuffDataStrided::resize() called" << std::endl;}

  inline T operator[](const std::size_t i) const
  {
    return op1[start + i*step];
  }

  inline T& operator[](const std::size_t i)
  {
    return op1[start + i*step];
  }
  
  inline std::size_t size() const
  {
    return n;
  }
  bool ownsMem() {return false;}
};


class ExprVectorDefaultIndex
{
public:
  constexpr ExprVectorDefaultIndex() {};
};

/** ExprVectorNeg represents the negation of a ExprVector */
template<typename T, typename Op1>
class ExprVectorNeg {
  const Op1& op1;

public:
  ExprVectorNeg(const Op1& a) : op1(a) {}

  inline T operator[](const std::size_t i) const
  {
    return -op1[i];
  }

  inline std::size_t size() const
  {
    return op1.size();
  }
};

namespace expr_vector_default_index
{
#if __GNUC__ < 6 || (__GNUC__ == 6 && __GNUC_MINOR__ <= 1)  // Old compiler
  static constexpr long _ = std::numeric_limits<long>::max();
#else
  static constexpr ExprVectorDefaultIndex _ = ExprVectorDefaultIndex();
#endif
};


/** ExprVector is the main class which represents a vector/buffer using expression templates */
template<typename T, typename Cont = BuffData<T> >
class ExprVector
{
public:
  Cont cont;

  using DI = ExprVectorDefaultIndex;

  // Empty constructed ExprVector must be given a buffer before being used
  ExprVector() {}
  void setBuffer(T* buffer, size_t n) {cont.buffer = buffer; cont.n = n;}
  void setBuffer(const T* buffer, size_t n) {cont.buffer = const_cast<T*>(buffer); cont.n = n;}

  // ExprVector with initial size
  ExprVector(const std::size_t n) : cont(n) {}

  // ExprVector with initial size and value
  ExprVector(const std::size_t n, const T initialValue) : cont(n, initialValue) {}

  // Constructor for underlying container
  ExprVector(const Cont& other) : cont(other) {}

  ExprVector(std::initializer_list<T> other)
  {
    cont.resize(other.size());
    for (std::size_t i = 0; i < cont.size(); ++i)
      cont[i] = (other.begin())[i];
  }

  operator ExprVector<T, BuffData<T>>() const {ExprVector<T, BuffData<T>> x; x = *this; return x;}

  // assignment operator for ExprVector of different type
  template<typename T2, typename R2>
  ExprVector& operator=(const ExprVector<T2, R2>& other)
  {
    if (cont.size() == 0 || cont.size() != other.size())
      cont.resize(other.size());
    for (std::size_t i = 0; i < cont.size(); ++i)
      cont[i] = other[i];
    return *this;
  }


  void operator=(const T& val)
  {
    for (std::size_t i = 0; i < cont.size(); ++i)
      cont[i] = val;
  }

  ExprVector& operator=(const ExprVector& other)
  {
    if (cont.size() == 0 || cont.size() != other.size())
      cont.resize(other.size());
    for (std::size_t i = 0; i < cont.size(); ++i)
      cont[i] = other[i];
    return *this;
  }

  ExprVector& operator=(std::initializer_list<T> other)
  {
    cont.resize(other.size());
    for (std::size_t i = 0; i < cont.size(); ++i)
      cont[i] = (other.begin())[i];
    return *this;
  }


  // size of underlying container
  inline std::size_t size() const
  {
    return cont.size();
  }

  // index operators
  inline T operator[](const std::size_t i) const
  {
    return cont[i];
  }

  inline T& operator[](const std::size_t i)
  {
    return cont[i];
  }

  // Negative of ExprVector
  inline ExprVector<T, ExprVectorNeg<T, Cont> > operator-() {return ExprVector<T, ExprVectorNeg<T, Cont>>(ExprVectorNeg<T, Cont>(data()));}

#if __GNUC__ < 6 || (__GNUC__ == 6 && __GNUC_MINOR__ <= 1)  // Old compiler
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::initializer_list<long> start_end_step)
  {
    using namespace expr_vector_default_index;
    long start  = start_end_step.begin()[0];
    long end    = start_end_step.begin()[1];
    long step   = start_end_step.begin()[2];

    DI x;

    if (start_end_step.size() == 3)
    {
      if (start == _ && end == _ && step == _)
        return operator[](std::make_tuple(x,x,x)); //return [](std::make_tuple<DI,DI,DI>(x,x,x));
      if (start == _ && end == _ && step != _)
        return operator[](std::make_tuple(x,x,step)); //return [](std::make_tuple<DI,DI,long>(x,x,step));
      if (start == _ && end != _ && step == _)
        return operator[](std::make_tuple(x,end,x)); //return [](std::make_tuple<DI,long,DI>(x,end,x));
      if (start == _ && end != _ && step != _)
        return operator[](std::make_tuple(x,end,step)); //return [](std::make_tuple<DI,long,long>(x,end,step));

      if (start != _ && end == _ && step == _)
        return operator[](std::make_tuple(start,x,x)); //return [](std::make_tuple<long,DI,DI>(start,x,x));
      if (start != _ && end == _ && step != _)
        return operator[](std::make_tuple(start,x,step)); //return [](std::make_tuple<long,DI,long>(start,x,step));
      if (start != _ && end != _ && step == _)
        return operator[](std::make_tuple(start,end,x));  //return [](std::make_tuple<long,long,DI>(start,end,x));
      if (start != _ && end != _ && step != _)
        return operator[](std::make_tuple(start,end,step));  //return [](std::make_tuple<long,long,long>(start,end,step));
    }
    else
    {
      if (start == _ && end == _)
        return operator[](std::make_tuple(x,x)); //return [](std::make_tuple<DI,DI>(x,x));
      if (start == _ && end != _)
        return operator[](std::make_tuple(x,end)); //return [](std::make_tuple<DI,long>(x,end));
      if (start != _ && end == _)
        return operator[](std::make_tuple(start,x)); //return [](std::make_tuple<long,DI>(start,x));
      if (start != _ && end != _)
        return operator[](std::make_tuple(start,end));  //return [](std::make_tuple<long,long>(start,end));
    }
    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) ); // This could not be executed
  }
#endif  


  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<DI,DI,DI> start_end_step)
  {
    long start = 0;
    long end = size();
    long step = 1;
    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }

  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<DI,DI,long> start_end_step)
  {
    long start;
    long end;
    long step = std::get<2>(start_end_step);

    if (step > 0)
    {
      start = 0;
      end = size();
    }
    else
    {
      start = size()-1;
      end = -1;
    }

    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }

  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<DI,long,DI> start_end_step)
  {
    long start = 0;
    long end = std::get<1>(start_end_step);
    long step = 1;
    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }

  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<DI,long,long> start_end_step)
  {
    long start;
    long end = std::get<1>(start_end_step);
    long step = std::get<2>(start_end_step);

    if (step > 0)
      start = 0;
    else
      start = size()-1;

    while (end < 0)
      end += size();
    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }



  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<long,DI,DI> start_end_step)
  {
    long start = std::get<0>(start_end_step);
    long end = size();
    long step = 1;
    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }

  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<long,DI,long> start_end_step)
  {
    long start  = std::get<0>(start_end_step);
    long end;
    long step   = std::get<2>(start_end_step);

    if (step > 0)
       end = size();
    else
      end = -1;

    while (start < 0)
      start += size();
    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }

  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<long,long,DI> start_end_step)
  {
    long start  = std::get<0>(start_end_step);
    long end    = std::get<1>(start_end_step);
    long step   = 1;
    while (start < 0)
      start += size();
    while (end < 0)
      end += size();
    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }  

  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<long,long,long> start_end_step)
  {
    long start  = std::get<0>(start_end_step);
    long end    = std::get<1>(start_end_step);
    long step   = std::get<2>(start_end_step);
    while (start < 0)
      start += size();
    while (end < 0)
      end += size();
    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }

  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<DI,DI> start_end)
  {
    long start  = 0;
    long end = size();
    long step = 1;

    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }

  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<DI,long> start_end)
  {
    long start  = 0;
    long end = std::get<1>(start_end);
    long step = 1;

    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }

  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<long,DI> start_end)
  {
    long start  = std::get<0>(start_end);
    long end = size();
    long step = 1;

    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }

  // Slice of ExprVector
  inline ExprVector<T, BuffDataStrided<T, Cont>> operator[](std::tuple<long,long> start_end)
  {
    long start  = std::get<0>(start_end);
    long end    = std::get<1>(start_end);
    long step   = 1;
    while (start < 0)
      start += size();
    while (end < 0)
      end += size();
    return ExprVector<T, BuffDataStrided<T, Cont>>( BuffDataStrided<T, Cont>(data(), start, end, step) );
  }

  inline T sum()
  {
    if (size() == 0)
    {
      throw std::logic_error("ExprVector::sum() called with zero length buffer");
    }
    T val = cont[0];
    for (size_t i=1; i<size(); i++)
      val = val + cont[i];   // += not used because the base class could not have defined it
    return val;
  }

  inline size_t count(const T& val)
  {
    size_t amount = 0;
    for (size_t i=0; i<size(); i++)
      amount += (cont[i] == val);
    return amount;
  }


  // returns the underlying data
  inline const Cont& data() const
  {
    return cont;
  }

  inline Cont& data()
  {
    return cont;
  }

  inline T* begin()
  {
    return cont.data();
  }

  inline T* end()
  {
    return cont.data() + cont.size();
  }

  std::vector<T> vect() {size_t n = size(); std::vector<T> v(n); for (size_t i=0; i<n; i++) v[i] = (*this)[i]; return v;}

  static ExprVector zeros(size_t n) {ExprVector v(n,0); return v;}
  static ExprVector linspace(T start, T stop, long n) {ExprVector v(n); for (size_t i=0; i<n; i++) v[i] = start + i * (stop-start)/(n-1); return v;}
  static ExprVector arange(T start, T stop, T step=1) {long n = (stop - start + step - 1) / step; if (n<=0) return ExprVector(0); ExprVector v(n); for (size_t i=0; i<n; i++) v[i] = start + step * i; return v;}
  static ExprVector arange(T stop) {return arange(0, stop, 1);}
  static ExprVector iota(T start, T stop) {return arange(start, stop);}

  static bool plot_py (const ExprVector& x, const ExprVector& y) {std::stringstream ss; ss << "python -c \"" << "import matplotlib.pyplot as plt; plt.plot(" << x << ", " << y <<"); plt.show()\""; int ret = system(ss.str().c_str()); if (ret != 0) return false; return true;}
  static bool plot_py2(const ExprVector& x, const ExprVector& y) {std::stringstream ss; ss << "python2 -c \"" << "import matplotlib.pyplot as plt; plt.plot(" << x << ", " << y <<"); plt.show()\""; int ret = system(ss.str().c_str()); if (ret != 0) return false; return true;}
  static bool plot_py3(const ExprVector& x, const ExprVector& y) {std::stringstream ss; ss << "python3 -c \"" << "import matplotlib.pyplot as plt; plt.plot(" << x << ", " << y <<"); plt.show()\""; int ret = system(ss.str().c_str()); if (ret != 0) return false; return true;}

  static void plot(const ExprVector& x, const ExprVector& y) { if (!plot_py(x,y) && !plot_py3(x,y) && !plot_py2(x,y)) std::cout << "python+matplotlib was not found for plotting, or too much points to plot" << std::endl;}
  static void plot(const std::vector<T>& x, const std::vector<T>& y) {ExprVector xx; ExprVector yy; xx.setBuffer(x.data(), x.size()); yy.setBuffer(y.data(), y.size()); plot(x,y);}
};

template <typename T, typename Cont>
std::ostream& operator<<(std::ostream& os, const ExprVector<T,Cont> & ev)
{
  os << "[";
  if (ev.size() > 0)
  {
    os << ev[0];
    for (size_t i = 1; i < ev.size(); i++)
      os << ", " << ev[i];
  }
  os << "]";
  return os;
}



#define ADD_EXPR_VECT_OPERATOR_2_ARGS(NAME, OP)                                   \
                                                                                  \
template<typename T, typename Op1, typename Op2>                                  \
class NAME                                                                        \
{                                                                                 \
  const Op1& op1;                                                                 \
  const Op2& op2;                                                                 \
                                                                                  \
public:                                                                           \
  NAME(const Op1& a, const Op2& b) : op1(a), op2(b) {}                            \
                                                                                  \
  inline T operator[](const std::size_t i) const                                  \
  {                                                                               \
    return op1[i] OP op2[i];                                                      \
  }                                                                               \
                                                                                  \
  inline std::size_t size() const                                                 \
  {                                                                               \
    return op1.size();                                                            \
  }                                                                               \
};                                                                                \
                                                                                  \
                                                                                  \
template<typename T, typename R1, typename R2>                                    \
inline ExprVector<T, NAME<T, R1, R2> >                                            \
operator OP (const ExprVector<T, R1>& a, const ExprVector<T, R2>& b)              \
{                                                                                 \
  return ExprVector<T, NAME<T, R1, R2> >(NAME<T, R1, R2 >(a.data(), b.data()));   \
}                                                                                 \


#define ADD_EXPR_VECT_PRE_OP(NAME, OP)                                            \
template<typename T, typename Op2>                                                \
class NAME                                                                        \
{                                                                                 \
  const T val1;                                                                   \
  const Op2& op2;                                                                 \
                                                                                  \
public:                                                                           \
  NAME(T a, const Op2& b) : val1(a), op2(b) {}                                    \
                                                                                  \
  inline T operator[](const std::size_t i) const                                  \
  {                                                                               \
    return val1 OP op2[i];                                                        \
  }                                                                               \
                                                                                  \
  inline std::size_t size() const                                                 \
  {                                                                               \
    return op2.size();                                                            \
  }                                                                               \
};                                                                                \
                                                                                  \
template<typename T, typename R2>                                                 \
inline ExprVector<T, NAME<T, R2> >                                                \
operator OP(T a, const ExprVector<T, R2>& b)                                      \
{                                                                                 \
  return ExprVector<T, NAME<T, R2> >(NAME<T, R2 >(a, b.data()));                  \
}                                                                                 \


#define ADD_EXPR_VECT_POST_OP(NAME, OP)                                           \
template<typename T, typename Op1>                                                \
class NAME                                                                        \
{                                                                                 \
  const Op1& op1;                                                                 \
  const T val2;                                                                   \
                                                                                  \
public:                                                                           \
  NAME(const Op1& a, T b) : op1(a), val2(b) {}                                    \
                                                                                  \
  inline T operator[](const std::size_t i) const                                  \
  {                                                                               \
    return op1[i] OP val2;                                                        \
  }                                                                               \
                                                                                  \
  inline std::size_t size() const                                                 \
  {                                                                               \
    return op1.size();                                                            \
  }                                                                               \
};                                                                                \
                                                                                  \
template<typename T, typename R1>                                                 \
inline ExprVector<T, NAME<T, R1> >                                                \
operator OP(const ExprVector<T, R1>& a, T b)                                      \
{                                                                                 \
  return ExprVector<T, NAME<T, R1> >(NAME<T, R1 >(a.data(), b));                  \
}                                                                                 



#define ADD_EXPR_VECT_PRE_DOUBLE(NAME, OP)                                        \
template<typename T, typename Op2, typename std::enable_if<!std::is_same<T,double>::value, nullptr_t>::type = nullptr>   \
class NAME                                                                        \
{                                                                                 \
  const double val1;                                                              \
  const Op2& op2;                                                                 \
                                                                                  \
public:                                                                           \
  NAME(double a, const Op2& b) : val1(a), op2(b) {}                               \
                                                                                  \
  inline T operator[](const std::size_t i) const                                  \
  {                                                                               \
    return val1 OP op2[i];                                                        \
  }                                                                               \
                                                                                  \
  inline std::size_t size() const                                                 \
  {                                                                               \
    return op2.size();                                                            \
  }                                                                               \
};                                                                                \
                                                                                  \
template<typename T, typename R2>                                                 \
inline ExprVector<T, NAME<T, R2> >                                                \
operator OP(double a, const ExprVector<T, R2>& b)                                 \
{                                                                                 \
  return ExprVector<T, NAME<T, R2> >(NAME<T, R2 >(a, b.data()));                  \
}                                                                                 \


#define ADD_EXPR_VECT_POST_DOUBLE(NAME, OP)                                        \
template<typename T, typename Op2, typename std::enable_if<!std::is_same<T,double>::value, nullptr_t>::type = nullptr>   \
class NAME                                                                        \
{                                                                                 \
  const Op2& op2;                                                                 \
  const double val1;                                                              \
                                                                                  \
public:                                                                           \
  NAME(double a, const Op2& b) : val1(a), op2(b) {}                               \
                                                                                  \
  inline T operator[](const std::size_t i) const                                  \
  {                                                                               \
    return op2[i] OP val1;                                                        \
  }                                                                               \
                                                                                  \
  inline std::size_t size() const                                                 \
  {                                                                               \
    return op2.size();                                                            \
  }                                                                               \
};                                                                                \
                                                                                  \
template<typename T, typename R2>                                                 \
inline ExprVector<T, NAME<T, R2> >                                                \
operator OP(double a, const ExprVector<T, R2>& b)                                 \
{                                                                                 \
  return ExprVector<T, NAME<T, R2> >(NAME<T, R2 >(a, b.data()));                  \
}                                                                                 \

// This is not too much faster than vectors because of function calls..
#define ADD_EXPR_VECT_FN_1_ARG(NAME, fn)                                   \
                                                                           \
template<typename T, typename Op1>                                         \
class NAME                                                                 \
{                                                                          \
  const Op1& op1;                                                          \
                                                                           \
public:                                                                    \
  NAME(const Op1& a) : op1(a) {}                                           \
                                                                           \
  inline T operator[](const std::size_t i) const                           \
  {                                                                        \
    return fn(op1[i]);                                                     \
  }                                                                        \
                                                                           \
  inline std::size_t size() const                                          \
  {                                                                        \
    return op1.size();                                                     \
  }                                                                        \
};                                                                         \
                                                                           \
                                                                           \
template<typename T, typename R1>                                          \
ExprVector<T, NAME<T, R1> >                                                \
inline fn (const ExprVector<T, R1>& a)                                     \
{                                                                          \
  return ExprVector<T, NAME<T, R1> >(NAME<T, R1>(a.data()));               \
}                                                                          \


// This is not too much faster than vectors because of function calls..
#define ADD_EXPR_VECT_FN_2_ARG(NAME, fn)                                          \
                                                                                  \
template<typename T, typename Op1, typename Op2>                                  \
class NAME                                                                        \
{                                                                                 \
  const Op1& op1;                                                                 \
  const Op2& op2;                                                                 \
                                                                                  \
public:                                                                           \
  NAME(const Op1& a, const Op1& b) : op1(a), op2(b) {}                            \
                                                                                  \
  inline T operator[](const std::size_t i) const                                  \
  {                                                                               \
    return fn(op1[i], op2[i]);                                                    \
  }                                                                               \
                                                                                  \
  inline std::size_t size() const                                                 \
  {                                                                               \
    return op1.size();                                                            \
  }                                                                               \
};                                                                                \
                                                                                  \
                                                                                  \
template<typename T, typename R1, typename R2>                                    \
ExprVector<T, NAME<T, R1, R2> >                                                   \
inline fn (const ExprVector<T, R1>& a, const ExprVector<T, R2>& b)                \
{                                                                                 \
  return ExprVector<T, NAME<T, R1, R2> >(NAME<T, R1, R2>(a.data(), b.data()));    \
}                                                                                 \


ADD_EXPR_VECT_OPERATOR_2_ARGS(ExprVectorAdd, +)
ADD_EXPR_VECT_OPERATOR_2_ARGS(ExprVectorSubtr, -)
ADD_EXPR_VECT_OPERATOR_2_ARGS(ExprVectorMult, *)
ADD_EXPR_VECT_OPERATOR_2_ARGS(ExprVectorDiv, /)

ADD_EXPR_VECT_PRE_OP(ExprVectPreSum, +)
ADD_EXPR_VECT_PRE_OP(ExprVectPreSubtr, -)
ADD_EXPR_VECT_PRE_OP(ExprVectPreMult, *)
ADD_EXPR_VECT_PRE_OP(ExprVectPreDiv, /)

ADD_EXPR_VECT_POST_OP(ExprVectPostSum, +)
ADD_EXPR_VECT_POST_OP(ExprVectPostSubtr, -)
ADD_EXPR_VECT_POST_OP(ExprVectPostMult, *)
ADD_EXPR_VECT_POST_OP(ExprVectPostDiv, /)

ADD_EXPR_VECT_PRE_DOUBLE(ExprVectPreMultDouble, *)
ADD_EXPR_VECT_POST_DOUBLE(ExprVectPostMultDouble, *)

ADD_EXPR_VECT_FN_1_ARG(ExprVectorSin, sin)
ADD_EXPR_VECT_FN_1_ARG(ExprVectorCos, cos)

ADD_EXPR_VECT_FN_2_ARG(ExprVectorAtan2, atan2)


#endif // EXPR_VECTOR_H_PL_
