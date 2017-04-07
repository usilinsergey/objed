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

#include "roicl.h"

#include <cassert>

template<class T>
inline static bool rectContains(const objed::Rect<T> &rect, const objed::Point<T> &point)
{
  return point.x >= rect.x && point.y >= rect.y &&
    point.x <= rect.x + rect.width && point.y <= rect.y + rect.height;
}

template<class T>
inline static bool rectContains(const objed::Rect<T> &largeRect, const objed::Rect<T> &smallRect)
{
  return smallRect.x >= largeRect.x && smallRect.y >= largeRect.y &&
    smallRect.x + smallRect.width <= largeRect.x + largeRect.width &&
    smallRect.y + smallRect.height <= largeRect.y + largeRect.height;
}

objed::RoiClassifier::RoiClassifier(int width, int height) :
clWidth(width), clHeight(height), baseImage(0),
objectRoi(0.0, 0.0, 1.0, 1.0), leftBorderRoi(0.0, 0.0, 1.0, 1.0),
topBorderRoi(0.0, 0.0, 1.0, 1.0), rightBorderRoi(0.0, 0.0, 1.0, 1.0),
bottomBorderRoi(0.0, 0.0, 1.0, 1.0)
{
  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);
}

objed::RoiClassifier::RoiClassifier(const Json::Value &data) :
clWidth(0), clHeight(0), baseImage(0),
objectRoi(0.0, 0.0, 1.0, 1.0), leftBorderRoi(0.0, 0.0, 1.0, 1.0),
topBorderRoi(0.0, 0.0, 1.0, 1.0), rightBorderRoi(0.0, 0.0, 1.0, 1.0),
bottomBorderRoi(0.0, 0.0, 1.0, 1.0)
{
  clWidth = data["width"].asInt();
  clHeight = data["height"].asInt();

  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);

  if (data.isMember("objectRoi") == true)
    objectRoi = dataToRect<double>(data["objectRoi"]);
  if (data.isMember("leftBorderRoi") == true)
    leftBorderRoi = dataToRect<double>(data["leftBorderRoi"]);
  if (data.isMember("topBorderRoi") == true)
    topBorderRoi = dataToRect<double>(data["topBorderRoi"]);
  if (data.isMember("rightBorderRoi") == true)
    rightBorderRoi = dataToRect<double>(data["rightBorderRoi"]);
  if (data.isMember("bottomBorderRoi") == true)
    bottomBorderRoi = dataToRect<double>(data["bottomBorderRoi"]);
}

objed::RoiClassifier::~RoiClassifier()
{
  return;
}

int objed::RoiClassifier::width() const
{
  return clWidth;
}

int objed::RoiClassifier::height() const
{
  return clHeight;
}

bool objed::RoiClassifier::prepare(ImagePool *imagePool)
{
  assert(imagePool != 0);

  baseImage = imagePool->base();

  return baseImage != 0;
}

bool objed::RoiClassifier::evaluate(float *result, int x, int y, DebugInfo *debugInfo) const
{
  assert(result != 0);
  assert(baseImage != 0);

  Rect<double> objectRect(
    (x - clWidth / 2 + 0.0) / (baseImage->width + 0.0),
    (y - clHeight / 2 + 0.0) / (baseImage->height + 0.0), 
    (clWidth + 0.0) / (baseImage->width + 0.0),
    (clHeight + 0.0) / (baseImage->height + 0.0));

  Point<double> leftBorder(objectRect.x, objectRect.y + objectRect.height / 2);
  Point<double> topBorder(objectRect.x + objectRect.width / 2, objectRect.y);
  Point<double> rightBorder(objectRect.x + objectRect.width, objectRect.y + objectRect.height / 2);
  Point<double> bottomBorder(objectRect.x + objectRect.width / 2, objectRect.y + objectRect.height);

  if (rectContains(objectRoi, objectRect) == false)
    *result = -1.0;
  else if (rectContains(leftBorderRoi, leftBorder) == false)
    *result = -1.0;
  else if (rectContains(topBorderRoi, topBorder) == false)
    *result = -1.0;
  else if (rectContains(rightBorderRoi, rightBorder) == false)
    *result = -1.0;
  else if (rectContains(bottomBorderRoi, bottomBorder) == false)
    *result = -1.0;
  else
    *result = 1.0;

  return true;
}

Json::Value objed::RoiClassifier::serialize() const
{
  Json::Value data;

  data["type"] = type();
  data["width"] = clWidth;
  data["height"] = clHeight;
  data["objectRoi"] = rectToData(objectRoi);
  data["leftBorderRoi"] = rectToData(leftBorderRoi);
  data["topBorderRoi"] = rectToData(topBorderRoi);
  data["rightBorderRoi"] = rectToData(rightBorderRoi);
  data["bottomBorderRoi"] = rectToData(bottomBorderRoi);

  return data;
}

objed::Classifier * objed::RoiClassifier::clone() const
{
  RoiClassifier *cl = new RoiClassifier(width(), height());
  assert(cl != 0);
  
  cl->objectRoi = this->objectRoi;
  cl->leftBorderRoi = this->leftBorderRoi;
  cl->topBorderRoi = this->topBorderRoi;
  cl->rightBorderRoi = this->rightBorderRoi;
  cl->bottomBorderRoi = this->bottomBorderRoi;

  return cl;
}
