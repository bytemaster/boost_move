//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright David Abrahams, Vicente Botet 2009.
// (C) Copyright Ion Gaztanaga 2009-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////

//! \file

#ifndef BOOST_MOVE_MOVE_HPP
#define BOOST_MOVE_MOVE_HPP

#include <boost/config.hpp>
#include <algorithm> //copy, copy_backward
#include <memory>    //uninitialized_copy
#include <iterator>  //std::iterator

#define BOOST_MOVE_AVOID_BOOST_DEPENDENCIES

#ifndef BOOST_MOVE_AVOID_BOOST_DEPENDENCIES
#include <boost/utility/enable_if.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>
#include <boost/type_traits/integral_constant.hpp>
#endif   //#ifdef BOOST_MOVE_AVOID_BOOST_DEPENDENCIES
/// @cond

#ifdef BOOST_MOVE_AVOID_BOOST_DEPENDENCIES
#define BOOST_MOVE_MPL_NS    ::boost::move_detail
#define BOOST_MOVE_BOOST_NS  ::boost::move_detail
#else
#define BOOST_MOVE_MPL_NS    ::boost::mpl
#define BOOST_MOVE_BOOST_NS  ::boost
#endif

#ifdef BOOST_MOVE_AVOID_BOOST_DEPENDENCIES

namespace boost {
namespace move_detail {

//if_
template<bool C, typename T1, typename T2>
struct if_c
{
    typedef T1 type;
};

template<typename T1, typename T2>
struct if_c<false,T1,T2>
{
    typedef T2 type;
};

template<typename T1, typename T2, typename T3>
struct if_
{
   typedef typename if_c<0 != T1::value, T2, T3>::type type;
};

//enable_if_
template <bool B, class T = void>
struct enable_if_c
{
   typedef T type;
};

template <class T>
struct enable_if_c<false, T> {};

template <class Cond, class T = void>
struct enable_if : public enable_if_c<Cond::value, T> {};

template <class Cond, class T = void>
struct disable_if : public enable_if_c<!Cond::value, T> {};

//integral_constant
template<class T, T v>
struct integral_constant
{
   static const T value = v;
   typedef T value_type;
   typedef integral_constant<T, v> type;
};

//identity
template <class T>
struct identity
{
   typedef T type;
};

//is_convertible
template <class T, class U>
class is_convertible
{
   typedef char true_t;
   class false_t { char dummy[2]; };
   static true_t dispatch(U);
   static false_t dispatch(...);
   static T trigger();
   public:
   enum { value = sizeof(dispatch(trigger())) == sizeof(true_t) };
};

//and_ not_
template <typename Condition1, typename Condition2, typename Condition3 = integral_constant<bool, true> >
  struct and_
    : public integral_constant<bool, Condition1::value && Condition2::value && Condition3::value>
{};

template <typename Boolean>
  struct not_
    : public integral_constant<bool, !Boolean::value>
{};

//is_lvalue_reference
template<class T>
struct is_lvalue_reference
   : public integral_constant<bool, false>
{};

template<class T>
struct is_lvalue_reference<T&>
   : public integral_constant<bool, true>
{};

//has_trivial_destructor
template<class T>
struct has_trivial_destructor
   : public integral_constant<bool, false>
{};

//addressof
template<class T> struct addr_impl_ref
{
   T & v_;
   inline addr_impl_ref( T & v ): v_( v ) {}
   inline operator T& () const { return v_; }

   private:
   addr_impl_ref & operator=(const addr_impl_ref &);
};

template<class T> struct addressof_impl
{
   static inline T * f( T & v, long )
   {
      return reinterpret_cast<T*>(
         &const_cast<char&>(reinterpret_cast<const volatile char &>(v)));
   }

   static inline T * f( T * v, int )
   {  return v;  }
};

template<class T> T * addressof( T & v )
{
   return ::boost::move_detail::addressof_impl<T>::f
      ( ::boost::move_detail::addr_impl_ref<T>( v ), 0 );
}

/*
typedef char one;
struct two {one _[2];};

template <typename B, typename D>
struct is_base_of_host
{
  operator B*() const;
  operator D*();
};

template <typename B, typename D>
struct is_base_of
{
   typedef char yes;
   class no { char dummy[2]; };

   template <typename T> 
   static yes check(D*, T);
   static no check(B*, int);

   static const bool value = sizeof(check(is_base_of_host<B,D>(), int())) == sizeof(yes);
};
*/

}  //namespace move_detail {
}  //namespace boost {

#endif   //BOOST_MOVE_AVOID_BOOST_DEPENDENCIES

/// @endcond

#if !defined(BOOST_NO_RVALUE_REFERENCES)

#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ < 5)

#define BOOST_MOVE_OLD_RVALUE_REF_BINDING_RULES

#else

#if defined(_MSC_VER) && (_MSC_VER == 1600)
#define BOOST_MOVE_MSVC_10_MEMBER_RVALUE_REF_BUG
#endif

#endif

#endif


#if defined(BOOST_NO_RVALUE_REFERENCES) && !defined(BOOST_MOVE_DOXYGEN_INVOKED)

#ifdef __GNUC__ 
#   define BOOST_MOVE_ATTRIBUTE_MAY_ALIAS __attribute__((__may_alias__)) 
#else 
#   define BOOST_MOVE_ATTRIBUTE_MAY_ALIAS 
#endif

namespace boost {

//////////////////////////////////////////////////////////////////////////////
//
//                            struct rv
//
//////////////////////////////////////////////////////////////////////////////
template <class T>
class rv : public T
{
   rv();
   ~rv();
   rv(rv const&);
   void operator=(rv const&);
} BOOST_MOVE_ATTRIBUTE_MAY_ALIAS;

//////////////////////////////////////////////////////////////////////////////
//
//                            move_detail::is_rv
//
//////////////////////////////////////////////////////////////////////////////

namespace move_detail {

template <class T>
struct is_rv
   : BOOST_MOVE_BOOST_NS::integral_constant<bool, false>
{};

template <class T>
struct is_rv< rv<T> >
   : BOOST_MOVE_BOOST_NS::integral_constant<bool, true>
{};

template <class T>
struct is_rv< const rv<T> >
   : BOOST_MOVE_BOOST_NS::integral_constant<bool, true>
{};

}  //namespace move_detail {

//////////////////////////////////////////////////////////////////////////////
//
//                               has_move_emulation_enabled
//
//////////////////////////////////////////////////////////////////////////////
template<class T>
struct has_move_emulation_enabled
   : BOOST_MOVE_BOOST_NS::is_convertible< T, ::boost::rv<T>& >
{};

template<class T>
struct has_move_emulation_enabled<T&>
   : BOOST_MOVE_BOOST_NS::integral_constant<bool, false>
{};

template<class T>
struct has_move_emulation_enabled< ::boost::rv<T> >
   : BOOST_MOVE_BOOST_NS::integral_constant<bool, false>
{};

template <class T> 
struct has_nothrow_move
   : public BOOST_MOVE_BOOST_NS::integral_constant<bool, false>
{};

//////////////////////////////////////////////////////////////////////////////
//
//                            move()
//
//////////////////////////////////////////////////////////////////////////////
template <class T>
typename BOOST_MOVE_BOOST_NS::disable_if<has_move_emulation_enabled<T>, T&>::type move(T& x)
{
   return x;
}

template <class T>
typename BOOST_MOVE_BOOST_NS::enable_if<has_move_emulation_enabled<T>, rv<T>&>::type move(T& x)
{
   return *static_cast<rv<T>* >(BOOST_MOVE_BOOST_NS::addressof(x));
}

template <class T>
typename BOOST_MOVE_BOOST_NS::enable_if<has_move_emulation_enabled<T>, rv<T>&>::type move(rv<T>& x)
{
   return x;
}

#define BOOST_RV_REF(TYPE)\
   ::boost::rv< TYPE >& \
//

#define BOOST_RV_REF_2_TEMPL_ARGS(TYPE, ARG1, ARG2)\
   ::boost::rv< TYPE<ARG1, ARG2> >& \
//

#define BOOST_RV_REF_3_TEMPL_ARGS(TYPE, ARG1, ARG2, ARG3)\
   ::boost::rv< TYPE<ARG1, ARG2, ARG3> >& \
//


#define BOOST_FWD_REF(TYPE)\
   const TYPE & \
//

#define BOOST_CATCH_CONST_RLVALUE(TYPE)\
   const ::boost::rv< TYPE >& \
//

#define BOOST_COPY_ASSIGN_REF(TYPE)\
   const ::boost::rv< TYPE >& \
//

#define BOOST_MOVE_COPY_ASSIGN_REF_2_TEMPL_ARGS(TYPE, ARG1, ARG2)\
   const ::boost::rv< TYPE<ARG1, ARG2> >& \
//

#define BOOST_MOVE_COPY_ASSIGN_REF_3_TEMPL_ARGS(TYPE, ARG1, ARG2, ARG3)\
   const ::boost::rv< TYPE<ARG1, ARG2, ARG3> >& \
//

//////////////////////////////////////////////////////////////////////////////
//
//                            forward()
//
//////////////////////////////////////////////////////////////////////////////

template <class T>
typename BOOST_MOVE_BOOST_NS::enable_if< ::boost::move_detail::is_rv<T>, T &>::type
   forward(const typename BOOST_MOVE_MPL_NS::identity<T>::type &x)
{
   return const_cast<T&>(x);
}

template <class T>
typename BOOST_MOVE_BOOST_NS::disable_if< ::boost::move_detail::is_rv<T>, const T &>::type
   forward(const typename BOOST_MOVE_MPL_NS::identity<T>::type &x)
{
   return x;
}

//////////////////////////////////////////////////////////////////////////////
//
//                         BOOST_MOVABLE_BUT_NOT_COPYABLE
//
//////////////////////////////////////////////////////////////////////////////
#define BOOST_MOVABLE_BUT_NOT_COPYABLE(TYPE)\
   private:\
   TYPE(TYPE &);\
   TYPE& operator=(TYPE &);\
   public:\
   operator ::boost::rv<TYPE>&() \
   {  return *static_cast< ::boost::rv<TYPE>* >(this);  }\
   operator const ::boost::rv<TYPE>&() const \
   {  return *static_cast<const ::boost::rv<TYPE>* >(this);  }\
   private:\
//

//////////////////////////////////////////////////////////////////////////////
//
//                         BOOST_COPYABLE_AND_MOVABLE
//
//////////////////////////////////////////////////////////////////////////////

#define BOOST_COPYABLE_AND_MOVABLE(TYPE)\
   public:\
   TYPE& operator=(TYPE &t)\
   {  this->operator=(static_cast<const ::boost::rv<TYPE> &>(const_cast<const TYPE &>(t))); return *this;}\
   public:\
   operator ::boost::rv<TYPE>&() \
   {  return *static_cast< ::boost::rv<TYPE>* >(this);  }\
   operator const ::boost::rv<TYPE>&() const \
   {  return *static_cast<const ::boost::rv<TYPE>* >(this);  }\
   private:\
//

#define BOOST_COPYABLE_AND_MOVABLE_ALT(TYPE)\
   public:\
   operator ::boost::rv<TYPE>&() \
   {  return *static_cast< ::boost::rv<TYPE>* >(this);  }\
   operator const ::boost::rv<TYPE>&() const \
   {  return *static_cast<const ::boost::rv<TYPE>* >(this);  }\
   private:\
//

}  //namespace boost

#else    //BOOST_NO_RVALUE_REFERENCES

#include <boost/type_traits/remove_reference.hpp>

namespace boost {

//! By default this traits returns false. Classes with non-thworing move construction
//! and assignment should specialize this trait to obtain some performance improvements.
template <class T> 
struct has_nothrow_move
   : public BOOST_MOVE_MPL_NS::integral_constant<bool, false>
{};

//////////////////////////////////////////////////////////////////////////////
//
//                                  move
//
//////////////////////////////////////////////////////////////////////////////


#if defined(BOOST_MOVE_DOXYGEN_INVOKED)
//! This function provides a way to convert a reference into a rvalue reference
//! in compilers with rvalue references. For other compilers converts T & into
//! <i>::boost::rv<T> &</i> so that move emulation is activated.
template <class T> inline 
rvalue_reference move (input_reference);

#else //BOOST_MOVE_DOXYGEN_INVOKED

#if defined(BOOST_MOVE_OLD_RVALUE_REF_BINDING_RULES)

//Old move approach, lvalues could bind to rvalue references
template <class T> inline
typename remove_reference<T>::type && move(T&& t)
{  return t;   }

#else //Old move

template <class T> inline
typename remove_reference<T>::type && move(T&& t)
{ return static_cast<typename remove_reference<T>::type &&>(t); } 

#endif   //Old move

#endif   //BOOST_MOVE_DOXYGEN_INVOKED


//////////////////////////////////////////////////////////////////////////////
//
//                                  forward
//
//////////////////////////////////////////////////////////////////////////////


#if defined(BOOST_MOVE_DOXYGEN_INVOKED)
//! This function provides limited form of forwarding that is usually enough for
//! in-place construction and avoids the exponential overloading necessary for
//! perfect forwarding in C++03.
//!
//! For compilers with rvalue references this function provides perfect forwarding.
//!
//! Otherwise:
//! * If input_reference binds to const ::boost::rv<T> & then it output_reference is
//!   ::boost::rev<T> &
//!
//! * Else, input_reference is equal to output_reference is equal to input_reference.
template <class T> inline output_reference forward(input_reference);

#else

#if defined(BOOST_MOVE_OLD_RVALUE_REF_BINDING_RULES)

//Old move approach, lvalues could bind to rvalue references

template <class T> inline
T&& forward (typename BOOST_MOVE_MPL_NS::identity<T>::type&& t)
{  return t;   }

#else //Old move

//Implementation #5 from N2951, thanks to Howard Hinnant

template <class T, class U>
inline T&& forward(U&& t
    , typename BOOST_MOVE_BOOST_NS::enable_if_c<
      move_detail::is_lvalue_reference<T>::value ? move_detail::is_lvalue_reference<U>::value : true>::type * = 0/*
    , typename BOOST_MOVE_BOOST_NS::enable_if_c<
      move_detail::is_convertible
         <typename remove_reference<U>::type*, typename remove_reference<T>::type*>::value>::type * = 0*/)
{ return static_cast<T&&>(t);   }

#endif   //Old move

#endif   //BOOST_MOVE_DOXYGEN_INVOKED

//////////////////////////////////////////////////////////////////////////////
//
//                         BOOST_ENABLE_MOVE_EMULATION
//
//////////////////////////////////////////////////////////////////////////////

///@cond

#define BOOST_ENABLE_MOVE_EMULATION(TYPE)\
   typedef int boost_move_emulation_t;
\
//

/// @endcond

//! This macro marks a type as movable but not copyable, disabling copy construction
//! and assignment. The user will need to write a move constructor/assignment as explained
//! in the documentation to fully write a movable but not copyable class.
#define BOOST_MOVABLE_BUT_NOT_COPYABLE(TYPE)\
   public:\
   typedef int boost_move_emulation_t;\
   private:\
   TYPE(const TYPE &);\
   TYPE& operator=(const TYPE &);\
//

//! This macro marks a type as copyable and movable.
//! The user will need to write a move constructor/assignment and a copy assignment
//! as explained in the documentation to fully write a copyable and movable class.
#define BOOST_COPYABLE_AND_MOVABLE(TYPE)\
//

/// @cond

#define BOOST_RV_REF_2_TEMPL_ARGS(TYPE, ARG1, ARG2)\
   TYPE<ARG1, ARG2> && \
//

#define BOOST_RV_REF_3_TEMPL_ARGS(TYPE, ARG1, ARG2, ARG3)\
   TYPE<ARG1, ARG2, ARG3> && \
//

/// @endcond

//!This macro is used to achieve portable syntax in move
//!constructors and assignments for classes marked as
//!BOOST_COPYABLE_AND_MOVABLE or BOOST_MOVABLE_BUT_NOT_COPYABLE
#define BOOST_RV_REF(TYPE)\
   TYPE && \
//

//!This macro is used to achieve portable syntax in copy
//!assignment for classes marked as BOOST_COPYABLE_AND_MOVABLE.
#define BOOST_COPY_ASSIGN_REF(TYPE)\
   const TYPE & \
//

/// @cond

#define BOOST_COPY_REF_2_TEMPL_ARGS(TYPE, ARG1, ARG2)\
   const TYPE<ARG1, ARG2> & \
//

#define BOOST_COPY_REF_3_TEMPL_ARGS(TYPE, ARG1, ARG2, ARG3)\
   TYPE<ARG1, ARG2, ARG3>& \
//

/// @endcond

//! This macro is used to implement portable perfect forwarding
//! as explained in the documentation.
#define BOOST_FWD_REF(TYPE)\
   TYPE && \
//

/// @cond

#define BOOST_CATCH_CONST_RLVALUE(TYPE)\
   const TYPE & \
//

/// @endcond

}  //namespace boost {

#endif   //BOOST_NO_RVALUE_REFERENCES

namespace boost {

//////////////////////////////////////////////////////////////////////////////
//
//                            move_iterator
//
//////////////////////////////////////////////////////////////////////////////

//! Class template move_iterator is an iterator adaptor with the same behavior
//! as the underlying iterator except that its dereference operator implicitly
//! converts the value returned by the underlying iterator's dereference operator
//! to an rvalue reference. Some generic algorithms can be called with move
//! iterators to replace copying with moving.
template <class It>
class move_iterator
{
   public:
   typedef It                                                              iterator_type;
   typedef typename std::iterator_traits<iterator_type>::value_type        value_type;
   #if !defined(BOOST_NO_RVALUE_REFERENCES) || defined(BOOST_MOVE_DOXYGEN_INVOKED)
   typedef value_type &&                                                   reference;
   #else
   typedef typename BOOST_MOVE_MPL_NS::if_
      < ::boost::has_move_emulation_enabled<value_type>
      , ::boost::rv<value_type>&
      , value_type & >::type                                               reference;
   #endif
   typedef It                                                              pointer;
   typedef typename std::iterator_traits<iterator_type>::difference_type   difference_type;
   typedef typename std::iterator_traits<iterator_type>::iterator_category iterator_category;

   move_iterator()
   {}

   explicit move_iterator(It i)
      :  m_it(i)
   {}

   template <class U>
   move_iterator(const move_iterator<U>& u)
      :  m_it(u.base())
   {}

   iterator_type base() const
   {  return m_it;   }

   reference operator*() const
   {
      #if defined(BOOST_NO_RVALUE_REFERENCES) || defined(BOOST_MOVE_OLD_RVALUE_REF_BINDING_RULES)
      return *m_it;
      #else
      return ::boost::move(*m_it);
      #endif
   }

   pointer   operator->() const
   {  return m_it;   }

   move_iterator& operator++()
   {  ++m_it; return *this;   }

   move_iterator<iterator_type>  operator++(int)
   {  move_iterator<iterator_type> tmp(*this); ++(*this); return tmp;   }

   move_iterator& operator--()
   {  --m_it; return *this;   }

   move_iterator<iterator_type>  operator--(int)
   {  move_iterator<iterator_type> tmp(*this); --(*this); return tmp;   }

   move_iterator<iterator_type>  operator+ (difference_type n) const
   {  return move_iterator<iterator_type>(m_it + n);  }

   move_iterator& operator+=(difference_type n)
   {  m_it += n; return *this;   }

   move_iterator<iterator_type>  operator- (difference_type n) const
   {  return move_iterator<iterator_type>(m_it - n);  }

   move_iterator& operator-=(difference_type n)
   {  m_it -= n; return *this;   }

   reference operator[](difference_type n) const
   {
      #if defined(BOOST_NO_RVALUE_REFERENCES) || defined(BOOST_MOVE_OLD_RVALUE_REF_BINDING_RULES)
      return m_it[n];
      #else
      return ::boost::move(m_it[n]);
      #endif
   }

   friend bool operator==(const move_iterator& x, const move_iterator& y)
   {  return x.base() == y.base();  }

   friend bool operator!=(const move_iterator& x, const move_iterator& y)
   {  return x.base() != y.base();  }

   friend bool operator< (const move_iterator& x, const move_iterator& y)
   {  return x.base() < y.base();   }

   friend bool operator<=(const move_iterator& x, const move_iterator& y)
   {  return x.base() <= y.base();  }

   friend bool operator> (const move_iterator& x, const move_iterator& y)
   {  return x.base() > y.base();  }

   friend bool operator>=(const move_iterator& x, const move_iterator& y)
   {  return x.base() >= y.base();  }

   friend difference_type operator-(const move_iterator& x, const move_iterator& y)
   {  return x.base() - y.base();   }

   friend move_iterator operator+(difference_type n, const move_iterator& x)
   {  return move_iterator(x.base() + n);   }

   private:
   It m_it;
};


//is_move_iterator
namespace move_detail {

template <class I>
struct is_move_iterator
   : public BOOST_MOVE_MPL_NS::integral_constant<bool, false>
{
};

template <class I>
struct is_move_iterator< ::boost::move_iterator<I> >
   : public BOOST_MOVE_MPL_NS::integral_constant<bool, true>
{
};

}  //namespace move_detail {

//////////////////////////////////////////////////////////////////////////////
//
//                            move_iterator
//
//////////////////////////////////////////////////////////////////////////////

//!
//! <b>Returns</b>: move_iterator<It>(i).
template<class It>
move_iterator<It> make_move_iterator(const It &it)
{  return move_iterator<It>(it); }

//////////////////////////////////////////////////////////////////////////////
//
//                         back_move_insert_iterator
//
//////////////////////////////////////////////////////////////////////////////


//! A move insert iterator that move constructs elements at the
//! back of a container
template <typename C> // C models Container
class back_move_insert_iterator
   : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
   C* container_m;

   public:
   typedef C container_type;

   explicit back_move_insert_iterator(C& x) : container_m(&x) { }

   back_move_insert_iterator& operator=(typename C::reference x)
   { container_m->push_back(boost::move(x)); return *this; }

   back_move_insert_iterator& operator*()     { return *this; }
   back_move_insert_iterator& operator++()    { return *this; }
   back_move_insert_iterator& operator++(int) { return *this; }
};

//!
//! <b>Returns</b>: back_move_insert_iterator<C>(x).
template <typename C> // C models Container
inline back_move_insert_iterator<C> back_move_inserter(C& x)
{
   return back_move_insert_iterator<C>(x);
}

//////////////////////////////////////////////////////////////////////////////
//
//                         front_move_insert_iterator
//
//////////////////////////////////////////////////////////////////////////////

//! A move insert iterator that move constructs elements int the
//! front of a container
template <typename C> // C models Container
class front_move_insert_iterator
   : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
   C* container_m;

public:
   typedef C container_type;

   explicit front_move_insert_iterator(C& x) : container_m(&x) { }

   front_move_insert_iterator& operator=(typename C::reference x)
   { container_m->push_front(boost::move(x)); return *this; }

   front_move_insert_iterator& operator*()     { return *this; }
   front_move_insert_iterator& operator++()    { return *this; }
   front_move_insert_iterator& operator++(int) { return *this; }
};

//!
//! <b>Returns</b>: front_move_insert_iterator<C>(x).
template <typename C> // C models Container
inline front_move_insert_iterator<C> front_move_inserter(C& x)
{
   return front_move_insert_iterator<C>(x);
}

//////////////////////////////////////////////////////////////////////////////
//
//                         insert_move_iterator
//
//////////////////////////////////////////////////////////////////////////////
template <typename C> // C models Container
class move_insert_iterator
   : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
   C* container_m;
   typename C::iterator pos_;

   public:
   typedef C container_type;

   explicit move_insert_iterator(C& x, typename C::iterator pos)
      : container_m(&x), pos_(pos)
   {}

   move_insert_iterator& operator=(typename C::reference x)
   {
      pos_ = container_m->insert(pos_, ::boost::move(x));
      ++pos_;
      return *this;
   }

   move_insert_iterator& operator*()     { return *this; }
   move_insert_iterator& operator++()    { return *this; }
   move_insert_iterator& operator++(int) { return *this; }
};

//!
//! <b>Returns</b>: move_insert_iterator<C>(x, it).
template <typename C> // C models Container
inline move_insert_iterator<C> move_inserter(C& x, typename C::iterator it)
{
   return move_insert_iterator<C>(x, it);
}

//////////////////////////////////////////////////////////////////////////////
//
//                               move
//
//////////////////////////////////////////////////////////////////////////////


//! <b>Effects</b>: Moves elements in the range [first,last) into the range [result,result + (last -
//!   first)) starting from first and proceeding to last. For each non-negative integer n < (last-first),
//!   performs *(result + n) = ::boost::move (*(first + n)).
//!
//! <b>Effects</b>: result + (last - first).
//!
//! <b>Requires</b>: result shall not be in the range [first,last).
//!
//! <b>Complexity</b>: Exactly last - first move assignments.
template <typename I, // I models InputIterator
          typename O> // O models OutputIterator
O move(I f, I l, O result)
{
   while (f != l) {
      *result = ::boost::move(*f);
      ++f; ++result;
   }
   return result;
}

//////////////////////////////////////////////////////////////////////////////
//
//                               move_backward
//
//////////////////////////////////////////////////////////////////////////////

//! <b>Effects</b>: Moves elements in the range [first,last) into the range
//!   [result - (last-first),result) starting from last - 1 and proceeding to
//!   first. For each positive integer n <= (last - first),
//!   performs *(result - n) = ::boost::move(*(last - n)).
//!
//! <b>Requires</b>: result shall not be in the range [first,last).
//!
//! <b>Returns</b>: result - (last - first).
//!
//! <b>Complexity</b>: Exactly last - first assignments.
template <typename I, // I models BidirectionalIterator
typename O> // O models BidirectionalIterator
O move_backward(I f, I l, O result)
{
   while (f != l) {
      --l; --result;
      *result = ::boost::move(*l);
   }
   return result;
}

//////////////////////////////////////////////////////////////////////////////
//
//                               uninitialized_move
//
//////////////////////////////////////////////////////////////////////////////

//! <b>Effects</b>:
//!   \code
//!   for (; first != last; ++result, ++first)
//!      new (static_cast<void*>(&*result))
//!         typename iterator_traits<ForwardIterator>::value_type(boost::move(*first));
//!   \endcode
//!
//! <b>Returns</b>: result
template
   <typename I, // I models InputIterator
    typename F> // F models ForwardIterator
F uninitialized_move(I f, I l, F r
   /// @cond
//   ,typename BOOST_MOVE_BOOST_NS::enable_if<has_move_emulation_enabled<typename std::iterator_traits<I>::value_type> >::type* = 0
   /// @endcond
   )
{
   typedef typename std::iterator_traits<I>::value_type input_value_type;
   while (f != l) {
      ::new(static_cast<void*>(&*r)) input_value_type(boost::move(*f));
      ++f; ++r;
   }
   return r;
}

/// @cond
/*
template
   <typename I,   // I models InputIterator
    typename F>   // F models ForwardIterator
F uninitialized_move(I f, I l, F r,
   typename BOOST_MOVE_BOOST_NS::disable_if<has_move_emulation_enabled<typename std::iterator_traits<I>::value_type> >::type* = 0)
{
   return std::uninitialized_copy(f, l, r);
}
*/
//////////////////////////////////////////////////////////////////////////////
//
//                            uninitialized_copy_or_move
//
//////////////////////////////////////////////////////////////////////////////

namespace move_detail {

template
<typename I,   // I models InputIterator
typename F>   // F models ForwardIterator
F uninitialized_move_move_iterator(I f, I l, F r
//                             ,typename BOOST_MOVE_BOOST_NS::enable_if< has_move_emulation_enabled<typename I::value_type> >::type* = 0
)
{
   return ::boost::uninitialized_move(f, l, r);
}
/*
template
<typename I,   // I models InputIterator
typename F>   // F models ForwardIterator
F uninitialized_move_move_iterator(I f, I l, F r,
                                   typename BOOST_MOVE_BOOST_NS::disable_if< has_move_emulation_enabled<typename I::value_type> >::type* = 0)
{
   return std::uninitialized_copy(f.base(), l.base(), r);
}
*/
}  //namespace move_detail {

template
<typename I,   // I models InputIterator
typename F>   // F models ForwardIterator
F uninitialized_copy_or_move(I f, I l, F r,
                             typename BOOST_MOVE_BOOST_NS::enable_if< move_detail::is_move_iterator<I> >::type* = 0)
{
   return ::boost::move_detail::uninitialized_move_move_iterator(f, l, r);
}

//////////////////////////////////////////////////////////////////////////////
//
//                            copy_or_move
//
//////////////////////////////////////////////////////////////////////////////

namespace move_detail {

template
<typename I,   // I models InputIterator
typename F>   // F models ForwardIterator
F move_move_iterator(I f, I l, F r
//                             ,typename BOOST_MOVE_BOOST_NS::enable_if< has_move_emulation_enabled<typename I::value_type> >::type* = 0
)
{
   return ::boost::move(f, l, r);
}
/*
template
<typename I,   // I models InputIterator
typename F>   // F models ForwardIterator
F move_move_iterator(I f, I l, F r,
                                   typename BOOST_MOVE_BOOST_NS::disable_if< has_move_emulation_enabled<typename I::value_type> >::type* = 0)
{
   return std::copy(f.base(), l.base(), r);
}
*/

}  //namespace move_detail {

template
<typename I,   // I models InputIterator
typename F>   // F models ForwardIterator
F copy_or_move(I f, I l, F r,
                             typename BOOST_MOVE_BOOST_NS::enable_if< move_detail::is_move_iterator<I> >::type* = 0)
{
   return ::boost::move_detail::move_move_iterator(f, l, r);
}

/// @endcond

//! <b>Effects</b>:
//!   \code
//!   for (; first != last; ++result, ++first)
//!      new (static_cast<void*>(&*result))
//!         typename iterator_traits<ForwardIterator>::value_type(*first);
//!   \endcode
//!
//! <b>Returns</b>: result
//!
//! <b>Note</b>: This function is provided because
//!   <i>std::uninitialized_copy</i> from some STL implementations
//!    is not compatible with <i>move_iterator</i>
template
<typename I,   // I models InputIterator
typename F>   // F models ForwardIterator
F uninitialized_copy_or_move(I f, I l, F r
   /// @cond
   ,typename BOOST_MOVE_BOOST_NS::disable_if< move_detail::is_move_iterator<I> >::type* = 0
   /// @endcond
   )
{
   return std::uninitialized_copy(f, l, r);
}

//! <b>Effects</b>:
//!   \code
//!   for (; first != last; ++result, ++first)
//!      *result = *first;
//!   \endcode
//!
//! <b>Returns</b>: result
//!
//! <b>Note</b>: This function is provided because
//!   <i>std::uninitialized_copy</i> from some STL implementations
//!    is not compatible with <i>move_iterator</i>
template
<typename I,   // I models InputIterator
typename F>   // F models ForwardIterator
F copy_or_move(I f, I l, F r
   /// @cond
   ,typename BOOST_MOVE_BOOST_NS::disable_if< move_detail::is_move_iterator<I> >::type* = 0
   /// @endcond
   )
{
   return std::copy(f, l, r);
}

//! If this trait yields to true
//! (<i>has_trivial_destructor_after_move &lt;T&gt;::value == true</i>)
//! means that if T is used as argument of a move construction/assignment,
//! there is no need to call T's destructor.
//! This optimization tipically is used to improve containers' performance.
//!
//! By default this trait is true if the type has trivial destructor,
//! every class should specialize this trait if it wants to improve performance
//! when inserted in containers.
template <class T>
struct has_trivial_destructor_after_move
   : BOOST_MOVE_BOOST_NS::has_trivial_destructor<T>
{};

}  //namespace boost {

#endif //#ifndef BOOST_MOVE_MOVE_HPP
