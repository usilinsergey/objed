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

#include "additivecl.h"

#include <cassert>

#define INCREASE_DBG_OUPUT_LEVEL \
  if (debugInfo != 0) debugInfo->int_data[DBG_OUPUT_LEVEL]++;

objed::AdditiveClassifier::AdditiveClassifier(int width, int height) :
clWidth(width), clHeight(height)
{
  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);
}

objed::AdditiveClassifier::AdditiveClassifier(const Json::Value &data) : 
clWidth(0), clHeight(0)
{
  clWidth = data["width"].asInt();
  clHeight = data["height"].asInt();

  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);

  Json::Value clListData = data["clList"];
  assert(clListData.isArray());

  if (clListData.isArray() && clListData.size() > 0)
  {
    for (size_t i = 0; i < clListData.size(); i++)
    {
      Json::Value clData = clListData[i];
      Classifier *cl = Classifier::create(clData);

      assert(cl != 0);
      if (cl == 0)
        continue;

      assert(cl->width() <= clWidth);
      assert(cl->height() <= clHeight);

      clList.push_back(cl);
    }    
  }
}

objed::AdditiveClassifier::~AdditiveClassifier()
{
  for (size_t i = 0; i < clList.size(); i++)
    Classifier::destroy(clList[i]);
}

int objed::AdditiveClassifier::width() const
{
  return clWidth;
}

int objed::AdditiveClassifier::height() const
{
  return clHeight;
}

bool objed::AdditiveClassifier::prepare(ImagePool *imagePool)
{
  bool ok = true;
  for (size_t i = 0; i < clList.size(); i++)
  {
    assert(clList[i] != 0);
    ok &= clList[i]->prepare(imagePool);
  }

  return ok;
}

bool objed::AdditiveClassifier::evaluate(float *result, int x, int y, DebugInfo *debugInfo) const
{
  bool ok = true;
  float clResult = 0.0;
  float totalResult = 0.0;

  for (size_t i = 0; i < clList.size(); i++)
  {
    assert(clList[i] != 0);
    ok &= clList[i]->evaluate(&clResult, x, y, debugInfo);
    totalResult += clResult;
  }

  INCREASE_SC_COUNT(debugInfo);
  *result = totalResult;
  return ok;
}

Json::Value objed::AdditiveClassifier::serialize() const
{
  Json::Value data;
  Json::Value clListData;

  for (size_t i = 0; i < clList.size(); i++)
    clListData.append(clList[i]->serialize());

  data["type"] = type();
  data["clList"] = clListData;
  data["width"] = clWidth;
  data["height"] = clHeight;

  return data;
}

objed::Classifier * objed::AdditiveClassifier::clone() const
{
  AdditiveClassifier *newAdditiveCl = new AdditiveClassifier(width(), height());
  assert(newAdditiveCl != 0);

  for (size_t i = 0; i < this->clList.size(); i++)
    newAdditiveCl->clList.push_back(this->clList[i]->clone());

  return newAdditiveCl;
}
