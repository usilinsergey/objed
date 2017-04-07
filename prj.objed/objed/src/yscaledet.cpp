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

#include "yscaledet.h"

#include <opencv/cv.h>

#include <cstring>
#include <algorithm>
#include <map>
#include <limits>


objed::YScaleDetector::YScaleDetector() : imagePool(0), isImagePoolOwn(false), 
classifier(0), y0(0.0), y1(0.0), yStp(1.0), y0Scale(1.0), y1Scale(1.0), leftMargin(0), 
rightMargin(0), topMargin(0), bottomMargin(0), xStep(0), yStep(0), 
mergeIncluded(false), overlap(0.5)
{
  imagePool = ImagePool::create();
  isImagePoolOwn = true;
}

objed::YScaleDetector::YScaleDetector(const Json::Value &data, const std::string &workDir) : 
imagePool(0), isImagePoolOwn(false), classifier(0),  y0(0.0), y1(0.0), yStp(1.0), y0Scale(1.0), y1Scale(1.0), 
leftMargin(0), rightMargin(0), topMargin(0), bottomMargin(0), xStep(0), yStep(0), 
mergeIncluded(false), overlap(0.5)
{
  imagePool = ImagePool::create();
  isImagePoolOwn = true;

  classifier = Classifier::create(data["classifier"], workDir);
  if (classifier == 0)
    return;

  classifier->prepare(imagePool);

  leftMargin = round(data["leftMargin"].asDouble() * classifier->width());
  rightMargin = round(data["rightMargin"].asDouble() * classifier->width());
  topMargin = round(data["topMargin"].asDouble() * classifier->height());
  bottomMargin = round(data["bottomMargin"].asDouble() * classifier->height());

  y0 = std::max(0.0, data["y0"].asDouble());
  y1 = std::max(y0, data["y1"].asDouble());
  yStp = std::max(1.0, data["yStp"].asDouble());

  y0Scale = std::max(0.0, data["y0Scale"].asDouble());
  y1Scale = std::max(0.0, data["y1Scale"].asDouble());

  xStep = round(std::max(1.0, data["xStep"].asDouble() * classifier->width()));
  yStep = round(std::max(1.0, data["yStep"].asDouble() * classifier->height()));

  mergeIncluded = data.get("mergeIncluded", true).asBool();
  overlap = data.get("overlap", 0.5).asDouble();
}

objed::YScaleDetector::~YScaleDetector()
{
  if (isImagePoolOwn == true)
    objed::ImagePool::destroy(imagePool);

  objed::Classifier::destroy(classifier);
}

objed::DetectionList objed::YScaleDetector::detect(IplImage *image, DebugInfo *debugInfo)
{
  if (imagePool == 0 || classifier == 0 || image == 0)
    return DetectionList();

  DebugInfo *clDebugInfo = 0;
  if (debugInfo != nullptr)
    clDebugInfo = new DebugInfo();

  // scale = k * y + b
  assert(std::abs(y1 - y0) > std::numeric_limits<double>::epsilon());
  double b = (y1 * y0Scale - y0 * y1Scale) / (y1 - y0);
  double k = (y1Scale - y0Scale) / (y1 - y0);

  std::vector<Rect<int> > rawDetectionList;
  for (double y = y0; y < y1; y *= yStp)
  {
    int yInt = round(image->height * y);
    assert(yInt >= 0 && yInt < image->height);

    double scale = k * y + b;
    assert(scale >= std::min(y0Scale, y1Scale));
    assert(scale <= std::max(y0Scale, y1Scale));

    const int clWd = classifier->width(), clWd2 = clWd / 2;
    const int clHt = classifier->height(), clHt2 = clHt / 2;

    int prevYInt = std::max(0, round(yInt / yStp - scale * clHt));
    int nextYInt = std::min(image->height - 1, round(yInt * yStp + scale * clHt));

    IplImage *region = cvCreateImageHeader(
      cvSize(image->width, nextYInt - prevYInt + 1), image->depth, image->nChannels);
    region->imageData = image->imageData + prevYInt * image->widthStep;
    region->widthStep = image->widthStep;
    region->origin = image->origin;

    int scaledRegionWidth = round(region->width / scale);
    int scaledRegionHeight = round(region->height / scale);
    if (scaledRegionWidth < classifier->width() || scaledRegionHeight < classifier->height())
    {
      cvReleaseImageHeader(&region);
      continue;
    }
    
    IplImage *scaledRegion = cvCreateImage(cvSize(
      scaledRegionWidth, scaledRegionHeight), region->depth, region->nChannels);
    cvResize(region, scaledRegion);
    imagePool->update(scaledRegion);

    for (int y = clHt2; y < scaledRegionHeight - clHt2; y += yStep)
    {
      for (int x = clWd2; x < scaledRegionWidth - clWd2; x += xStep)
      {
        if (clDebugInfo != nullptr)
          clDebugInfo->int_data.clear();

        float result = 0.0;
        if (classifier->evaluate(&result, x, y, clDebugInfo) && result > 0)
        {
          Detection rawDetection;
          rawDetection.power = 1;
          rawDetection.x = round((x - clWd2 + leftMargin) * scale);
          rawDetection.y = round((y - clHt2 + topMargin) * scale + prevYInt);
          rawDetection.width = round((clWd - leftMargin - rightMargin) * scale);
          rawDetection.height = round((clHt - topMargin - bottomMargin) * scale);
          rawDetectionList.push_back(rawDetection);
        }

        if (debugInfo != nullptr)
        {
          int outputLevel = clDebugInfo->int_data[Classifier::DBG_SC_COUNT];
          if (debugInfo->int_data.find(Detector::DBG_MIN_SC_COUNT) != debugInfo->int_data.end())
            debugInfo->int_data[Detector::DBG_MIN_SC_COUNT] = std::min(outputLevel, debugInfo->int_data[Detector::DBG_MIN_SC_COUNT]);
          else
            debugInfo->int_data[Detector::DBG_MIN_SC_COUNT] = outputLevel;
          if (debugInfo->int_data.find(Detector::DBG_MAX_SC_COUNT) != debugInfo->int_data.end())
            debugInfo->int_data[Detector::DBG_MAX_SC_COUNT] = std::max(outputLevel, debugInfo->int_data[Detector::DBG_MAX_SC_COUNT]);
          else
            debugInfo->int_data[Detector::DBG_MAX_SC_COUNT] = outputLevel;
          debugInfo->int_data[Detector::DBG_TOTAL_SC_COUNT] += outputLevel;
          debugInfo->int_data[Detector::DBG_EVALUATION_COUNT]++;
        }
      }
    }

    cvReleaseImage(&scaledRegion);
    cvReleaseImageHeader(&region);
  }

  if (clDebugInfo != nullptr)
    delete clDebugInfo;

  DetectionList clusteredDetectionList = objed::cluster(rawDetectionList, overlap);
  return mergeIncluded == true ? objed::mergeIncluded(clusteredDetectionList) : clusteredDetectionList;
}

objed::Detector * objed::YScaleDetector::clone() const
{
  YScaleDetector *newYScaleDet = new YScaleDetector();

  if (isImagePoolOwn == true)
  {
    newYScaleDet->imagePool = ImagePool::create();
    newYScaleDet->isImagePoolOwn = isImagePoolOwn;
  }
  else
  {
    newYScaleDet->imagePool = imagePool;
    newYScaleDet->isImagePoolOwn = isImagePoolOwn;
  }

  if (classifier != 0)
  {
    newYScaleDet->classifier = classifier->clone();
    newYScaleDet->classifier->prepare(newYScaleDet->imagePool);
  }

  newYScaleDet->y0 = y0;
  newYScaleDet->y1 = y1;
  newYScaleDet->yStp = yStp;

  newYScaleDet->y0Scale = y0Scale;
  newYScaleDet->y1Scale = y1Scale;

  newYScaleDet->leftMargin = leftMargin;
  newYScaleDet->rightMargin = rightMargin;
  newYScaleDet->topMargin = topMargin;
  newYScaleDet->bottomMargin = bottomMargin;

  newYScaleDet->xStep = xStep;
  newYScaleDet->yStep = yStep;

  newYScaleDet->overlap = overlap;
  newYScaleDet->mergeIncluded = mergeIncluded;

  return newYScaleDet;
}

void objed::YScaleDetector::setImagePool(objed::ImagePool *extImagePool)
{
  if (isImagePoolOwn = true)
    objed::ImagePool::destroy(imagePool);

  isImagePoolOwn = false;
  imagePool = extImagePool;

  if (classifier != 0)
    classifier->prepare(imagePool);
}

void objed::YScaleDetector::resetImagePool()
{
  if (isImagePoolOwn = true)
    objed::ImagePool::destroy(imagePool);

  isImagePoolOwn = true;
  imagePool = objed::ImagePool::create();

  if (classifier != 0)
    classifier->prepare(imagePool);
}
