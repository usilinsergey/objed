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
#ifndef PARALLELCL_H_INCLUDED
#define PARALLELCL_H_INCLUDED

#include <objed/objed.h>
#include <objed/objedutils.h>

#include <vector>

namespace objed
{
  class ParallelClassifier : public Classifier
  {
    OBJED_TYPE("parallelClassifier")
    OBJED_DISABLE_COPY(ParallelClassifier)

  public:
    ParallelClassifier(int width, int height);
    ParallelClassifier(const Json::Value &data);
    virtual ~ParallelClassifier();

  public:
    virtual int width() const;
    virtual int height() const;
    virtual bool prepare(ImagePool *imagePool);
    virtual bool evaluate(float *result, int x, int y, DebugInfo *debugInfo) const;
    virtual Json::Value serialize() const;
    virtual Classifier * clone() const;

  private:
    bool evaluate1(float *result, int x, int y) const;
    bool evaluate2(float *result, int x, int y) const;
    bool evaluate3(float *result, int x, int y) const;
    bool evaluate4(float *result, int x, int y) const;
    bool evaluate5(float *result, int x, int y) const;
    bool evaluateN(float *result, int x, int y) const;

  public:
    int clWidth, clHeight;
    std::vector<Classifier *> clList;
  };
}

#endif  // PARALLELCL_H_INCLUDED