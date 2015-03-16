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

#include <QGraphicsScene>
#include <QGridLayout>

#include <objedutils/objedconf.h>

#include <objed/objedutils.h>
#include <objed/objed.h>

#include "imageviewdock.h"

ImageViewDock::ImageViewDock(QWidget *parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) : 
QDockWidget(QString("Image View"), parent, flags), centralWidget(0), preview(0)
{
  centralWidget = new QWidget(this);
  preview = new QGraphicsView(new QGraphicsScene(this), this);
  
  centralWidget->setLayout(new QGridLayout(this));
  centralWidget->layout()->addWidget(preview);

  this->setFloating(true);
  this->setFeatures(AllDockWidgetFeatures);
  this->setAllowedAreas(Qt::AllDockWidgetAreas);
  this->setWidget(centralWidget);
}

ImageViewDock::~ImageViewDock()
{
  return;
}

void ImageViewDock::setImageView(const QString &id)
{
  this->imageId = id;

  QString title = QString("Image %0").arg(id);
  this->setWindowTitle(title);
}

void ImageViewDock::refreshImageView(objed::ImagePool *imagePool)
{
  preview->scene()->clear();

  const IplImage *image = imagePool->image(imageId.toStdString());
  if (image == 0)
    return;

  QImage::Format format = image->nChannels == 1 ? QImage::Format_Indexed8 : QImage::Format_RGB888;
  QImage qChannelImage((uchar *)image->imageData, image->width, image->height, image->widthStep, format);

  preview->scene()->addPixmap(QPixmap::fromImage(qChannelImage));
  preview->setSceneRect(0, 0, qChannelImage.width(), qChannelImage.height());
  preview->fitInView(preview->sceneRect(), Qt::KeepAspectRatio);
}

void ImageViewDock::resizeEvent(QResizeEvent *event)
{
  preview->fitInView(preview->sceneRect(), Qt::KeepAspectRatio);
  QDockWidget::resizeEvent(event);
}

