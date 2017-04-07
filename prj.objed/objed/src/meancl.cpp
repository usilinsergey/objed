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

#include "meancl.h"

#include <cassert>

template<class T>
inline static bool intervalContains(const objed::Interval<T> &interval, T value)
{
  return value >= interval.mn && value <= interval.mx;
}

objed::MeanClassifier::MeanClassifier(int width, int height) : 
clWidth(width), clHeight(height)
{
  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);
}

objed::MeanClassifier::MeanClassifier(const Json::Value &data) :
clWidth(0), clHeight(0)
{
  clWidth = data["width"].asInt();
  clHeight = data["height"].asInt();

  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);

  Json::Value meanListData = data["meanList"];
  if (meanListData.isArray() && meanListData.size() > 0)
  {
    for (Json::UInt i = 0; i < meanListData.size(); i++)
    {
      intervalList.push_back(dataToInterval<int>(meanListData[i]));
      preprocList.push_back(meanListData[i]["preproc"].asString());
    }
  }
}

objed::MeanClassifier::~MeanClassifier()
{
  return;
}

int objed::MeanClassifier::width() const
{
  return clWidth;
}

int objed::MeanClassifier::height() const
{
  return clHeight;
}

bool objed::MeanClassifier::prepare(ImagePool *imagePool)
{
  assert(intervalList.size() == preprocList.size());

  integralList.clear();
  for (size_t i = 0; i < preprocList.size(); i++)
    integralList.push_back(imagePool->integral(preprocList[i]));

  return true;
}

bool objed::MeanClassifier::evaluate(float *result, int x, int y, DebugInfo *debugInfo) const
{
  assert(result != 0);
  assert(intervalList.size() == preprocList.size());
  assert(integralList.size() == preprocList.size());

  const int wd = clWidth, wd2 = clWidth / 2;
  const int ht = clHeight, ht2 = clHeight / 2;

  for (size_t i = 0; i < integralList.size(); i++)
  {
    int aver = rectAver(integralList[i], x - wd2, y - ht2, wd, ht);
    if (intervalContains(intervalList[i], aver) == false)
    {
      *result = -1.0;
      return true;
    }
  }

  *result = 1.0;
  return true;
}

Json::Value objed::MeanClassifier::serialize() const
{
  Json::Value data;

  Json::Value meanListData(Json::arrayValue);
  for (size_t i = 0; i < preprocList.size(); i++)
  {
    Json::Value meanData = intervalToData(intervalList[i]);
    meanData["preproc"] = preprocList[i];
    meanListData.append(meanData);
  }

  data["type"] = type();
  data["width"] = clWidth;
  data["height"] = clHeight;
  data["meanList"] = meanListData;

  return data;
}

objed::Classifier * objed::MeanClassifier::clone() const
{
  MeanClassifier *cl = new MeanClassifier(width(), height());
  assert(cl != 0);

  cl->preprocList = this->preprocList;
  cl->intervalList = this->intervalList;

  return cl;
}
