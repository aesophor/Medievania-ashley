// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_STD_MAKE_UNIQUE_H_
#define VIGILANTE_STD_MAKE_UNIQUE_H_


// Provides make_unique if compiling with C++11
#if __cplusplus >= 201103L && __cplusplus < 201402L
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace std {

template<class T>
struct _Unique_if {
  typedef unique_ptr<T> _Single_object;
};

template<class T>
struct _Unique_if<T[]> {
  typedef unique_ptr<T[]> _Unknown_bound;
};

template<class T, size_t N>
struct _Unique_if<T[N]> {
  typedef void _Known_bound;
};

template<class T, class... Args>
typename _Unique_if<T>::_Single_object
make_unique(Args&&... args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T>
typename _Unique_if<T>::_Unknown_bound
make_unique(size_t n) {
  typedef typename remove_extent<T>::type U;
  return unique_ptr<T>(new U[n]());
}

template<class T, class... Args>
typename _Unique_if<T>::_Known_bound
make_unique(Args&&...) = delete;

}  // namespace std

#endif


#endif  // VIGILANTE_STD_MAKE_UNIQUE_H_
