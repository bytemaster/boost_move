// Copyright David Abrahams 2004. Distributed under the Boost
// Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_MOVE_DWA2004410_HPP
# define BOOST_MOVE_DWA2004410_HPP

#include <boost/type_traits/is_convertible.hpp>

#include <boost/mpl/if.hpp>

#include <boost/has_swap.hpp>

#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/identity.hpp>

#include <boost/detail/workaround.hpp>

#if  BOOST_WORKAROUND(BOOST_MSVC, <= 1300)
#include <boost/ref.hpp>
#include <boost/type_traits/is_class.hpp>
// MSVC 6/7 decides the implicit move assignment operator is ambiguous.
# define BOOST_MOVE_COPY_SWAP_ASSIGN // Also binds rvalues to non-const refs in assignment
#endif

// Normal EDG like Comeau does enough (N)RVO that the only case where
// turning on the move construction hacks help is when *explicitly*
// direct-initializing an object.  These cases are important, but the
// hacks break down in strict mode.

// Borland will always copy a temporary when used in a direct
// initialization.  Enabling the move construction hacks just makes
// the compiler ICE, so we'll skip 'em

// Intel 8.x on win32 needs help on direct initialization because it
// won't automatically elide copies.  Earlier versions don't seem to
// respond to the move construction hacks, because they blithely bind
// rvalues to non-const references.

#if defined(BOOST_INTEL_CXX_VERSION) 
# if !defined(_MSC_VER) /* Intel-Linux */ \
  || BOOST_INTEL_CXX_VERSION >= 800

#  define BOOST_IMPLICIT_MOVE_CTOR_FOR_COPYABLE_TYPES

# else

// because of the way rvalues bind to non-const references, move
// assignment is never called implicitly.  This at least avoids a copy
// when the source is a genuine rvalue.
#  define BOOST_MOVE_COPY_SWAP_ASSIGN

# endif 
#endif 

namespace boost { 

template <class T, class X, class R = void*>
struct enable_if_same
#ifndef BOOST_NO_SFINAE
{};

template <class X, class R>
struct enable_if_same<X, X, R>
#endif
{
    typedef R type;
};

template <class T, class X, class R = void*>
struct disable_if_same
{
    typedef R type;
};

#ifndef BOOST_NO_SFINAE
template <class X, class R>
struct disable_if_same<X, X, R>
{
};
#endif

// This is the class that enables the distinction of const lvalues in
// the move overload set in case you don't like using the templated
// reference method.  Movable types T used with const_lvalue overloads
// must have a separate conversion to const_lvalue<T>
template <class X>
struct const_lvalue
{
    explicit const_lvalue(X const* p)
      : p(p) {}

    // It acts like a smart pointer, for syntactic convenience
    X const& operator*() const { return *p; }
    
    X const* operator->() const { return p; }

#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
    // A convenience for constructors taking a const_lvalue.  As a
    // result, implicit_cast<X const&>(rhs) can be used to get at the
    // rhs uniformly.
    operator X const&() const { return **this; }
#endif 
    
    friend inline X const& copy_source(const_lvalue<X> x) { return *x; }
#ifdef BOOST_DEBUG_MOVE
    ~const_lvalue()
    {
        p =0;
    }
#endif
    
 protected:
    X const* p; // this pointer will refer to the object from which to move
};

template <class T>
inline T const& copy_source(T& x) { return x; }
    
template <class X>
struct move_from : const_lvalue<X>
{
    explicit move_from(X* p)
      : const_lvalue<X>(p) {}

    // Some compilers need this; seems to do no harm in general.
    explicit move_from(X& x)
      : const_lvalue<X>(&x) {}

#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
    // It would be perverse if we inherited const_lvalue's conversion
    // to X const& without providing this one.
    operator X&() const { return **this; }
#endif 
    
    // It acts like a smart pointer, for syntactic convenience
    X& operator*() const { return *const_cast<X*>(this->p); }
    X* operator->() const { return const_cast<X*>(this->p); }
};

// Detect whether T is movable
template <class T>
struct is_movable
  : boost::is_convertible<T,move_from<T> >
{
};

#if defined(BOOST_MOVE_COPY_SWAP_ASSIGN) || defined(__BORLANDC__)
namespace move_
{
# if BOOST_WORKAROUND(BOOST_MSVC, == 1200)
  
  template <class T>
  struct move_result_
    : mpl::if_<
          is_movable<T>
        , move_from<T>
        , reference_wrapper<T>
      >::type
  {
      typedef typename mpl::if_<
          is_movable<T>
        , move_from<T>
        , reference_wrapper<T>
      >::type base;

      move_result_(T& x) : base(x) {}
  };
  
  template <class T>
  struct move_result
  {
      typedef move_result_<T> type;
  };
  
# elif BOOST_WORKAROUND(BOOST_MSVC, == 1300)
  
  template <class T>
  struct move_result
    : mpl::if_<
          is_class<T>,
          typename mpl::if_<
              is_movable<T>
            , move_from<T>
            , reference_wrapper<T>
          >::type
        , T&
      >
  {};
  
# else // default version
  
  template <class T>
  struct move_result
    : mpl::if_< is_movable<T>, move_from<T> , T& >
  {};

# endif 
}

template <class T>
typename move_::move_result<T>::type
move(T& x)
{
    typedef typename move_::move_result<T>::type r;
    return r(x);
}

#else 

template <class T>
inline
typename boost::mpl::if_<
    is_movable<T>
  , T
  , T&
>::type
move(T& x)
{
    typedef typename boost::mpl::if_<
        is_movable<T>
      , move_from<T>
      , T&
    >::type r1;

    typedef typename boost::mpl::if_<
    is_movable<T>
        , T
        , T&
    >::type r2;
   
    return r2(r1(x));
}

#endif 

// CRTP base class for conveniently making movable types.  You don't
// have to use this to make a type movable (if, for example, you don't
// want the MI non-EBO penalty); you just need to provide the
// conversion to move_from<Derived> yourself in that case.
template <class Derived>
struct movable
{
    operator move_from<Derived>()
    {
        return move_from<Derived>(static_cast<Derived*>(this));
    }

    operator const_lvalue<Derived>() const
    {
        return const_lvalue<Derived>(static_cast<Derived const*>(this));
    }
    
 protected:
    movable() {}
    
 private:
    // any class that wants to be movable should to define its copy
    // ctor and assignment using the macros below, or else those
    // should remain private.
    movable(movable&)
#if BOOST_WORKAROUND(BOOST_MSVC, == 1200) // avoid a codegen bug
    {}
#else
    ;
#endif
    movable& operator=(movable&);
};

namespace move_
{
  // Algorithms lifted directly from
  // http://std.dkuug.dk/jtc1/sc22/wg21/docs/papers/2002/n1377.htm
  template <class T>
  void
  swap(T& a, T& b)
  {
      T tmp(boost::move(a));
      a = boost::move(b);
      b = boost::move(tmp);
  }
}

using move_::swap;

// This is for GCC2 compatibility.  GCC2's standard lib puts std::swap
// into the global namespace, causing ambiguities with boost::swap
// when brought in with a using declaration.
namespace move_swap_
{
  template <class T>
  void
  move_swap(T& a, T& b, mpl::true_)
  {
      swap(a,b); // use ADL
  }

  template <class T>
  void
  move_swap(T& a, T& b, mpl::false_)
  {
      move_::swap(a,b);
  }
  
  template <class T>
  void
  move_swap(T& a, T& b)
  {
#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
      using ::boost::move_::swap;
      using ::swap;
      swap(a,b);
#else
      typedef typename has_swap<T>::type has_swap_;
      move_swap(a,b, has_swap_());
#endif 
  }
}

using move_swap_::move_swap;

// I wonder whether it's a good idea to have the 1-argument and
// 3-argument overloads that do different things?
template <class InputIterator, class OutputIterator>
OutputIterator
move(InputIterator first, InputIterator last, OutputIterator result)
{
    for (; first != last; ++first, ++result)
        *result = move(*first, 0);
    return result;
}
 
template <class BidirectionalIterator1, class BidirectionalIterator2>
BidirectionalIterator2
move_backward(BidirectionalIterator1 first, BidirectionalIterator1 last,
              BidirectionalIterator2 result)
{
    while (last != first)
        *--result = move(*--last, 0);
    return result;
}

//
// Move library convenience macros for generating lvalue copy ctors
// assignment operators
//

// Generates copy constructors for lvalue right-hand-side.  Usage:
//
//     BOOST_LVALUE_COPY_CTOR(
//            class-name, (argument-name)
//          , { ...body code... }
//     )
//
// or
//
//     BOOST_LVALUE_COPY_CTOR(
//            class-name, (argument-name) // note no comma
//              (: initializer1(a1), initializer2(a2), ... )
//          , { ...body code... }
//     )
//
// if you only want to declare a copy ctor, but not define it:
//
//     BOOST_LVALUE_COPY_CTOR(class-name, (argument-name), ;)
//
#ifdef BOOST_IMPLICIT_MOVE_CTOR_FOR_COPYABLE_TYPES

# define BOOST_LVALUE_COPY_CTOR(klass, arg_init, body)                          \
    klass(klass& BOOST_PP_SEQ_HEAD(arg_init))                                   \
        BOOST_MOVE_INITIALIZER_LIST(arg_init)                                   \
        body                                                                    \
                                                                                \
    template <class BoostMove_##klass>                                          \
    klass(                                                                      \
        BoostMove_##klass& BOOST_PP_SEQ_HEAD(arg_init)                          \
      , typename boost::enable_if_same<klass const,BoostMove_##klass>::type = 0 \
    )                                                                           \
        BOOST_MOVE_INITIALIZER_LIST(arg_init)                                   \
    body

# define BOOST_LVALUE_COPY_CTOR2(klass, arg_init, body)                             \
    klass(klass& BOOST_PP_SEQ_HEAD(arg_init))                                       \
        BOOST_MOVE_INITIALIZER_LIST(arg_init)                                       \
        body                                                                        \
                                                                                    \
    klass(boost::const_lvalue<klass> BOOST_PP_SEQ_HEAD(arg_init))                   \
        BOOST_MOVE_INITIALIZER_LIST(arg_init)                                       \
        body

#else

// Generate a "regular" copy ctor.
# define BOOST_LVALUE_COPY_CTOR(klass, arg_init, body)                          \
    klass(klass const& BOOST_PP_SEQ_HEAD(arg_init))                             \
        BOOST_MOVE_INITIALIZER_LIST(arg_init)                                   \
        body 

# define BOOST_LVALUE_COPY_CTOR2(klass, arg_init, body) \
    BOOST_LVALUE_COPY_CTOR(klass, arg_init, body)
#endif 

// Generates copy assignment operators for lvalue right-hand-side.
// Usage:
//
//      BOOST_LVALUE_ASSIGN(
//             class-name, (argument-name)
//          , { ...body code... }
//      )
//
// if you only want to declare a copy assignment operator, but not
// define it:
//
//     BOOST_LVALUE_ASSIGN(class-name, (argument-name), ;)
//
#ifdef BOOST_MOVE_COPY_SWAP_ASSIGN

// Due to compiler deficiencies, the best default is to move from
// rvalues and copy/swap lvalues.  This approach could be more
// expensive than neccessary in some lvalue assignment cases.
// Consider a vector that already has enough memory allocated for all
// rhs elements.

# define BOOST_LVALUE_ASSIGN(klass, rhs, body)                          \
                                                                        \
    klass&                                                              \
    operator=(klass BOOST_PP_SEQ_HEAD(rhs))                             \
    { return *this = boost::move_from<klass>(BOOST_PP_SEQ_HEAD(rhs)); }
        
# define BOOST_LVALUE_ASSIGN2(klass, rhs, body)                         \
    BOOST_LVALUE_ASSIGN(klass, rhs, body)
        
#else

# define BOOST_LVALUE_ASSIGN(klass, rhs, body)                                  \
                                                                                \
    klass&                                                                      \
    operator=(klass& BOOST_PP_SEQ_HEAD(rhs))                                    \
        body                                                                    \
                                                                                \
    template <class BoostMove_##klass>                                          \
    typename boost::enable_if_same<klass const,BoostMove_##klass,klass&>::type  \
    operator=(BoostMove_##klass& BOOST_PP_SEQ_HEAD(rhs))                        \
        body

# define BOOST_LVALUE_ASSIGN2(klass, rhs, body)                             \
                                                                            \
    klass&                                                                  \
    operator=(klass& BOOST_PP_SEQ_HEAD(rhs))                                \
    {                                                                       \
        return *this = boost::const_lvalue<klass>(&BOOST_PP_SEQ_HEAD(rhs)); \
    }                                                                       \
                                                                            \
    klass&                                                                  \
    operator=(boost::const_lvalue<klass> BOOST_MOVE_DUMMY_VAR(rhs))         \
    {                                                                       \
        klass const& BOOST_PP_SEQ_HEAD(rhs) = *BOOST_MOVE_DUMMY_VAR(rhs);   \
        body                                                                \
    }

#endif 

//
// Helper macros
//

# define BOOST_MOVE_DUMMY_VAR(arg_init) BOOST_PP_CAT(BoostMove_,BOOST_PP_SEQ_HEAD(arg_init))

// Given a SEQ appropriate for the 2nd argument to
// BOOST_LVALUE_COPY_CTOR, extract the initializer list.
#define BOOST_MOVE_DROP_ARG(x)
#define BOOST_MOVE_KEEP_ARG(x) x
#define BOOST_MOVE_3RD_OF_2ND(s) BOOST_PP_SEQ_ELEM(2,s)(BOOST_PP_SEQ_ELEM(1,s))
#define BOOST_MOVE_INITIALIZER_LIST(arg_init) \
    BOOST_MOVE_3RD_OF_2ND(arg_init (BOOST_MOVE_KEEP_ARG)(BOOST_MOVE_DROP_ARG))

} // namespace boost

#endif // BOOST_MOVE_DWA2004410_HPP
