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

#include <QVariantMap>

#include <objedutils/objedconf.h>

#include <objed/src/haar1stumpcl.h>
#include <objed/src/haar2stumpcl.h>
#include <objed/src/haar3stumpcl.h>
#include <objed/src/haar1pwcl.h>
#include <objed/src/haar2pwcl.h>
#include <objed/src/haar3pwcl.h>
#include <objed/simplest.h>

#include "objedtrainutils.h"

Sample::Sample(const ObjedImage *image, const QString &path) :
imagePool(0), weight(0.0), sourceImagePath(path)
{
  imagePool = objed::ImagePool::create();
  imagePool->update(image->image());
}

Sample::~Sample()
{
  objed::ImagePool::destroy(imagePool);
}

static QList<int> parseIntRange(const QString &line)
{
  QList<int> intRangeList;
  int minValue = 0, maxValue = 0, step = 0;

#ifdef _MSC_VER
  sscanf_s(qPrintable(line), "%d:%d/%d", &minValue, &maxValue, &step);
#else
  sscanf(qPrintable(line), "%d:%d/%d", &minValue, &maxValue, &step);
#endif

  maxValue = qMax(minValue, maxValue); step = qMax(1, step);
  for (int value = minValue; value <= maxValue; value += step)
    intRangeList.append(value);

  return intRangeList;
}

WcList WcMaker::make(const QString &wcLine, const QSize &size)
{
  QStringList itemList = wcLine.split(" ", QString::SkipEmptyParts);
  if (itemList.isEmpty() == true)
    return WcList();

  QVariantMap wcParams;
  for (int i = 1; i < itemList.count(); i++)
  {
    QStringList paramList = itemList.value(i).split("=");
    if (paramList.count() != 2) 
      continue;

    QString paramName = paramList.first().remove("-").simplified();
    QVariant paramValue = paramList.last().simplified();
    wcParams.insert(paramName, paramValue);
  }
  
  if (itemList.first().simplified() == "Haar1StumpWc")
    return makeHaar1StumpWc(wcParams, size);
  if (itemList.first().simplified() == "Haar2StumpWc")
    return makeHaar2StumpWc(wcParams, size);
  if (itemList.first().simplified() == "Haar3StumpWc")
    return makeHaar3StumpWc(wcParams, size);
  if (itemList.first().simplified() == "Haar1PwWc")
    return makeHaar1PwWc(wcParams, size);
  if (itemList.first().simplified() == "Haar2PwWc")
    return makeHaar2PwWc(wcParams, size);
  if (itemList.first().simplified() == "Haar3PwWc")
    return makeHaar3PwWc(wcParams, size);

  return WcList();
}

QStringList WcMaker::help()
{
  return QStringList() 
    << "Haar1StumpWc -preproc=<preproc> -wd=<rectWd> -ht=<rectHt>"
    << "Haar1PwWc -preproc=<preproc> -bins=<binCount> -wd=<rectWd> -ht=<rectHt>"
    << "Haar2StumpWc -preproc=<preproc> -wd=<rectWd> -ht=<rectHt> -dist=<rectDist> -normalize=<true|false>"
    << "Haar2PwWc -preproc=<preproc> -bins=<binCount> -wd=<rectWd> -ht=<rectHt> -dist=<rectDist> -normalize=<true|false>"
    << "Haar3StumpWc -preproc=<preproc> -wd=<rectWd> -ht=<rectHt> -dist=<rectDist> -normalize=<true|false>"
    << "Haar3PwWc -preproc=<preproc> -bins=<binCount> -wd=<rectWd> -ht=<rectHt> -dist=<rectDist> -normalize=<true|false>";
}

WcList WcMaker::makeHaar1StumpWc(const QVariantMap &wcParams, const QSize &size)
{
  QList<int> rectWdRange = parseIntRange(wcParams["wd"].toString());
  QList<int> rectHtRange = parseIntRange(wcParams["ht"].toString());
  QString preproc = wcParams["preproc"].toString();

  QList<QSharedPointer<objed::Classifier> > wcList;

  foreach (int rectWd, rectWdRange)
  {
    foreach (int rectHt, rectHtRange)
    {
      const int xStep = std::max(1, rectWd / 4), yStep = std::max(1, rectHt / 4);
      for (int y = -size.height() / 2 + 1; y <= size.height() / 2 - 1; y += yStep)
      {
        for (int x = -size.width() / 2 + 1; x <= size.width() / 2 - 1; x += xStep)
        {
          objed::Rect<int> rect(x, y, rectWd, rectHt);
          if (rect.x + rect.width > size.width() / 2 - 1)
            continue;
          if (rect.y + rect.height > size.height() / 2 - 1)
            continue;

          objed::Haar1StumpClassifier *wc = new 
            objed::Haar1StumpClassifier(size.width(), size.height());

          wc->rect = rect; wc->preproc = preproc.toStdString();
          wcList.append(QSharedPointer<objed::Classifier>(wc));
        }
      }
    }
  }

  return wcList;
}

WcList WcMaker::makeHaar2StumpWc(const QVariantMap &wcParams, const QSize &size)
{
  QList<int> rectWdRange = parseIntRange(wcParams["wd"].toString());
  QList<int> rectHtRange = parseIntRange(wcParams["ht"].toString());
  QList<int> distRange = parseIntRange(wcParams["dist"].toString());
  QString preproc = wcParams["preproc"].toString();
  bool normalize = wcParams["normalize"].toBool();

  QList<QSharedPointer<objed::Classifier> > wcList;

  foreach (int rectWd, rectWdRange)
  {
    foreach (int rectHt, rectHtRange)
    {
      foreach (int dist, distRange)
      {
        const int xStep = std::max(1, rectWd / 4), yStep = std::max(1, rectHt / 4);
        for (int y = -size.height() / 2 + 1; y <= size.height() / 2 - 1; y += yStep)
        {
          for (int x = -size.width() / 2 + 1; x <= size.width() / 2 - 1; x += xStep)
          {
            objed::Rect<int> rect0(x, y, rectWd, rectHt);
            objed::Rect<int> rect1hor(x + dist + rectWd, y, rectWd, rectHt);
            objed::Rect<int> rect1ver(x, y + dist + rectHt, rectWd, rectHt);

            if (rect0.x + rect0.width > size.width() / 2 - 1)
              continue;
            if (rect0.y + rect0.height > size.height() / 2 - 1)
              continue;

            if (rect1hor.x + rect1hor.width <= size.width() / 2 - 1)
            {
              objed::Haar2StumpClassifier *wc = 
                wc = new objed::Haar2StumpClassifier(size.width(), size.height());

              wc->normalize = normalize;
              wc->preproc = preproc.toStdString();
              wc->rect0 = rect0; wc->rect1 = rect1hor;
              wcList.append(QSharedPointer<objed::Classifier>(wc));
            }
              
            if (rect1ver.y + rect1ver.height <= size.height() / 2 - 1)
            {
              objed::Haar2StumpClassifier *wc = 
                wc = new objed::Haar2StumpClassifier(size.width(), size.height());

              wc->normalize = normalize;
              wc->preproc = preproc.toStdString();
              wc->rect0 = rect0; wc->rect1 = rect1ver;
              wcList.append(QSharedPointer<objed::Classifier>(wc));
            }
          }
        }
      }
    }
  }

  return wcList;
}

WcList WcMaker::makeHaar3StumpWc(const QVariantMap &wcParams, const QSize &size)
{
  QList<int> rectWdRange = parseIntRange(wcParams["wd"].toString());
  QList<int> rectHtRange = parseIntRange(wcParams["ht"].toString());
  QList<int> distRange = parseIntRange(wcParams["dist"].toString());
  QString preproc = wcParams["preproc"].toString();
  bool normalize = wcParams["normalize"].toBool();

  QList<QSharedPointer<objed::Classifier> > wcList;

  foreach (int rectWd, rectWdRange)
  {
    foreach (int rectHt, rectHtRange)
    {
      foreach (int dist, distRange)
      {
        const int xStep = std::max(1, rectWd / 4), yStep = std::max(1, rectHt / 4);
        for (int y = -size.height() / 2 + 1; y <= size.height() / 2 - 1; y += yStep)
        {
          for (int x = -size.width() / 2 + 1; x <= size.width() / 2 - 1; x += xStep)
          {
            objed::Rect<int> rect0(x, y, rectWd, rectHt);
            objed::Rect<int> rect1hor(x + dist + rectWd, y, rectWd, rectHt);
            objed::Rect<int> rect1ver(x, y + dist + rectHt, rectWd, rectHt);
            objed::Rect<int> rect2hor(x + 2 * dist + 2 * rectWd, y, rectWd, rectHt);
            objed::Rect<int> rect2ver(x, y + 2 * dist + 2 * rectHt, rectWd, rectHt);


            if (rect0.x + rect0.width > size.width() / 2 - 1)
              continue;
            if (rect0.y + rect0.height > size.height() / 2 - 1)
              continue;

            if (rect2hor.x + rect2hor.width <= size.width() / 2 - 1)
            {
              objed::Haar3StumpClassifier *wc = 
                wc = new objed::Haar3StumpClassifier(size.width(), size.height());

              wc->normalize = normalize;
              wc->preproc = preproc.toStdString();
              wc->rect0 = rect0; wc->rect1 = rect1hor; wc->rect2 = rect2hor;
              wcList.append(QSharedPointer<objed::Classifier>(wc));
            }

            if (rect2ver.y + rect2ver.height <= size.height() / 2 - 1)
            {
              objed::Haar3StumpClassifier *wc = 
                wc = new objed::Haar3StumpClassifier(size.width(), size.height());

              wc->normalize = normalize;
              wc->preproc = preproc.toStdString();
              wc->rect0 = rect0; wc->rect1 = rect1ver; wc->rect2 = rect2ver;
              wcList.append(QSharedPointer<objed::Classifier>(wc));
            }
          }
        }
      }
    }
  }

  return wcList;
}

WcList WcMaker::makeHaar1PwWc(const QVariantMap &wcParams, const QSize &size)
{
  QList<int> rectWdRange = parseIntRange(wcParams["wd"].toString());
  QList<int> rectHtRange = parseIntRange(wcParams["ht"].toString());
  QList<int> binCountRange = parseIntRange(wcParams["bins"].toString());
  QString preproc = wcParams["preproc"].toString();

  QList<QSharedPointer<objed::Classifier> > wcList;

  foreach (int rectWd, rectWdRange)
  {
    foreach (int rectHt, rectHtRange)
    {
      foreach (size_t binCount, binCountRange)
      {
        const int xStep = std::max(1, rectWd / 4), yStep = std::max(1, rectHt / 4);
        for (int y = -size.height() / 2 + 1; y <= size.height() / 2 - 1; y += yStep)
        {
          for (int x = -size.width() / 2 + 1; x <= size.width() / 2 - 1; x += xStep)
          {
            objed::Rect<int> rect(x, y, rectWd, rectHt);
            if (rect.x + rect.width > size.width() / 2 - 1)
              continue;
            if (rect.y + rect.height > size.height() / 2 - 1)
              continue;

            objed::Haar1PwClassifier *wc = new 
              objed::Haar1PwClassifier(size.width(), size.height());

            wc->bins.resize(binCount, static_cast<float>(0.0));
            wc->rect = rect; wc->preproc = preproc.toStdString();
            wcList.append(QSharedPointer<objed::Classifier>(wc));
          }
        }
      }
    }
  }

  return wcList;
}

WcList WcMaker::makeHaar2PwWc(const QVariantMap &wcParams, const QSize &size)
{
  QList<int> rectWdRange = parseIntRange(wcParams["wd"].toString());
  QList<int> rectHtRange = parseIntRange(wcParams["ht"].toString());
  QList<int> distRange = parseIntRange(wcParams["dist"].toString());
  QList<int> binCountRange = parseIntRange(wcParams["bins"].toString());
  QString preproc = wcParams["preproc"].toString();
  bool normalize = wcParams["normalize"].toBool();

  QList<QSharedPointer<objed::Classifier> > wcList;

  foreach (int rectWd, rectWdRange)
  {
    foreach (int rectHt, rectHtRange)
    {
      foreach (int dist, distRange)
      {
        foreach (size_t binCount, binCountRange)
        {
          const int xStep = std::max(1, rectWd / 4), yStep = std::max(1, rectHt / 4);
          for (int y = -size.height() / 2 + 1; y <= size.height() / 2 - 1; y += yStep)
          {
            for (int x = -size.width() / 2 + 1; x <= size.width() / 2 - 1; x += xStep)
            {
              objed::Rect<int> rect0(x, y, rectWd, rectHt);
              objed::Rect<int> rect1hor(x + dist + rectWd, y, rectWd, rectHt);
              objed::Rect<int> rect1ver(x, y + dist + rectHt, rectWd, rectHt);

              if (rect0.x + rect0.width > size.width() / 2 - 1)
                continue;
              if (rect0.y + rect0.height > size.height() / 2 - 1)
                continue;

              if (rect1hor.x + rect1hor.width <= size.width() / 2 - 1)
              {
                objed::Haar2PwClassifier *wc = 
                  wc = new objed::Haar2PwClassifier(size.width(), size.height());

                wc->normalize = normalize;
                wc->preproc = preproc.toStdString();
                wc->rect0 = rect0; wc->rect1 = rect1hor;
                wc->bins.resize(binCount, static_cast<float>(0.0));
                wcList.append(QSharedPointer<objed::Classifier>(wc));
              }

              if (rect1ver.y + rect1ver.height <= size.height() / 2 - 1)
              {
                objed::Haar2PwClassifier *wc = 
                  wc = new objed::Haar2PwClassifier(size.width(), size.height());

                wc->normalize = normalize;
                wc->preproc = preproc.toStdString();
                wc->rect0 = rect0; wc->rect1 = rect1ver;
                wc->bins.resize(binCount, static_cast<float>(0.0));
                wcList.append(QSharedPointer<objed::Classifier>(wc));
              }
            }
          }
        }
      }
    }
  }

  return wcList;
}

WcList WcMaker::makeHaar3PwWc(const QVariantMap &wcParams, const QSize &size)
{
  QList<int> rectWdRange = parseIntRange(wcParams["wd"].toString());
  QList<int> rectHtRange = parseIntRange(wcParams["ht"].toString());
  QList<int> distRange = parseIntRange(wcParams["dist"].toString());
  QList<int> binCountRange = parseIntRange(wcParams["bins"].toString());
  QString preproc = wcParams["preproc"].toString();
  bool normalize = wcParams["normalize"].toBool();

  QList<QSharedPointer<objed::Classifier> > wcList;

  foreach (int rectWd, rectWdRange)
  {
    foreach (int rectHt, rectHtRange)
    {
      foreach (int dist, distRange)
      {
        foreach (size_t binCount, binCountRange)
        {
          const int xStep = std::max(1, rectWd / 4), yStep = std::max(1, rectHt / 4);
          for (int y = -size.height() / 2 + 1; y <= size.height() / 2 - 1; y += yStep)
          {
            for (int x = -size.width() / 2 + 1; x <= size.width() / 2 - 1; x += xStep)
            {
              objed::Rect<int> rect0(x, y, rectWd, rectHt);
              objed::Rect<int> rect1hor(x + dist + rectWd, y, rectWd, rectHt);
              objed::Rect<int> rect1ver(x, y + dist + rectHt, rectWd, rectHt);
              objed::Rect<int> rect2hor(x + 2 * dist + 2 * rectWd, y, rectWd, rectHt);
              objed::Rect<int> rect2ver(x, y + 2 * dist + 2 * rectHt, rectWd, rectHt);


              if (rect0.x + rect0.width > size.width() / 2 - 1)
                continue;
              if (rect0.y + rect0.height > size.height() / 2 - 1)
                continue;

              if (rect2hor.x + rect2hor.width <= size.width() / 2 - 1)
              {
                objed::Haar3PwClassifier *wc = 
                  wc = new objed::Haar3PwClassifier(size.width(), size.height());

                wc->normalize = normalize;
                wc->preproc = preproc.toStdString();
                wc->rect0 = rect0; wc->rect1 = rect1hor; wc->rect2 = rect2hor;
                wc->bins.resize(binCount, static_cast<float>(0.0));
                wcList.append(QSharedPointer<objed::Classifier>(wc));
              }

              if (rect2ver.y + rect2ver.height <= size.height() / 2 - 1)
              {
                objed::Haar3PwClassifier *wc = 
                  wc = new objed::Haar3PwClassifier(size.width(), size.height());

                wc->normalize = normalize;
                wc->preproc = preproc.toStdString();
                wc->rect0 = rect0; wc->rect1 = rect1ver; wc->rect2 = rect2ver;
                wc->bins.resize(binCount, static_cast<float>(0.0));
                wcList.append(QSharedPointer<objed::Classifier>(wc));
              }
            }
          }
        }
      }
    }
  }

  return wcList;
}

ParityCascadeClassifier::ParityCascadeClassifier(int width, int height) :
clWidth(width), clHeight(height)
{
  assert(clWidth > 0 && clHeight > 0);
  assert(clWidth & 1 && clHeight & 1);
}

ParityCascadeClassifier::~ParityCascadeClassifier()
{
  for (size_t i = 0; i < clList.size(); i++)
    objed::Classifier::destroy(clList[i]);

  clList.clear();
  parityList.clear();
}

int ParityCascadeClassifier::width() const
{
  return clWidth;
}

int ParityCascadeClassifier::height() const
{
  return clHeight;
}

bool ParityCascadeClassifier::prepare(objed::ImagePool *imagePool)
{
  bool ok = true;
  for (size_t i = 0; i < clList.size(); i++)
    ok &= clList[i]->prepare(imagePool);

  return ok;
}

bool ParityCascadeClassifier::evaluate(float *result, int x, int y) const
{
  bool ok = true;
  for (size_t i = 0; i < clList.size(); i++)
  {
    ok &= clList[i]->evaluate(result, x, y);
    *result *= parityList[i];
    if (*result < 0)
      return ok;
  }

  return ok;
}

Json::Value ParityCascadeClassifier::serialize() const
{
  assert(false);
  return Json::Value();
}

std::string ParityCascadeClassifier::type() const
{
  return std::string("parityCascadeClassifier");
}

objed::Classifier * ParityCascadeClassifier::clone() const
{
  ParityCascadeClassifier *newCascadeCl = new ParityCascadeClassifier(width(), height());
  assert(newCascadeCl != 0);

  for (size_t i = 0; i < this->clList.size(); i++)
  {
    newCascadeCl->clList.push_back(this->clList[i]->clone());
    newCascadeCl->parityList.push_back(this->parityList[i]);
  }

  return newCascadeCl;
}
