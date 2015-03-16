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

#include "haar2stumpcl.h"
#include "imgutils.h"

#include <cstring>
#include <cassert>

objed::Haar2StumpClassifier::Haar2StumpClassifier(int width, int height) : 
clWidth(width), clHeight(height), threshold(0), normalize(false), integral(0)
{
  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);
  memset(values, 0, sizeof(values));
}

objed::Haar2StumpClassifier::Haar2StumpClassifier(const Json::Value &data) : 
clWidth(0), clHeight(0), threshold(0), normalize(false), integral(0)
{
  clWidth = data["width"].asInt();
  clHeight = data["height"].asInt();

  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);

  normalize = data["normalize"].asBool();
  rect0 = dataToRect<int>(data["rect0"]);
  rect1 = dataToRect<int>(data["rect1"]);
  threshold = data["threshold"].asInt();
  preproc = data["preproc"].asString();

  assert(data["values"].isArray() == true);
  values[0] = static_cast<float>(data["values"][(Json::UInt)0].asDouble());
  values[1] = static_cast<float>(data["values"][(Json::UInt)1].asDouble());
}

objed::Haar2StumpClassifier::~Haar2StumpClassifier()
{
  return;
}

int objed::Haar2StumpClassifier::width() const
{
  return clWidth;
}

int objed::Haar2StumpClassifier::height() const
{
  return clHeight;
}

bool objed::Haar2StumpClassifier::prepare(ImagePool *imagePool)
{
  assert(preproc.length() > 0);
  integral = imagePool->integral(preproc);

  assert(integral != 0);
  return integral != 0;
}

bool objed::Haar2StumpClassifier::evaluate(float *result, int x, int y) const
{
  *result = compute(integral, x, y) > threshold ? values[0] : values[1];
  return true;
}

Json::Value objed::Haar2StumpClassifier::serialize() const
{
  Json::Value data;
  Json::Value valuesData;
  valuesData.append(values[0]);
  valuesData.append(values[1]);

  data["type"] = type();
  data["width"] = clWidth;
  data["height"] = clHeight;
  data["preproc"] = preproc;
  data["values"] = valuesData;
  data["threshold"] = threshold;
  data["normalize"] = normalize;
  data["rect0"] = rectToData(rect0);
  data["rect1"] = rectToData(rect1);

  return data;
}

objed::Classifier * objed::Haar2StumpClassifier::clone() const
{
  Haar2StumpClassifier *cl = new Haar2StumpClassifier(width(), height());
  assert(cl != 0);

  cl->preproc = this->preproc;
  cl->threshold = this->threshold;
  cl->normalize = this->normalize;
  cl->rect0 = this->rect0;
  cl->rect1 = this->rect1;

  memcpy(cl->values, this->values, sizeof(cl->values));

  return cl;
}

