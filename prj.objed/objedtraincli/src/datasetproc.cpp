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

#include <QSharedPointer>

#include <objedutils/objedconsole.h>
#include <objedutils/objedconfig.h>
#include <objedutils/objedopenmp.h>
#include <objedutils/objedimage.h>
#include <objedutils/objedexp.h>
#include <objedutils/objedsys.h>

#include "objedtrainutils.h"
#include "datasetproc.h"

static QStringList makeImagePathList(const QVariantList &datasetList)
{
  QStringList imagePathList;

  for (int i = 0; i < datasetList.count(); i++)
  {
    QDir datasetDir(datasetList[i].toString());
    QStringList imageFilterList = ObjedSys::imageFilters();
    QStringList imageNameList = datasetDir.entryList(imageFilterList);

    for (int j = 0; j < imageNameList.count(); j++)
      imagePathList.append(datasetDir.absoluteFilePath(imageNameList[j]));
  }

  return imagePathList;
}

DatasetProcessor::DatasetProcessor(ObjedConfig *config) : 
classifierWidth(0), classifierHeight(0), minScale(1.0), 
maxScale(1.0), stpScale(0.1), negativeCount(0)
{
  classifierWidth = config->value("ClassifierWidth").toInt();
  classifierHeight = config->value("ClassifierHeight").toInt();

  minScale = config->value("MinimumScale").toDouble();
  maxScale = config->value("MaximumScale").toDouble();
  stpScale = config->value("StepScale").toDouble();
  if (minScale > maxScale || stpScale <= 1.0)
    throw ObjedException("Invalid Scale values");
  
  negativeCount = config->value("NegativeCount").toInt();
  if (negativeCount <= 0)
    throw ObjedException("Invalid NegativeCount value");

  positiveImagePathList = makeImagePathList(config->listValue("PositiveDatasetList"));
  if (positiveImagePathList.isEmpty() == true) throw ObjedException("There are no positive images");

  negativeImagePathList = makeImagePathList(config->listValue("NegativeDatasetList"));
  if (negativeImagePathList.isEmpty() == true) throw ObjedException("There are no negative images");
}

DatasetProcessor::~DatasetProcessor()
{
  return;
}

SampleList DatasetProcessor::preparePositiveSamples(objed::Classifier *classifier)
{
  if (classifier == 0)
    throw ObjedException("Invalid passed classifier");
  if (classifier->width() != classifierWidth || classifier->height() != classifierHeight)
    throw ObjedException("Invalid classifier size");

  SampleList positiveSamples;
  ObjedOpenMP::ClassifierList clasifierList = ObjedOpenMP::multiplyClassifier(classifier);
  QVector<SampleList> positiveSamplesList(ObjedOpenMP::maxThreadCount());

  int loadedSampleCount = 0;

  #pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < positiveImagePathList.count(); i++)
  {
    QString imagePath = positiveImagePathList[i];
    QString imageName = QFileInfo(imagePath).fileName();

    int progress = 100 * loadedSampleCount / positiveImagePathList.count();
    ObjedConsole::printProgress(QString("Processing positive image %0").arg(imageName), progress);

    #pragma omp atomic
    loadedSampleCount++;

    objed::Classifier *classifier = clasifierList[ObjedOpenMP::threadId()].data();
    SampleList &positiveSamples = positiveSamplesList[ObjedOpenMP::threadId()];

    QSharedPointer<ObjedImage> image = ObjedImage::create(imagePath);
    image->resize(QSize(classifierWidth, classifierHeight));
    QSharedPointer<Sample> sample(new Sample(image.data(), imagePath));
    
    float result = objed::epsilon;
    classifier->prepare(sample->imagePool);
    classifier->evaluate(&result, classifierWidth / 2, classifierHeight / 2);

    if (result <= 0.0) 
      continue;

    positiveSamples.append(sample);
  }

  for (int i = 0; i < positiveSamplesList.count(); i++)
    positiveSamples.append(positiveSamplesList[i]);

  ObjedConsole::printProgress(QString("%0 positive sample were generated").arg(positiveSamples.count()), 100);
  return positiveSamples;
}

SampleList DatasetProcessor::prepareNegativeSamples(objed::Classifier *classifier)
{
  if (classifier == 0)
    throw ObjedException("Invalid passed classifier");
  if (classifier->width() != classifierWidth || classifier->height() != classifierHeight)
    throw ObjedException("Invalid classifier size");

  ObjedOpenMP::ClassifierList clasifierList = ObjedOpenMP::multiplyClassifier(classifier);
  QVector<QSharedPointer<objed::ImagePool> > imagePoolList(ObjedOpenMP::maxThreadCount());

  for (int i = 0; i < ObjedOpenMP::maxThreadCount(); i++)
  {
    imagePoolList[i].reset(objed::ImagePool::create(), objed::ImagePool::destroy);
    clasifierList[i]->prepare(imagePoolList[i].data());
  }

  SampleList negativeSamples;
  for (int i = 0; i < negativeImagePathList.count() && negativeSamples.count() < negativeCount; i++)
  {
    QString imagePath = negativeImagePathList[i];
    QString imageName = QFileInfo(imagePath).fileName();

    int progress = 100 * i / negativeImagePathList.count();
    ObjedConsole::printProgress(QString("Processing negative image %0").arg(imageName), progress);

    QList<QSharedPointer<ObjedImage> > imagePyramid = prepareImagePyramid(imagePath);
    qint64 period = computePeriod(imagePyramid);

    for (qint64 shift = 0; shift < period && negativeSamples.count() < negativeCount; shift++)
    {
      QVector<SampleList> negativeSamplesList(ObjedOpenMP::maxThreadCount());

      #pragma omp parallel for schedule(dynamic)
      for (int i = 0; i < imagePyramid.size(); i++)
      {
        objed::Classifier *classifier = clasifierList[ObjedOpenMP::threadId()].data();
        objed::ImagePool *imagePool = imagePoolList[ObjedOpenMP::threadId()].data();
        SampleList &negativeSamples = negativeSamplesList[ObjedOpenMP::threadId()];

        QSharedPointer<ObjedImage> image = imagePyramid[i];
        imagePool->update(image->image());

        qint64 subwindowNumber = 0;
        for (int y = classifierHeight / 2; y < image->height() - classifierHeight / 2; y++)
        {
          for (int x = classifierWidth / 2; x < image->width() - classifierWidth / 2; x++, subwindowNumber++)
          {
            float result = objed::epsilon;
            classifier->evaluate(&result, x, y);

            if ((result > 0) && ((subwindowNumber % period) == shift))
            {
              QSharedPointer<ObjedImage> imageRegion = ObjedImage::create(image.data(), 
                QRect(x - classifierWidth / 2, y - classifierHeight / 2, classifierWidth, classifierHeight));

              QSharedPointer<Sample> sample(new Sample(imageRegion.data(), imagePath));
              negativeSamples.append(sample);
            }
          }
        }
      }

      for (int i = 0; i < negativeSamplesList.count(); i++)
        negativeSamples.append(negativeSamplesList[i]);
    }
  }

  ObjedConsole::printProgress(QString("%0 negative sample were generated").arg(negativeSamples.count()), 100);
  return negativeSamples;
}

QList<QSharedPointer<ObjedImage> > DatasetProcessor::prepareImagePyramid(const QString &imagePath)
{
  QList<QSharedPointer<ObjedImage> > imagePyramid;
  for (double scale = minScale; scale <= maxScale; scale *= stpScale)
  {
    QSharedPointer<ObjedImage> image = ObjedImage::create(imagePath);
    if (image->isEmpty() == true)
      continue;

    int wd = std::max(1, static_cast<int>(image->width() / scale + 0.5));
    int ht = std::max(1, static_cast<int>(image->height() / scale + 0.5));
    
    image->resize(QSize(wd, ht));
    imagePyramid.append(image);
  }

  return imagePyramid;
}

qint64 DatasetProcessor::computePeriod(const QList<QSharedPointer<ObjedImage> > &imagePyramid)
{
  qint64 subwindowCount = 0;
  foreach (const QSharedPointer<ObjedImage> image, imagePyramid)
  {
    if (image->isEmpty() == true)
      continue;

    subwindowCount += std::max(0, image->width() - classifierWidth) *
      std::max(0, image->height() - classifierHeight);
  }

  return qMax<qint64>(1, subwindowCount / negativeCount);
}
