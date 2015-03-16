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

#include "haar2pwcl.h"
#include "imgutils.h"

#include <cstring>
#include <cassert>

objed::Haar2PwClassifier::Haar2PwClassifier(int width, int height):
clWidth(width), clHeight(height), normalize(false), integral(0)
{
  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);
}

objed::Haar2PwClassifier::Haar2PwClassifier(const Json::Value &data) :
clWidth(0), clHeight(0), normalize(false), integral(0)
{
  clWidth = data["width"].asInt();
  clHeight = data["height"].asUInt();

  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);

  normalize = data["normalize"].asBool();
  rect0 = dataToRect<int>(data["rect0"]);
  rect1 = dataToRect<int>(data["rect1"]);
  preproc = data["preproc"].asString();

  assert(data["bins"].isArray() == true);
  for (Json::UInt i = 0; i < data["bins"].size(); i++)
    bins.push_back(static_cast<float>(data["bins"][i].asDouble()));
}

objed::Haar2PwClassifier::~Haar2PwClassifier()
{
  return;
}

int objed::Haar2PwClassifier::width() const
{
  return clWidth;
}

int objed::Haar2PwClassifier::height() const
{
  return clHeight;
}

bool objed::Haar2PwClassifier::prepare(ImagePool *imagePool)
{
  assert(preproc.length() > 0);
  integral = imagePool->integral(preproc);

  assert(integral != 0);
  return integral != 0;
}

bool objed::Haar2PwClassifier::evaluate(float *result, int x, int y) const
{
  *result = bins[compute(integral, x, y) * bins.size() / 256];
  return true;
}

Json::Value objed::Haar2PwClassifier::serialize() const
{
  Json::Value data;
  Json::Value binsData;
  for (size_t i = 0; i < bins.size(); i++)
    binsData.append(bins[i]);

  data["type"] = type();
  data["width"] = clWidth;
  data["height"] = clHeight;
  data["preproc"] = preproc;
  data["bins"] = binsData;
  data["normalize"] = normalize;
  data["rect0"] = rectToData(rect0);
  data["rect1"] = rectToData(rect1);

  return data;
}

objed::Classifier * objed::Haar2PwClassifier::clone() const
{
  Haar2PwClassifier *cl = new Haar2PwClassifier(width(), height());
  assert(cl != 0);

  cl->preproc = this->preproc;
  cl->normalize = this->normalize;
  cl->rect0 = this->rect0;
  cl->rect1 = this->rect1;
  cl->bins = this->bins;

  return cl;
}

