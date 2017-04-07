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

#include "cascadecl.h"

#include <cassert>

objed::CascadeClassifier::CascadeClassifier(int width, int height) :
clWidth(width), clHeight(height)
{
  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);
}

objed::CascadeClassifier::CascadeClassifier(const Json::Value &data) : 
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

objed::CascadeClassifier::~CascadeClassifier()
{
  for (size_t i = 0; i < clList.size(); i++)
     Classifier::destroy(clList[i]);
}

int objed::CascadeClassifier::width() const
{
  return clWidth;
}

int objed::CascadeClassifier::height() const
{
  return clHeight;
}

bool objed::CascadeClassifier::prepare(ImagePool *imagePool)
{
  bool ok = true;
  for (size_t i = 0; i < clList.size(); i++)
    ok &= clList[i]->prepare(imagePool);

  return ok;
}

bool objed::CascadeClassifier::evaluate(float *result, int x, int y, DebugInfo *debugInfo) const
{
  switch(clList.size())
  {
    case 0: return false;
    case 1: return evaluate1(result, x, y, debugInfo);
    case 2: return evaluate2(result, x, y, debugInfo);
    case 3: return evaluate3(result, x, y, debugInfo);
    case 4: return evaluate4(result, x, y, debugInfo);
    case 5: return evaluate5(result, x, y, debugInfo);
    default: return evaluateN(result, x, y, debugInfo);
  }
}

Json::Value objed::CascadeClassifier::serialize() const
{
  Json::Value data;
  Json::Value scListData;

  for (size_t i = 0; i < clList.size(); i++)
    scListData.append(clList[i]->serialize());

  data["type"] = type();
  data["clList"] = scListData;
  data["width"] = clWidth;
  data["height"] = clHeight;

  return data;
}

bool objed::CascadeClassifier::evaluate1(float *result, int x, int y, DebugInfo *debugInfo) const
{
  assert(clList.size() >= 1);

  return clList[0]->evaluate(result, x, y, debugInfo);
}

bool objed::CascadeClassifier::evaluate2(float *result, int x, int y, DebugInfo *debugInfo) const
{
  assert(clList.size() >= 2);
  bool ok = false;

  ok = clList[0]->evaluate(result, x, y, debugInfo);
  if (ok == false || *result < 0.0)
    return ok;

  return clList[1]->evaluate(result, x, y, debugInfo);
}

bool objed::CascadeClassifier::evaluate3(float *result, int x, int y, DebugInfo *debugInfo) const
{
  assert(clList.size() >= 3);
  bool ok = false;

  ok = clList[0]->evaluate(result, x, y, debugInfo);
  if (ok == false || *result < 0.0)
    return ok;

  ok = clList[1]->evaluate(result, x, y, debugInfo);
  if (ok == false || *result < 0.0)
    return ok;

  return clList[2]->evaluate(result, x, y, debugInfo);
}

bool objed::CascadeClassifier::evaluate4(float *result, int x, int y, DebugInfo *debugInfo) const
{
  assert(clList.size() >= 4);
  bool ok = false;

  ok = clList[0]->evaluate(result, x, y, debugInfo);
  if (ok == false || *result < 0.0)
    return ok;

  ok = clList[1]->evaluate(result, x, y, debugInfo);
  if (ok == false || *result < 0.0)
    return ok;

  ok = clList[2]->evaluate(result, x, y, debugInfo);
  if (ok == false || *result < 0.0)
    return ok;

  return clList[3]->evaluate(result, x, y, debugInfo);
}

bool objed::CascadeClassifier::evaluate5(float *result, int x, int y, DebugInfo *debugInfo) const
{
  assert(clList.size() >= 5);
  bool ok = false;

  ok = clList[0]->evaluate(result, x, y, debugInfo);
  if (ok == false || *result < 0.0)
    return ok;

  ok = clList[1]->evaluate(result, x, y, debugInfo);
  if (ok == false || *result < 0.0)
    return ok;

  ok = clList[2]->evaluate(result, x, y, debugInfo);
  if (ok == false || *result < 0.0)
    return ok;

  ok = clList[3]->evaluate(result, x, y, debugInfo);
  if (ok == false || *result < 0.0)
    return ok;

  return clList[4]->evaluate(result, x, y, debugInfo);
}

bool objed::CascadeClassifier::evaluateN(float *result, int x, int y, DebugInfo *debugInfo) const
{
  assert(clList.size() >= 6);
  *result = objed::epsilon;
  bool ok = true;

  for (size_t i = 0; i < clList.size() && *result > 0.0; i++)
    ok &= clList[i]->evaluate(result, x, y, debugInfo);

  return ok;
}

objed::Classifier * objed::CascadeClassifier::clone() const
{
  CascadeClassifier *newCascadeCl = new CascadeClassifier(width(), height());
  assert(newCascadeCl != 0);

  for (size_t i = 0; i < this->clList.size(); i++)
    newCascadeCl->clList.push_back(this->clList[i]->clone());

  return newCascadeCl;
}
