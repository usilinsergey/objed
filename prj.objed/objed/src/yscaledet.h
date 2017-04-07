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
#ifndef YSCALEDET_H_INClUDED
#define YSCALEDET_H_INClUDED

#include <objed/objed.h>
#include <objed/objedutils.h>

namespace objed
{
  class YScaleDetector : public Detector
  {
    OBJED_TYPE("yScaleDetector")
    OBJED_DISABLE_COPY(YScaleDetector)

  public:
    YScaleDetector();
    YScaleDetector(const Json::Value &data, const std::string &workDir);
    virtual ~YScaleDetector();

  public:
    virtual DetectionList detect(IplImage *image, DebugInfo *debugInfo);

    virtual Detector * clone() const;

    virtual void setImagePool(objed::ImagePool *extImagePool);
    virtual void resetImagePool();

  private:
    ImagePool *imagePool;
    bool isImagePoolOwn;

  private:
    Classifier *classifier;

  private:
    double y0, y1, yStp;
    double y0Scale, y1Scale;
    int leftMargin, rightMargin;
    int topMargin, bottomMargin;
    int xStep, yStep;
    bool mergeIncluded;
    double overlap;
  };
}

#endif  // YSCALEDET_H_INClUDED