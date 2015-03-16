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

#include "lazydet.h"

#include <opencv/cv.h>

#include <algorithm>
#include <cstring>
#include <map>

objed::LazyDetector::LazyDetector() : 
imagePool(0), isImagePoolOwn(false), classifier(0),
minScale(1.0), maxScale(1.0), stpScale(1.1), leftMargin(0), 
rightMargin(0), topMargin(0), bottomMargin(0), 
xRawStep(0), yRawStep(0), xStep(0), yStep(0), 
mergeIncluded(true), overlap(0.5)
{
  imagePool = ImagePool::create();
  isImagePoolOwn = true;
}

objed::LazyDetector::LazyDetector(const Json::Value &data, const std::string &workDir) : 
imagePool(0), isImagePoolOwn(false), classifier(0), minScale(1.0), maxScale(1.0), stpScale(1.1), leftMargin(0), rightMargin(0),
topMargin(0), bottomMargin(0), xRawStep(0), yRawStep(0), xStep(0), yStep(0), mergeIncluded(true), overlap(0.5)
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

  xRawStep = round(std::max(1.0, data["xRawStep"].asDouble() * classifier->width()));
  yRawStep = round(std::max(1.0, data["yRawStep"].asDouble() * classifier->height()));

  xStep = round(std::max(1.0, data["xStep"].asDouble() * classifier->width()));
  yStep = round(std::max(1.0, data["yStep"].asDouble() * classifier->height()));

  minScale = data.get("minScale", 1.0).asDouble();
  maxScale = data.get("maxScale", 1.0).asDouble();
  stpScale = data.get("stpScale", 1.1).asDouble();

  mergeIncluded = data.get("mergeIncluded", true).asBool();
  overlap = data.get("overlap", 0.5).asDouble();
}

objed::LazyDetector::~LazyDetector()
{
  if (isImagePoolOwn == true)
    ImagePool::destroy(imagePool);

  Classifier::destroy(classifier);
}

objed::DetectionList objed::LazyDetector::detect(IplImage *image)
{
  if (imagePool == 0 || classifier == 0 || image == 0)
    return DetectionList();

  std::vector<Rect<int> > rawDetectionList;
  const int clWd = classifier->width(), clWd2 = clWd / 2;
  const int clHt = classifier->height(), clHt2 = clHt / 2;
  const int imageWd = image->width, imageHt = image->height;

  for (double scale = minScale; scale <= maxScale; scale *= stpScale)
  {
    int scaledImageWd = round(std::max(1.0, imageWd / scale));
    int scaledImageHt = round(std::max(1.0, imageHt / scale));

    IplImage *scaledImage = cvCreateImage(cvSize(
      scaledImageWd, scaledImageHt), image->depth, image->nChannels);
    cvResize(image, scaledImage);
    imagePool->update(scaledImage);

    for (int y = clHt2; y < scaledImageHt - clHt2; y += yRawStep)
    {
      for (int x = clWd2; x < scaledImageWd - clWd2; x += xRawStep)
      {
        float result = 0.0;
        if (classifier->evaluate(&result, x, y) && result > 0)
        {
          int yMin = std::max(clHt2, y - yRawStep / 2), yMax = std::min(y + yRawStep / 2, scaledImageHt - clHt2);
          int xMin = std::max(clWd2, x - xRawStep / 2), xMax = std::min(x + xRawStep / 2, scaledImageWd - clWd2);

          for (int y0 = yMin; y0 < yMax; y0 += yStep)
          {
            for (int x0 = xMin; x0 < xMax; x0 += xStep)
            {
              if (classifier->evaluate(&result, x0, y0) && result > 0)
              {
                Detection rawDetection;
                rawDetection.power = 1;
                rawDetection.x = round((x0 - clWd2 + leftMargin) * scale);
                rawDetection.y = round((y0 - clHt2 + topMargin) * scale);
                rawDetection.width = round((clWd - leftMargin - rightMargin) * scale);
                rawDetection.height = round((clHt - topMargin - bottomMargin) * scale);
                rawDetectionList.push_back(rawDetection);
              }
            }
          }
        }
      }
    }

    cvReleaseImage(&scaledImage);
  }

  DetectionList clusteredDetectionList = objed::cluster(rawDetectionList, overlap);
  return mergeIncluded == true ? objed::mergeIncluded(clusteredDetectionList) : clusteredDetectionList;
}

objed::Detector * objed::LazyDetector::clone() const
{
  LazyDetector *newLazyDet = new LazyDetector();

  if (isImagePoolOwn == true)
  {
    newLazyDet->imagePool = ImagePool::create();
    newLazyDet->isImagePoolOwn = isImagePoolOwn;
  }
  else
  {
    newLazyDet->imagePool = imagePool;
    newLazyDet->isImagePoolOwn = isImagePoolOwn;
  }

  if (classifier != 0)
  {
    newLazyDet->classifier = classifier->clone();
    newLazyDet->classifier->prepare(newLazyDet->imagePool);
  }

  newLazyDet->minScale = minScale;
  newLazyDet->maxScale = maxScale;
  newLazyDet->stpScale = stpScale;

  newLazyDet->leftMargin = leftMargin;
  newLazyDet->rightMargin = rightMargin;
  newLazyDet->topMargin = topMargin;
  newLazyDet->bottomMargin = bottomMargin;

  newLazyDet->xRawStep = xRawStep;
  newLazyDet->yRawStep = yRawStep;
  newLazyDet->xStep = xStep;
  newLazyDet->yStep = yStep;

  newLazyDet->overlap = overlap;
  newLazyDet->mergeIncluded = mergeIncluded;

  return newLazyDet;
}

void objed::LazyDetector::setImagePool(objed::ImagePool *extImagePool)
{
  if (isImagePoolOwn = true)
    objed::ImagePool::destroy(imagePool);

  isImagePoolOwn = false;
  imagePool = extImagePool;

  if (classifier != 0)
    classifier->prepare(imagePool);
}

void objed::LazyDetector::resetImagePool()
{
  if (isImagePoolOwn = true)
    objed::ImagePool::destroy(imagePool);

  isImagePoolOwn = true;
  imagePool = objed::ImagePool::create();

  if (classifier != 0)
    classifier->prepare(imagePool);
}
