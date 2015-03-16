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

#include <objedutils/objedconsole.h>
#include <objedutils/objedexp.h>
#include <objedutils/objedio.h>

#include "objedcututils.h"

static bool sizeLessThan(const QSize &s1, const QSize &s2)
{
  return s1.width() * s1.height() < s2.width() * s2.height();
}

ObjedCutCutting::ObjedCutCutting(ObjedConfig *config) : 
objectCount(0), negativeCount(0)
{
  minObjectWidth = config->value("MinObjectWidth").toInt();
  maxObjectWidth = config->value("MaxObjectWidth").toInt();
  if (minObjectWidth > maxObjectWidth)
    throw ObjedException("Invalid object range");

  minObjectHeight = config->value("MinObjectHeight").toInt();
  maxObjectHeight = config->value("MaxObjectHeight").toInt();
  if (minObjectHeight > maxObjectHeight)
    throw ObjedException("Invalid object range");

  leftMargin = config->value("LeftMargin").toReal();
  rightMargin = config->value("RightMargin").toReal();
  topMargin = config->value("TopMargin").toReal();
  bottomMargin = config->value("BottomMargin").toReal();
  if (leftMargin < 0.0 || rightMargin < 0.0 || topMargin < 0.0 || bottomMargin < 0.0)
    throw ObjedException("Invalid object margins (negative values)");

  positiveDataset.setPath(config->value("PositiveDataset").toString());
  negativeDataset.setPath(config->value("NegativeDataset").toString());

  outputFormat = config->value("OutputFormat").toString().toLower();
}

ObjedCutCutting::~ObjedCutCutting()
{
  return;
}

void ObjedCutCutting::process(ObjedImage *image, const ObjedMarkup *markup, const QString &imageName)
{
  Q_ASSERT(image != 0 && markup != 0);

  if (markup->isStarred() == false && markup->objects().isEmpty() == true)
    return;

  QString imageSuffix = QFileInfo(imageName).suffix();
  if (outputFormat.isEmpty() == false) imageSuffix = outputFormat;

  if (positiveDataset.path().isEmpty() == false && positiveDataset.exists() == true)
  {
    for (int i = 0; i < markup->objects().count(); i++)
    {
      const QRect &object = markup->objects().value(i);
      int lm = qRound(leftMargin * object.width()), rm = qRound(rightMargin * object.width());
      int tm = qRound(topMargin * object.height()), bm = qRound(bottomMargin * object.height());

      if (object.width() < minObjectWidth || object.width() > maxObjectWidth)
        continue;
      if (object.height() < minObjectHeight || object.height() > maxObjectHeight)
        continue;

      QRect expendedObject = object;
      expendedObject.translate(-lm, -tm);
      expendedObject.setWidth(object.width() + lm + rm);
      expendedObject.setHeight(object.height() + tm + bm);

      QString objectName = QString("%0_%1.%2").arg(imageName).arg(i).arg(imageSuffix);
      QString objectPath = positiveDataset.absoluteFilePath(objectName);
      
      QSharedPointer<ObjedImage> expendedObjectImage = ObjedImage::create(image, expendedObject);
      if (expendedObjectImage->isEmpty() == true)
        continue;

      expendedObjectImage->save(objectPath);

      objectCount++;
    }
  }

  if (negativeDataset.path().isEmpty() == false && negativeDataset.exists() == true)
  {
    if (markup->objects().count() == 0)
    {
      QString negativeName = QString("%0.%1").arg(imageName).arg(imageSuffix);
      QString negativePath = negativeDataset.absoluteFilePath(negativeName);

      image->save(negativePath);

      negativeCount++;
    }
    else if (markup->objects().count() == 1)
    {
      const QRect object = markup->objects().first();
      const QRect topRect(0, 0, image->width(), object.y() + object.height() / 2);
      const QRect leftRect(0, 0, object.x() + object.width() / 2, image->height());
      const QRect rightRect(leftRect.width(), 0, image->width() - leftRect.width(), image->height());
      const QRect bottomRect(0, topRect.height(), image->width(), image->height() - topRect.height());

      QSharedPointer<ObjedImage> topImage = ObjedImage::create(image, topRect);
      if (topImage != 0) topImage->save(negativeDataset.absoluteFilePath(QString("%0t.%1").arg(imageName).arg(imageSuffix)));

      QSharedPointer<ObjedImage> leftImage = ObjedImage::create(image, leftRect);
      if (leftImage != 0) leftImage->save(negativeDataset.absoluteFilePath(QString("%0l.%1").arg(imageName).arg(imageSuffix)));

      QSharedPointer<ObjedImage> rightImage = ObjedImage::create(image, rightRect);
      if (rightImage != 0) rightImage->save(negativeDataset.absoluteFilePath(QString("%0r.%1").arg(imageName).arg(imageSuffix)));

      QSharedPointer<ObjedImage> bottomImage = ObjedImage::create(image, bottomRect);
      if (bottomImage != 0) bottomImage->save(negativeDataset.absoluteFilePath(QString("%0b.%1").arg(imageName).arg(imageSuffix)));
    }
  }
}

void ObjedCutCutting::summarize()
{
  ObjedConsole::printInfo(QString("Cutting:"));
  ObjedConsole::printInfo(QString("Total cut object count:          %0").arg(objectCount));
  ObjedConsole::printInfo(QString("Total cut negative image count:  %0").arg(negativeCount));
}

ObjedCutStatistics::ObjedCutStatistics(ObjedConfig *config) : 
starredImageCount(0), totalImageCount(0)
{
  return;
}

ObjedCutStatistics::~ObjedCutStatistics()
{
  return;
}

void ObjedCutStatistics::process(ObjedImage *image, const ObjedMarkup *markup, const QString &imageName)
{
  Q_ASSERT(image != 0 && markup != 0);

  totalImageCount++;
  starredImageCount += markup->isStarred();

  foreach (QRect object, markup->objects())
    objectSizeList.append(object.size());
}

void ObjedCutStatistics::summarize()
{
  qSort(objectSizeList.begin(), objectSizeList.end(), sizeLessThan);
  QSize minSize = objectSizeList.count() > 0 ? objectSizeList.first() : QSize(0, 0);
  QSize maxSize = objectSizeList.count() > 0 ? objectSizeList.last() : QSize(0, 0);
  QSize midSize = objectSizeList.count() > 0 ? objectSizeList.value(objectSizeList.count() / 2) : QSize(0, 0);

  ObjedConsole::printInfo(QString("Statistics:"));
  ObjedConsole::printInfo(QString("Total image count:         %0").arg(totalImageCount));
  ObjedConsole::printInfo(QString("Starred image count:       %0").arg(starredImageCount));
  ObjedConsole::printInfo(QString("Total object count:        %0").arg(objectSizeList.count()));
  ObjedConsole::printInfo(QString("Minimum object size:       %0x%1").arg(minSize.width()).arg(minSize.height()));
  ObjedConsole::printInfo(QString("Median object size:        %0x%1").arg(midSize.width()).arg(midSize.height()));
  ObjedConsole::printInfo(QString("Maximum object size:       %0x%1").arg(maxSize.width()).arg(maxSize.height()));
}