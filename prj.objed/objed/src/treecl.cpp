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

#include "treecl.h"

#include <cassert>

objed::TreeClassifier::TreeClassifier(int width, int height) :
clWidth(width), clHeight(height), centralCl(0), leftCl(0), rightCl(0)
{
  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);
}

objed::TreeClassifier::TreeClassifier(const Json::Value &data) :
clWidth(0), clHeight(0), centralCl(0), leftCl(0), rightCl(0)
{
  clWidth = data["width"].asInt();
  clHeight = data["height"].asInt();

  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);

  centralCl = Classifier::create(data["centralCl"]);
  assert(centralCl != 0);

  if (data["leftCl"].isNull() == false)
    leftCl = new TreeClassifier(data["leftCl"]);
  if (data["rightCl"].isNull() == false)
    rightCl = new TreeClassifier(data["rightCl"]);
}

objed::TreeClassifier::~TreeClassifier()
{
  Classifier::destroy(centralCl);
  Classifier::destroy(rightCl);
  Classifier::destroy(leftCl);
}

int objed::TreeClassifier::width() const
{
  return clWidth;
}

int objed::TreeClassifier::height() const
{
  return clHeight;
}

bool objed::TreeClassifier::prepare(ImagePool *imagePool)
{
  assert(centralCl != 0);
  bool ok = centralCl->prepare(imagePool);
  if (leftCl != 0) ok &= leftCl->prepare(imagePool);
  if (rightCl != 0) ok &= rightCl->prepare(imagePool);

  return ok;
}

bool objed::TreeClassifier::evaluate(float *result, int x, int y) const
{
  if (leftCl != 0 && rightCl != 0)
    return evaluateBoth(result, x, y);
  else if (leftCl == 0 && rightCl != 0)
    return evaluateRight(result, x, y);
  else if (leftCl != 0 && rightCl == 0)
    return evaluateLeft(result, x, y);
  else if (leftCl == 0 && rightCl == 0)
    return evaluateNone(result, x, y);
  else
    return false;
}

Json::Value objed::TreeClassifier::serialize() const
{
  Json::Value data;

  data["type"] = type();
  data["width"] = clWidth;
  data["height"] = clHeight;

  assert(centralCl != 0);
  data["centralCl"] = centralCl->serialize();
  data["leftCl"] = leftCl != 0 ? leftCl->serialize() : Json::Value();
  data["rightCl"] = rightCl != 0 ? rightCl->serialize() : Json::Value();

  return data;
}

objed::Classifier * objed::TreeClassifier::clone() const
{
  TreeClassifier *newTreeCl = new TreeClassifier(width(), height());
  assert(newTreeCl != 0);

  assert(centralCl != 0);
  newTreeCl->centralCl = centralCl->clone();

  if (leftCl != 0) newTreeCl->leftCl = dynamic_cast<TreeClassifier *>(leftCl->clone());
  if (rightCl != 0) newTreeCl->rightCl = dynamic_cast<TreeClassifier *>(rightCl->clone());

  return newTreeCl;
}

bool objed::TreeClassifier::evaluateBoth(float *result, int x, int y) const
{
  assert(centralCl != 0 && leftCl != 0 && rightCl != 0);

  bool ok = centralCl->evaluate(result, x, y);
  return ok && (*result > 0 ? rightCl->evaluate(result, x, y) : leftCl->evaluate(result, x, y));
}

bool objed::TreeClassifier::evaluateLeft(float *result, int x, int y) const
{
  assert(centralCl != 0 && leftCl != 0 && rightCl == 0);

  bool ok = centralCl->evaluate(result, x, y);
  return *result > 0 ? ok : (ok && leftCl->evaluate(result, x, y));
}

bool objed::TreeClassifier::evaluateRight(float *result, int x, int y) const
{
  assert(centralCl != 0 && leftCl == 0 && rightCl != 0);

  bool ok = centralCl->evaluate(result, x, y);
  return *result > 0 ? (ok && rightCl->evaluate(result, x, y)) : ok;
}

bool objed::TreeClassifier::evaluateNone(float *result, int x, int y) const
{
  assert(centralCl != 0 && leftCl == 0 && rightCl == 0);

  return centralCl->evaluate(result, x, y);
}
