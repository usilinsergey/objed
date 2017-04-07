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
#ifndef HAAR3PWCL_H_INCLUDED
#define HAAR3PWCL_H_INCLUDED

#include <objed/objed.h>
#include <objed/objedutils.h>

namespace objed
{
  class Haar3PwClassifier : public Classifier
  {
    OBJED_TYPE("haar3PwClassifier")
    OBJED_DISABLE_COPY(Haar3PwClassifier)

  public:
    Haar3PwClassifier(int width, int height);
    Haar3PwClassifier(const Json::Value &data);
    virtual ~Haar3PwClassifier();

  public:
    virtual int width() const;
    virtual int height() const;
    virtual bool prepare(ImagePool *imagePool);
    virtual bool evaluate(float *result, int x, int y, DebugInfo *debugInfo) const;
    virtual Json::Value serialize() const;
    virtual Classifier * clone() const;

  public:
    inline int compute(const IplImage *integral, int x, int y) const
    {
      int x0 = x + rect0.x, y0 = y + rect0.y;
      int x1 = x + rect1.x, y1 = y + rect1.y;
      int x2 = x + rect2.x, y2 = y + rect2.y;
      int sum0 = rectSum(integral, x0, y0, rect0.width, rect0.height);
      int sum1 = rectSum(integral, x1, y1, rect1.width, rect1.height);
      int sum2 = rectSum(integral, x2, y2, rect2.width, rect2.height);

      if (normalize == true)
      {
        return 255 * (sum0 + sum2) / (sum0 + sum1 + sum2 + 1);
      }
      else
      {
        int sq0 = rect0.width * rect0.height, sq1 = rect1.width * rect1.height, sq2 = rect2.width * rect2.height;
        return (sum0 / sq0 - sum1 / sq1 + sum2 / sq2 + 255) / 3;
      }
    }

  public:
    const IplImage *integral;

  public:
    int clWidth, clHeight;
    Rect<int> rect0, rect1, rect2;
    std::string preproc;
    std::vector<float> bins;
    bool normalize;
  };
}

#endif  // HAAR3PWCL_H_INCLUDED