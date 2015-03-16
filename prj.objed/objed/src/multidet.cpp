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

#include "multidet.h"

objed::MultiDetector::MultiDetector() : 
imagePool(0), isImagePoolOwn(false), mergeIncluded(false), overlap(0.5)
{
  return;
}

objed::MultiDetector::MultiDetector(const Json::Value &data, const std::string &workDir) : 
imagePool(0), isImagePoolOwn(false), mergeIncluded(false), overlap(0.5)
{
  imagePool = ImagePool::create();
  isImagePoolOwn = true;

  Json::Value detectorListData = data["detectorList"];
  if (detectorListData.isArray() && detectorListData.size() > 0)
  {
    for (Json::UInt i = 0; i < detectorListData.size(); i++)
    {
      Detector *detector = 0;
      if (detectorListData[i].isString() == true)
        detector = objed::Detector::create(detectorListData[i].asString(), workDir);
      else
        detector = objed::Detector::create(detectorListData[i], workDir);

      if (detector != 0)
      {
        detector->setImagePool(imagePool);
        detectorList.push_back(detector);
      }
    }
  }

  mergeIncluded = data.get("mergeIncluded", true).asBool();
  overlap = data.get("overlap", 0.5).asDouble();
}

objed::MultiDetector::~MultiDetector()
{
  if (isImagePoolOwn == true)
    objed::ImagePool::destroy(imagePool);

  for (size_t i = 0; i < detectorList.size(); i++)
    objed::Detector::destroy(detectorList[i]);
}

objed::DetectionList objed::MultiDetector::detect(IplImage *image)
{
  DetectionList rawDetectionList;

  for (size_t i = 0; i < detectorList.size(); i++)
  {
    DetectionList d = detectorList[i]->detect(image);
    for (size_t j = 0; j < d.size(); j++)
      rawDetectionList.push_back(d[j]);
  }

  DetectionList clusteredDetectionList = objed::cluster(rawDetectionList, overlap);
  return mergeIncluded == true ? objed::mergeIncluded(clusteredDetectionList) : clusteredDetectionList;
}

objed::Detector * objed::MultiDetector::clone() const
{
  MultiDetector *newMultiDet = new MultiDetector();

  if (isImagePoolOwn == true)
  {
    newMultiDet->imagePool = ImagePool::create();
    newMultiDet->isImagePoolOwn = isImagePoolOwn;
  }
  else
  {
    newMultiDet->imagePool = imagePool;
    newMultiDet->isImagePoolOwn = isImagePoolOwn;
  }

  for (size_t i = 0; i < detectorList.size(); i++)
  {
    Detector *newDetector = detectorList[i]->clone();
    newDetector->setImagePool(imagePool);
    newMultiDet->detectorList.push_back(newDetector);
  }

  return newMultiDet;
}

void objed::MultiDetector::setImagePool(objed::ImagePool *extImagePool)
{
  if (isImagePoolOwn = true)
    objed::ImagePool::destroy(imagePool);

  isImagePoolOwn = false;
  imagePool = extImagePool;

  for (size_t i = 0; i < detectorList.size(); i++)
    detectorList[i]->setImagePool(imagePool);
}

void objed::MultiDetector::resetImagePool()
{
  if (isImagePoolOwn = true)
    objed::ImagePool::destroy(imagePool);

  isImagePoolOwn = true;
  imagePool = objed::ImagePool::create();

  for (size_t i = 0; i < detectorList.size(); i++)
    detectorList[i]->setImagePool(imagePool);
}
