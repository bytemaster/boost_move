//-----------------------------------------------------------------------------
// boost move/moveable.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2002-2003
// Eric Friedman
//
// See below original copyright by Andrei Alexandrescu.
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee, 
// provided that the above copyright notice appears in all copies and 
// that both the copyright notice and this permission notice appear in 
// supporting documentation. No representations are made about the 
// suitability of this software for any purpose. It is provided "as is" 
// without express or implied warranty.

#ifndef BOOST_MOVE_MOVEABLE_HPP
#define BOOST_MOVE_MOVEABLE_HPP

#include "boost/utility/addressof.hpp"

namespace boost {

namespace detail {

//////////////////////////////////////////////////////////////////////////
// (detail) class template const_lvalue_ref
//
// Returned from moveable to distinguish from rvalue references (i.e.,
// move_sources).
//
template <typename T>
class const_lvalue_ref
{
private: // representation

    const T* data_;

public: // structors

    explicit const_lvalue_ref(const T& obj)
        : data_(boost::addressof(obj))
    {
    }

public: // queries

    const T& get() const
    {
        return *data_;
    }

};

} // namespace detail

//////////////////////////////////////////////////////////////////////////
// class template move_source
//
// Passed to object constructors and assignment operators indicating to
// perform a move operation (i.e., instead of a copy operation).
//
template <typename T>
class move_source
    : private detail::const_lvalue_ref<T>
{
public: // structors

    explicit move_source(T& obj)
        : detail::const_lvalue_ref<T>(obj)
    {
    }

public: // queries

    T& get() const
    {
        return const_cast<T&>(detail::const_lvalue_ref<T>::get());
    }

};

//////////////////////////////////////////////////////////////////////////
// class template move_return
//
// Facilitates return-by-move for function return values.
//
// Example usage for function f and moveable type X:
//   move_return< X > f( ... );
//
template <typename T> 
class move_return
{
private: // representation

    T returned_;

public: // structors

    explicit move_return(T& returned)
        : returned_(move_source<T>(returned))
    {
    }

    // "The cast below is valid given that nobody ever really creates a 
    //  const move_return object..."

    move_return(const move_return& operand)
        : returned_(const_cast<move_return&>(operand))
    {
    }

public: // operators

    operator move_source<T>()
    {
        return move_source<T>(returned_);
    }

};

//////////////////////////////////////////////////////////////////////////
// class template moveable
//
// Derived by types seeking to support move-construction and move-assign.
//
template <typename Deriving>
class moveable
{
public: // operators

    operator detail::const_lvalue_ref<Deriving>() const
    {
        return detail::const_lvalue_ref<Deriving>(
              static_cast<const Deriving&>(*this)
            );
    }

    operator move_source<Deriving>()
    {
        return move_source<Deriving>(
              static_cast<Deriving&>(*this)
            );
    }

    operator move_return<Deriving>()
    {
        return move_return<Deriving>(
              static_cast<Deriving&>(*this)
            );
    }

protected: // noninstantiable

    moveable() { }
    ~moveable() { }

};

} // namespace boost

#endif // BOOST_MOVE_MOVEABLE_HPP


/* This file derivative of MoJO. Much thanks to Andrei for his initial work.
 * See <http://www.cuj.com/experts/2102/alexandr.htm> for information on MOJO.

 * Original copyright -- on mojo.h -- follows:

////////////////////////////////////////////////////////////////////////////////
// MOJO: MOving Joint Objects
// Copyright (c) 2002 by Andrei Alexandrescu
//
// Created by Andrei Alexandrescu
//
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author makes no representations about the suitability of this software 
//     for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

*/
