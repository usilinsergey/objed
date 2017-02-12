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

#include <opencv/cv.h>

#include <cstring>
#include <cassert>
#include <string>
#include <cmath>

static const int DIR_0    = 1;
static const int DIR_45   = 2;
static const int DIR_90   = 4;
static const int DIR_135  = 8;

static void GetGradientComponents(IplImage *mx, IplImage *my, IplImage *mg, IplImage *gray)
{
  cvSetZero(mx); cvSetImageROI(mx, cvRect(1, 1, gray->width - 2, gray->height - 2));
  cvSetZero(my); cvSetImageROI(my, cvRect(1, 1, gray->width - 2, gray->height - 2));

  cvSetImageROI(gray, cvRect(2, 1, gray->width - 2, gray->height - 2)); cvAdd(mx, gray, mx);
  cvSetImageROI(gray, cvRect(0, 1, gray->width - 2, gray->height - 2)); cvSub(mx, gray, mx);

  cvSetImageROI(gray, cvRect(1, 2, gray->width - 2, gray->height - 2)); cvAdd(my, gray, my);
  cvSetImageROI(gray, cvRect(1, 0, gray->width - 2, gray->height - 2)); cvSub(my, gray, my);

  cvResetImageROI(gray);
  cvResetImageROI(mx);
  cvResetImageROI(my);

  cvCartToPolar(mx, my, mg);
}

static void DiscretizeGradientDirection(IplImage *dstImage, const IplImage *dxImage, const IplImage *dyImage)
{
  static const float pi = static_cast<float>(4.0 * atan(1.0));
  static const float atan_pi_8 = static_cast<float>(atan(pi / 8));

  cvSetZero(dstImage);

  for (int y = 1; y < dstImage->height - 1; y++)
  {
    const float *dxLine = reinterpret_cast<float *>(dxImage->imageData + dxImage->widthStep * y);
    const float *dyLine = reinterpret_cast<float *>(dyImage->imageData + dyImage->widthStep * y);
    uchar *dstLine = reinterpret_cast<uchar *>(dstImage->imageData + dstImage->widthStep * y);

    for (int x = 1; x < dstImage->width - 1; x++)
    {
      uchar &dir = dstLine[x];
      const float &dx = dxLine[x], &dy = dyLine[x], dxdy = dx * dy;
      const float dxa = std::abs(dx), dya = std::abs(dy);

      if (dya < dxa * atan_pi_8)
        dir = DIR_0;
      else if (dxa < dya * atan_pi_8)
        dir = DIR_90;
      else if (dxdy > 0)
        dir = DIR_45;
      else if (dxdy < 0)
        dir = DIR_135;
    }
  }
}

void objed::PrepareGrayImage(IplImage *grayImage, IplImage *image)
{
  assert(grayImage != 0 && image != 0);

  if (grayImage->nChannels == image->nChannels)
    cvCopy(image, grayImage);
  else
    cvCvtColor(image, grayImage, CV_RGB2GRAY);
}

void objed::PrepareSaturationImage(IplImage *saturationImage, IplImage *image)
{
  assert(saturationImage != 0 && image != 0);
  assert(saturationImage->nChannels == 1 && image->nChannels == 3);

  for (int y = 0; y < saturationImage->height; y++)
  {
    uchar *saturLine = reinterpret_cast<uchar *>(saturationImage->imageData + y * saturationImage->widthStep);
    const uchar *imageLine = reinterpret_cast<const uchar *>(image->imageData + y * image->widthStep);

    for (int x = 0, xx = 0; x < saturationImage->width; x++, xx += 3)
    {
      const uchar &r = imageLine[xx + 0], &b = imageLine[xx + 1], &g = imageLine[xx + 2];
      uchar maxValue = std::max(std::max(r, g), b), minValue = std::min(std::min(r, g), b);
      saturLine[x] = maxValue - minValue;
    }
  }
}

void objed::PrepareChannelImage(IplImage *channelImage, IplImage *image, int channel)
{
  assert(channelImage != 0 && image != 0);

  cv::Mat src(image, false), dst(channelImage, false);
  cv::extractChannel(src, dst, channel);
}

void objed::PrepareGradientImage(IplImage *gradientImage, IplImage *grayImage, int direction)
{
  assert(gradientImage != 0 && grayImage != 0);
  assert(gradientImage->nChannels == 1 && grayImage->nChannels == 1);

  IplImage *realGrayImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  IplImage *realDxImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  IplImage *realDyImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  IplImage *realMgImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  cvConvert(grayImage, realGrayImage);

  GetGradientComponents(realDxImage, realDyImage, realMgImage, realGrayImage);
  if (direction == 1 || direction == 2)
    DiscretizeGradientDirection(gradientImage, realDxImage, realDyImage);

  for (int y = 0; y < realMgImage->height; y++)
  {
    static const float sqrt2 = static_cast<float>(sqrt(2.0));
    float *realMgLine = reinterpret_cast<float *>(realMgImage->imageData + y * realMgImage->widthStep);
    for (int x = 0; x < realMgImage->width; x++) realMgLine[x] /= sqrt2;
  }

  for (int y = 0; y < gradientImage->height; y++)
  {
    uchar *gradientLine = reinterpret_cast<uchar *>(gradientImage->imageData + y * gradientImage->widthStep);
    const float *realMgLine = reinterpret_cast<float *>(realMgImage->imageData + y * realMgImage->widthStep);

    for (int x = 0; x < gradientImage->width; x++)
    {
      if (direction == 0)
        gradientLine[x] = static_cast<int>(realMgLine[x]);
      else if (direction == 1 && gradientLine[x] == DIR_0)
        gradientLine[x] = 128 + (static_cast<int>(realMgLine[x]) >> 1);
      else if (direction == 1 && gradientLine[x] == DIR_90)
        gradientLine[x] = 128 - (static_cast<int>(realMgLine[x]) >> 1);
      else if (direction == 2 && gradientLine[x] == DIR_45)
        gradientLine[x] = 128 + (static_cast<int>(realMgLine[x]) >> 1);
      else if (direction == 2 && gradientLine[x] == DIR_135)
        gradientLine[x] = 128 - (static_cast<int>(realMgLine[x]) >> 1);
      else
        gradientLine[x] = 128;
    }
  }

  cvReleaseImage(&realGrayImage);
  cvReleaseImage(&realDxImage);
  cvReleaseImage(&realDyImage);
  cvReleaseImage(&realMgImage);
}

void objed::PrepareCannyImage(IplImage *cannyImage, IplImage *grayImage, int direction)
{
  assert(cannyImage != 0 && grayImage != 0);
  assert(cannyImage->nChannels == 1 && grayImage->nChannels == 1);

  IplImage *realGrayImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  IplImage *realDxImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  IplImage *realDyImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  IplImage *realMgImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  cvConvert(grayImage, realGrayImage);

  GetGradientComponents(realDxImage, realDyImage, realMgImage, realGrayImage);
  DiscretizeGradientDirection(cannyImage, realDxImage, realDyImage);

  for (int y = 1; y < cannyImage->height - 1; y++)
  {
    uchar *cannyLine = reinterpret_cast<uchar *>(cannyImage->imageData + y * cannyImage->widthStep);
    const float *realMgPrevLine = reinterpret_cast<float *>(realMgImage->imageData + (y - 1) * realMgImage->widthStep);
    const float *realMgCurrLine = reinterpret_cast<float *>(realMgImage->imageData + y * realMgImage->widthStep);
    const float *realMgNextLine = reinterpret_cast<float *>(realMgImage->imageData + (y + 1) * realMgImage->widthStep);

    for (int x = 1; x < cannyImage->width - 1; x++)
    {
      const float &mg = realMgCurrLine[x];
      if (cannyLine[x] == DIR_0 && realMgCurrLine[x - 1] < mg && mg >= realMgCurrLine[x + 1])
          cannyLine[x] = (direction == 0 || direction == 1) ? 255 : 128;
      else if (cannyLine[x] == DIR_45 && realMgPrevLine[x - 1] < mg && mg >= realMgNextLine[x + 1])
          cannyLine[x] = (direction == 0 || direction == 2) ? 255 : 128;
      else if (cannyLine[x] == DIR_90 && realMgPrevLine[x] < mg && mg >= realMgNextLine[x])
          cannyLine[x] = direction == 0 ? 255 : (direction == 1 ? 0 : 128);
      else if (cannyLine[x] == DIR_135 && realMgPrevLine[x + 1] < mg && mg >= realMgNextLine[x - 1])
          cannyLine[x] = direction == 0 ? 255 : (direction == 2 ? 0 : 128);
      else
        cannyLine[x] = direction == 0 ? 0 : 128;
    }
  }

  cvReleaseImage(&realGrayImage);
  cvReleaseImage(&realDxImage);
  cvReleaseImage(&realDyImage);
  cvReleaseImage(&realMgImage);
}

void objed::PrepareRawCannyImage(IplImage *rawCannyImage, IplImage *grayImage)
{
  assert(rawCannyImage != 0 && grayImage != 0);
  assert(rawCannyImage->nChannels == 1 && grayImage->nChannels == 1);

  IplImage *realGrayImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  IplImage *realDxImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  IplImage *realDyImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  IplImage *realMgImage = cvCreateImage(cvSize(grayImage->width, grayImage->height), IPL_DEPTH_32F, 1);
  cvConvert(grayImage, realGrayImage);

  GetGradientComponents(realDxImage, realDyImage, realMgImage, realGrayImage);
  DiscretizeGradientDirection(rawCannyImage, realDxImage, realDyImage);

  for (int y = 1; y < rawCannyImage->height - 1; y++)
  {
    uchar *rawCannyLine = reinterpret_cast<uchar *>(rawCannyImage->imageData + y * rawCannyImage->widthStep);
    const float *realMgPrevLine = reinterpret_cast<float *>(realMgImage->imageData + (y - 1) * realMgImage->widthStep);
    const float *realMgCurrLine = reinterpret_cast<float *>(realMgImage->imageData + y * realMgImage->widthStep);
    const float *realMgNextLine = reinterpret_cast<float *>(realMgImage->imageData + (y + 1) * realMgImage->widthStep);

    for (int x = 1; x < rawCannyImage->width - 1; x++)
    {
      const float &mg = realMgCurrLine[x];

      if ((rawCannyLine[x] == DIR_0) && (realMgCurrLine[x - 1] >= mg || mg <= realMgCurrLine[x + 1]))
        rawCannyLine[x] = 0;
      if ((rawCannyLine[x] == DIR_45) && (realMgPrevLine[x - 1] >= mg || mg <= realMgNextLine[x + 1]))
        rawCannyLine[x] = 0;
      if ((rawCannyLine[x] == DIR_90) && (realMgPrevLine[x] >= mg || mg <= realMgNextLine[x]))
        rawCannyLine[x] = 0;
      if ((rawCannyLine[x] == DIR_135) && ((realMgPrevLine[x + 1] >= mg || mg <= realMgNextLine[x - 1])))
        rawCannyLine[x] = 0;
    }
  }

  cvReleaseImage(&realGrayImage);
  cvReleaseImage(&realDxImage);
  cvReleaseImage(&realDyImage);
  cvReleaseImage(&realMgImage);
}

void objed::PrepareFilterImage(IplImage *filterImage, IplImage *grayImage, int index)
{
  assert(filterImage != 0 && grayImage != 0);
  assert(filterImage->nChannels == 1 && grayImage->nChannels == 1);

  for (int y = 0; y < filterImage->height; y++)
  {
    uchar *filterLine = reinterpret_cast<uchar *>(filterImage->imageData + y * filterImage->widthStep);
    const uchar *grayLine = reinterpret_cast<uchar *>(grayImage->imageData + y * grayImage->widthStep);

    for (int x = 0; x < filterImage->width; x++)
      filterLine[x] = 255 * (grayLine[x] == index);
  }
}

void objed::PrepareNoSaltImage(IplImage *noSaltImage, IplImage *grayImage)
{
  assert(noSaltImage != 0 && grayImage != 0);
  assert(noSaltImage->nChannels == 1 && grayImage->nChannels == 1);

  for (int y = 1; y < noSaltImage->height - 1; y++)
  {
    uchar *noSaltLine = reinterpret_cast<uchar *>(noSaltImage->imageData + y * noSaltImage->widthStep);
    const uchar *currGrayLine = reinterpret_cast<uchar *>(grayImage->imageData + y * grayImage->widthStep);
    const uchar *prevGrayLine = reinterpret_cast<uchar *>(grayImage->imageData + (y - 1) * grayImage->widthStep);
    const uchar *nextGrayLine = reinterpret_cast<uchar *>(grayImage->imageData + (y + 1) * grayImage->widthStep);

    for (int x = 1; x < noSaltImage->width - 1; x++)
    {
      short sum = prevGrayLine[x - 1] +  prevGrayLine[x] + prevGrayLine[x + 1] + 
        nextGrayLine[x - 1] +  nextGrayLine[x] + nextGrayLine[x + 1] + 
        currGrayLine[x - 1] + currGrayLine[x + 1];

      noSaltLine[x] = 255 * (currGrayLine[x] == 255 && sum > 0);
    }
  }

}
