// Provided using BSD license
// Author: Patricio Loncomilla, year 2023

// Compile with -O3 in g++
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <cmath>
#include <functional>

// Start of main classes for ExprVector

/** BuffData represents a buffer, which can be a std::vector<T> or a buffer pointer T* */
template<typename T>
class BuffData
{
public:
  std::vector<T> vect;
  T* buffer;
  size_t n;

  BuffData<T>() {n=0;}
  BuffData<T>(const BuffData<T>& other) {vect.resize(n); for (size_t i=0; i<vect.size(); i++) vect[i] = other[i]; buffer = vect.data(); this->n = vect.size();}

  BuffData<T>(size_t n) {vect.resize(n); buffer = vect.data(); this->n = vect.size();}
  BuffData<T>(size_t n, const T initialValue) {vect = std::vector<T>(n, initialValue); buffer = vect.data(); this->n = vect.size();}
  
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

/** ExprVector is the main class which represents a vector/buffer using expression templates */
template<typename T, typename Cont = BuffData<T> >
class ExprVector
{
public:
  Cont cont;

  // Empty constructed ExprVector must be swapped with a vector with known size before being used
  ExprVector() {}
  void setBuffer(T* buffer, size_t n) {cont.buffer = buffer; cont.n = n;}
  void setBuffer(const T* buffer, size_t n) {cont.buffer = const_cast<T*>(buffer); cont.n = n;}

  // MyVector with initial size
  ExprVector(const std::size_t n) : cont(n) {}

  // MyVector with initial size and value
  ExprVector(const std::size_t n, const T initialValue) : cont(n, initialValue) {}

  // Constructor for underlying container
  ExprVector(const Cont& other) : cont(other) {}

  // assignment operator for ExprVector of different type
  template<typename T2, typename R2>
  ExprVector& operator=(const ExprVector<T2, R2>& other)
  {
    if (cont.size() == 0)
      cont.resize(other.size());
    for (std::size_t i = 0; i < cont.size(); ++i)
      cont[i] = other[i];
    return *this;
  }

  inline ExprVector<T, ExprVectorNeg<T, Cont> > operator-() {return ExprVector<T, ExprVectorNeg<T, Cont>>(ExprVectorNeg<T, Cont>(data()));}

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

  // returns the underlying data
  inline const Cont& data() const
  {
    return cont;
  }

  inline Cont& data()
  {
    return cont;
  }
};


#define ADD_EXPR_VECT_OPERATOR_2_ARGS(NAME, OP)                            \
                                                                           \
template<typename T, typename Op1, typename Op2>                           \
class NAME                                                                 \
{                                                                          \
  const Op1& op1;                                                          \
  const Op2& op2;                                                          \
                                                                           \
public:                                                                    \
  NAME(const Op1& a, const Op2& b) : op1(a), op2(b) {}                     \
                                                                           \
  inline T operator[](const std::size_t i) const                           \
  {                                                                        \
    return op1[i] OP op2[i];                                               \
  }                                                                        \
                                                                           \
  inline std::size_t size() const                                          \
  {                                                                        \
    return op1.size();                                                     \
  }                                                                        \
};                                                                         \
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
#define ADD_EXPR_VECT_FN_2_ARG(NAME, fn)                                   \
                                                                           \
template<typename T, typename Op1, typename Op2>                           \
class NAME                                                                 \
{                                                                          \
  const Op1& op1;                                                          \
  const Op2& op2;                                                          \
                                                                           \
public:                                                                    \
  NAME(const Op1& a, const Op1& b) : op1(a), op2(b) {}                     \
                                                                           \
  inline T operator[](const std::size_t i) const                           \
  {                                                                        \
    return fn(op1[i], op2[i]);                                             \
  }                                                                        \
                                                                           \
  inline std::size_t size() const                                          \
  {                                                                        \
    return op1.size();                                                     \
  }                                                                        \
};                                                                         \
                                                                           \
                                                                           \
template<typename T, typename R1, typename R2>                             \
ExprVector<T, NAME<T, R1, R2> >                                            \
inline fn (const ExprVector<T, R1>& a, const ExprVector<T, R2>& b)         \
{                                                                          \
  return ExprVector<T, NAME<T, R1, R2> >(NAME<T, R1, R2>(a.data(), b.data()));               \
}                                                                          \


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

