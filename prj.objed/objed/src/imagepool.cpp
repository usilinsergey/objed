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

#include "imgutils.h"
#include "imagepool.h"

#include <opencv/cv.h>

#include <cstring>
#include <cassert>

static const std::string METHOD_GRAY         = "gray";
static const std::string METHOD_CHANNEL      = "channel";
static const std::string METHOD_SATURATION   = "saturation";
static const std::string METHOD_GRADIENT     = "gradient";
static const std::string METHOD_CANNY        = "canny";
static const std::string METHOD_RAWCANNY     = "rawCanny";
static const std::string METHOD_FILTER       = "filter";
static const std::string METHOD_NOSALT       = "noSalt";

static void updateImageItem(objed::ImagePoolItem *imageItem, const std::string &method, IplImage *image)
{
  if (imageItem == 0 || image == 0 || image->imageData == 0)
    return;

  if (method.compare(0, METHOD_GRAY.length(), METHOD_GRAY) == 0)
  {
    objed::PrepareGrayImage(imageItem->image(image->width, image->height, 1, IPL_DEPTH_8U), image);
  }
  else if (method.compare(0, METHOD_SATURATION.length(), METHOD_SATURATION) == 0)
  {
    objed::PrepareSaturationImage(imageItem->image(image->width, image->height, 1, IPL_DEPTH_8U), image);
  }
  else if (method.compare(0, METHOD_CHANNEL.length(), METHOD_CHANNEL) == 0)
  {
    int channel = atoi(method.substr(METHOD_CHANNEL.length()).c_str());
    objed::PrepareChannelImage(imageItem->image(image->width, image->height, 1, IPL_DEPTH_8U), image, channel);
  }
  else if (method.compare(0, METHOD_GRADIENT.length(), METHOD_GRADIENT) == 0)
  {
    int direction = atoi(method.substr(METHOD_GRADIENT.length()).c_str());
    objed::PrepareGradientImage(imageItem->image(image->width, image->height, 1, IPL_DEPTH_8U), image, direction);
  }
  else if (method.compare(0, METHOD_CANNY.length(), METHOD_CANNY) == 0)
  {
    int direction = atoi(method.substr(METHOD_CANNY.length()).c_str());
    objed::PrepareCannyImage(imageItem->image(image->width, image->height, 1, IPL_DEPTH_8U), image, direction);
  }
  else if (method.compare(0, METHOD_RAWCANNY.length(), METHOD_RAWCANNY) == 0)
  {
    objed::PrepareRawCannyImage(imageItem->image(image->width, image->height, 1, IPL_DEPTH_8U), image);
  }
  else if (method.compare(0, METHOD_FILTER.length(), METHOD_FILTER) == 0)
  {
    int index = atoi(method.substr(METHOD_FILTER.length()).c_str());
    objed::PrepareFilterImage(imageItem->image(image->width, image->height, 1, IPL_DEPTH_8U), image, index);
  }
  else if (method.compare(0, METHOD_NOSALT.length(), METHOD_NOSALT) == 0)
  {
    objed::PrepareNoSaltImage(imageItem->image(image->width, image->height, 1, IPL_DEPTH_8U), image);
  }
  else
  {
    assert(false);
  }
}

static void updateIntegralItem(objed::ImagePoolItem *integralItem, IplImage *image)
{
  if (integralItem == 0 || image == 0 || image->imageData == 0)
    return;

  IplImage *integral = integralItem->image(image->width + 1, image->height + 1, image->nChannels, IPL_DEPTH_32S);
  cvIntegralImage(image, integral);
}

objed::ImagePoolItem::ImagePoolItem()
{
  sourceImage = cvCreateImageHeader(cvSize(1, 1), IPL_DEPTH_8U, 1);
  regionImage = cvCreateImageHeader(cvSize(1, 1), IPL_DEPTH_8U, 1);;
}

objed::ImagePoolItem::~ImagePoolItem()
{
  if (sourceImage->imageData != 0)
    cvReleaseImageData(sourceImage);
  cvReleaseImageHeader(&sourceImage);
  cvReleaseImageHeader(&regionImage);
}

IplImage * objed::ImagePoolItem::image(int width, int height, int channels, int depth)
{
  if (sourceImage == 0 || sourceImage->width < width || sourceImage->height < height ||
    sourceImage->depth != depth || sourceImage->nChannels != channels)
  {
    if (sourceImage->imageData != 0)
      cvReleaseImageData(sourceImage);
    IplImage *newSourceImage = cvCreateImage(cvSize(width, height), depth, channels);
    memcpy(sourceImage, newSourceImage, sourceImage->nSize);
    cvReleaseImageHeader(&newSourceImage);
  }

  regionImage->width = width;
  regionImage->height = height;
  regionImage->depth = sourceImage->depth;
  regionImage->nChannels = sourceImage->nChannels;
  regionImage->widthStep = sourceImage->widthStep;
  regionImage->imageData = sourceImage->imageData;
  regionImage->origin = sourceImage->origin;

  return regionImage;
}

IplImage * objed::ImagePoolItem::image() const
{
  return regionImage;
}

objed::ImagePoolImpl::ImagePoolImpl()
{
  baseItem = new ImagePoolItem();
}

objed::ImagePoolImpl::~ImagePoolImpl()
{
  delete baseItem;

  std::map<std::string, ImagePoolItem *>::iterator itImageItem;
  for (itImageItem = imageItems.begin(); itImageItem != imageItems.end(); ++itImageItem)
    delete itImageItem->second;
  imageItems.clear();

  std::map<std::string, ImagePoolItem *>::iterator itIntegralItem;
  for (itIntegralItem = integralItems.begin(); itIntegralItem != integralItems.end(); ++itIntegralItem)
    delete itIntegralItem->second;
  integralItems.clear();
}

bool objed::ImagePoolImpl::update(IplImage *image)
{
  cvCopyImage(image, baseItem->image(image->width, image->height, image->nChannels, image->depth));

  std::map<std::string, ImagePoolItem *>::iterator itImageItem;
  for (itImageItem = imageItems.begin(); itImageItem != imageItems.end(); ++itImageItem)
  {
    const std::string &id = itImageItem->first;
    ImagePoolItem *imageItem = itImageItem->second;

    size_t sepPos = id.rfind("|");
    std::string method = sepPos >= std::string::npos ? id : id.substr(sepPos + 1);
    std::string subid = sepPos >= std::string::npos ? std::string() : id.substr(0, sepPos);

    if (method.empty() || subid.empty())
      updateImageItem(imageItem, method, image);
    else
      updateImageItem(imageItem, method, imageItems[subid]->image());
  }

  std::map<std::string, ImagePoolItem *>::iterator itIntegralItem;
  for (itIntegralItem = integralItems.begin(); itIntegralItem != integralItems.end(); ++itIntegralItem)
  {
    const std::string &id = itIntegralItem->first;
    ImagePoolItem *integralItem = itIntegralItem->second;

    updateIntegralItem(integralItem, imageItems[id]->image());
  }

  return true;
}

IplImage *objed::ImagePoolImpl::integral(const std::string &id)
{
  ImagePoolItem *&integralItem = integralItems[id];
  if (integralItem == 0)
  {
    integralItem = new ImagePoolItem();
    updateIntegralItem(integralItem, image(id));
  }

  return integralItem->image();
}

IplImage *objed::ImagePoolImpl::image(const std::string &id)
{
  ImagePoolItem *&imageItem = imageItems[id];
  if (imageItem == 0)
  {
    size_t sepPos = id.rfind("|");
    std::string method = sepPos >= std::string::npos ? id : id.substr(sepPos + 1);
    std::string subid = sepPos >= std::string::npos ? std::string() : id.substr(0, sepPos);

    imageItem = new ImagePoolItem();
    if (method.empty() || subid.empty())
      updateImageItem(imageItem, method, baseItem->image());
    else
      updateImageItem(imageItem, method, image(subid));
  }

  return imageItem->image();
}

IplImage *objed::ImagePoolImpl::base()
{
  return baseItem->image();
}

std::vector<std::string> objed::ImagePoolImpl::imageNames() const
{
  std::vector<std::string> names;

  std::map<std::string, ImagePoolItem *>::const_iterator it;
  for (it = imageItems.begin(); it != imageItems.end(); ++it)
    names.push_back(it->first);

  return names;
}

std::vector<std::string> objed::ImagePoolImpl::integralNames() const
{
  std::vector<std::string> names;

  std::map<std::string, ImagePoolItem *>::const_iterator it;
  for (it = integralItems.begin(); it != integralItems.end(); ++it)
    names.push_back(it->first);

  return names;
}
