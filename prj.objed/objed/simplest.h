/*
Copyright (c) 2011-2013, Sergey Usilin. All rights reserved.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of copyright holders.
*/

#pragma once
#ifndef SIMPLEST_H_INCLUDED
#define SIMPLEST_H_INCLUDED

#include <utility>
#include <vector>
#include <string>

namespace objed
{
  #define OBJED_DISABLE_COPY(Class) \
  private: \
    Class(const Class &); \
    Class &operator=(const Class &);

  #define OBJED_TYPE(Type) \
  public: \
    static std::string typeStatic() { return Type; } \
    std::string type() const { return Type; }

  const float epsilon = 0.000001f;

  template<class T> inline int round(T value)
  {
    return static_cast<int>(value + 0.5);
  }

  template<class T> class Rect
  {
  public:
    Rect<T>() : x(0), y(0), width(0), height(0) {}
    Rect<T>(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) {};

  public:
    T x, y, width, height;
  };

  template<class T> class Point
  {
  public:
    Point<T>() : x(0), y(0) {}
    Point<T>(T x, T y) : x(x), y(y) {}

  public:
    T x, y;
  };

  template<class T> class Size
  {
  public:
    Size<T>() : width(0), height(0) {}
    Size<T>(T width, T height) : width(width), height(height) {}

  public:
    T width, height;
  };

  template<class T> class Interval
  {
  public:
    Interval<T>() : mn(0), mx(0) {}
    Interval<T>(T mn, T mx) : mn(mn), mx(mx) {}

  public:
    T mn, mx;
  };

  template<class T> class Cluster : public Rect<T>
  {
  public:
    Cluster<T>() : Rect<T>(), power(0) {}
    Cluster<T>(const Rect<T> &rect, int power = 1) : Rect<T>(rect), power(power) {}
    Cluster<T>(T x, T y, T width, T height, int power = 1) : Rect<T>(x, y, width, height), power(power) {}

  public:
    int power;
  };

  typedef Cluster<int> Detection;
  typedef std::vector<Detection> DetectionList;
}

#endif  // SIMPLEST_H_INCLUDED