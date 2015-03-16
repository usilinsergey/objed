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

#include <objedutils/objedimage.h>
#include <objedutils/objedexp.h>

#include <opencv/highgui.h>
#include <opencv/cv.h>

class ObjedImageImpl : public ObjedImage
{
public:
  ObjedImageImpl();
  ObjedImageImpl(const QString &imagePath);
  ObjedImageImpl(ObjedImageImpl *source, const QRect &rect);
  virtual ~ObjedImageImpl();

public:
public:
  virtual int width() const;
  virtual int height() const;
  virtual bool isEmpty() const;

public:
  virtual void resize(const QSize &size);
  virtual void gammaCorrection(double gamma);

public:
  virtual void save(const QString &imagePath) const;
  virtual QPixmap toQPixmap() const;

public:
  IplImage * image() const;

private:
  IplImage *image_;

};


ObjedImageImpl::ObjedImageImpl() : image_(0)
{
  return;
}

ObjedImageImpl::ObjedImageImpl(const QString &imagePath) : image_(0)
{
#pragma omp critical(objedimage)
  {
    if (image_ != 0) cvReleaseImage(&image_);

    image_ = cvLoadImage(qPrintable(imagePath), CV_LOAD_IMAGE_UNCHANGED);

    if (image_ == 0 || image_->depth != IPL_DEPTH_8U || (image_->nChannels != 1 && image_->nChannels != 3))
      throw ObjedException("Unsupported image format");
  }
}

ObjedImageImpl::ObjedImageImpl(ObjedImageImpl *source, const QRect &rect) : image_(0)
{
  if (rect.x() < 0 || rect.x() + rect.width() > source->width())
    return;
  if (rect.y() < 0 || rect.y() + rect.height() > source->height())
    return;

  image_ = cvCreateImage(cvSize(rect.width(), rect.height()), source->image_->depth, source->image_->nChannels);
  cvSetImageROI(source->image_, cvRect(rect.x(), rect.y(), rect.width(), rect.height()));
  cvCopy(source->image_, image_);
  cvResetImageROI(source->image_);
}

ObjedImageImpl::~ObjedImageImpl()
{
  cvReleaseImage(&image_);
}

int ObjedImageImpl::width() const
{
  return image_->width;
}

int ObjedImageImpl::height() const
{
  return image_->height;
}

bool ObjedImageImpl::isEmpty() const
{
  return image_ == 0;
}

void ObjedImageImpl::resize(const QSize &size)
{
  IplImage *newImage = cvCreateImage(cvSize(size.width(), size.height()), image_->depth, image_->nChannels);
  cvResize(image_, newImage, CV_INTER_LINEAR);
  cvReleaseImage(&image_);
  image_ = newImage;
}

void ObjedImageImpl::gammaCorrection(double gamma)
{
  if (gamma <= 0.0)
    return;

  unsigned char lut[256] = {0};
  for (int px = 0; px < 256; px++)
  {
    double val = pow(px / 256.0, gamma) * 256.0;
    lut[px] = std::min(std::max(0, static_cast<int>(val + 0.5)), 255);
  }

  for (int y = 0; y < image_->height; y++)
  {
    char *line = image_->imageData + y * image_->widthStep;
    for (int x = 0; x < image_->width * image_->nChannels; x++)
      line[x] = lut[static_cast<unsigned char>(line[x])];
  }
}


void ObjedImageImpl::save(const QString &imagePath) const
{
#pragma omp critical(objedimage)
  {
    if (cvSaveImage(qPrintable(imagePath), image_) < 0)
      throw ObjedException(QString("Cannot save image to '%0'").arg(imagePath));
  }
}

QPixmap ObjedImageImpl::toQPixmap() const
{
  QImage::Format format = image_->nChannels == 1 ? QImage::Format_Indexed8 : QImage::Format_RGB888;
  QImage qImage((uchar *)image_->imageData, image_->width, image_->height, image_->widthStep, format);

  return QPixmap::fromImage(qImage);
}

IplImage * ObjedImageImpl::image() const
{
  return image_;
}

QSharedPointer<ObjedImage> ObjedImage::create(const QString &imagePath)
{
  return QSharedPointer<ObjedImage>(new ObjedImageImpl(imagePath));
}

QSharedPointer<ObjedImage> ObjedImage::create(ObjedImage *source, const QRect &rect)
{
  ObjedImageImpl *sourceImpl = dynamic_cast<ObjedImageImpl *>(source);
  return sourceImpl != 0 ? QSharedPointer<ObjedImage>(new ObjedImageImpl(sourceImpl, rect)) : QSharedPointer<ObjedImage>();
}
