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
#ifndef OBJEDUTILS_H_INCLUDED
#define OBJEDUTILS_H_INCLUDED

#include <objed/simplest.h>
#include <json-cpp/json.h>

#include <cassert>
#include <cstdio>

#include <opencv/cv.h>

namespace objed
{
#define INCREASE_SC_COUNT(debugInfo) \
  if (debugInfo != nullptr) debugInfo->int_data[Classifier::DBG_SC_COUNT]++;

  template<class T> Json::Value rectToData(const Rect<T> &rect);

  template<class T> Rect<T> dataToRect(const Json::Value &data);

  template<class T> Json::Value intervalToData(const Interval<T> &interval);

  template<class T> Interval<T> dataToInterval(const Json::Value &data);

  inline int rectSum(const IplImage *integral, int x, int y, int width, int height)
  {
    assert(x >= 0 && y >= 0 && width > 0 && height > 0);
    assert(x < integral->width && x + width < integral->width);
    assert(y < integral->height && y + height < integral->height);

    const int *line1 = (int *)(integral->imageData + y * integral->widthStep);
    const int *line2 = (int *)(integral->imageData + (y + height) * integral->widthStep);

    return line2[x + width] - line1[x + width] - line2[x] + line1[x];
  }

  inline int rectAver(const IplImage *integral, int x, int y, int width, int height)
  {
    assert(width > 0 && height > 0);
    return rectSum(integral, x, y, width, height) / (width * height);
  }

  template<class T> static inline Point<T> center(const Rect<T> &rect)
  {
    return Point<T>(rect.x + rect.width / 2, rect.y + rect.height / 2);
  }

  template<class T> Rect<T> static inline united(const Rect<T> &rect1, const Rect<T> &rect2)
  {
    const T x = std::min(rect1.x, rect2.x), y = std::min(rect1.y, rect2.y);
    const T width = std::max(rect1.x + rect1.width, rect2.x + rect2.width) - x;
    const T height = std::max(rect1.y + rect1.height, rect2.y + rect2.height) - y;
    return Rect<T>(x, y, width, height);
  }

  template<class T> Rect<T> static inline intersected(const Rect<T> &rect1, const Rect<T> &rect2)
  {
    const T x = std::max(rect1.x, rect2.x), y = std::max(rect1.y, rect2.y);
    const T width = std::min(rect1.x + rect1.width, rect2.x + rect2.width) - x;
    const T height = std::min(rect1.y + rect1.height, rect2.y + rect2.height) - y;
    return (width <= 0 || height <= 0) ? Rect<T>() : Rect<T>(x, y, width, height);
  }

  template<class T> std::vector<Cluster<T> > cluster(const std::vector<Rect<T> > &rectList, double overlap);

  template<class T> std::vector<Cluster<T> > cluster(const std::vector<Cluster<T> > &clusterList, double overlap);

  template<class T> std::vector<Cluster<T> > mergeIncluded(const std::vector<Cluster<T> > &clusterList);
}

#endif  // OBJEDUTILS_H_INCLUDED
