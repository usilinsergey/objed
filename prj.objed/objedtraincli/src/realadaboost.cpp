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

#include <QVector>

#include <objedutils/objedconsole.h>
#include <objedutils/objedconf.h>
#include <objedutils/objedexp.h>

#include <objed/src/additivecl.h>
#include <objed/src/haar1stumpcl.h>
#include <objed/src/haar2stumpcl.h>
#include <objed/src/haar3stumpcl.h>
#include <objed/src/haar1pwcl.h>
#include <objed/src/haar2pwcl.h>
#include <objed/src/haar3pwcl.h>

#include <objed/src/additivecl.h>
#include <objed/src/linearcl.h>
#include <objed/src/maxcl.h>

#include <climits>

#include "trainscproc.h"

template<class T>
static double trainHaarStumpWc(objed::Classifier *wc, const SampleList &posSampleList, const SampleList &negSampleList)
{
  const int clWidth2 = wc->width() / 2, clHeight2 = wc->height() / 2;
  T *haarWc = static_cast<T *>(wc);
  Q_ASSERT(haarWc != 0);

  double posWeights0[256] = {0}, posWeights1[256] = {0};
  double negWeights0[256] = {0}, negWeights1[256] = {0};

  for (int i = 0; i < posSampleList.count(); i++)
  {
    Sample *posSample = posSampleList[i].data();
    int value = haarWc->compute(posSample->imagePool->
      integral(haarWc->preproc), clWidth2, clHeight2);

    for (int i = 0; i < 256; i++)
    {
      if (value > i) 
        posWeights0[i] += posSample->weight;
      else 
        posWeights1[i] +=posSample->weight;
    }
  }

  for (int i = 0; i < negSampleList.count(); i++)
  {
    Sample *negSample = negSampleList[i].data();
    int value = haarWc->compute(negSample->imagePool->
      integral(haarWc->preproc), clWidth2, clHeight2);

    for (int i = 0; i < 256; i++)
    {
      if (value > i)
        negWeights0[i] += negSample->weight;
      else 
        negWeights1[i] += negSample->weight;
    }
  }

  double bestZ = 1.0 + objed::epsilon;
  double sEpsilon = objed::epsilon / (posSampleList.count() + negSampleList.count());

  for (int i = 0; i < 256; i++)
  {
    double z = 2 * (std::sqrt(posWeights0[i] * negWeights0[i]) + std::sqrt(posWeights1[i] * negWeights1[i]));
    if (z < bestZ)
    {
      haarWc->values[0] = 0.5 * std::log((posWeights0[i] + sEpsilon) / (negWeights0[i] + sEpsilon));
      haarWc->values[1] = 0.5 * std::log((posWeights1[i] + sEpsilon) / (negWeights1[i] + sEpsilon));
      haarWc->threshold = i;
      bestZ = z;
    }
  }

  return bestZ;
}

template<class T>
static double trainHaarPwWc(objed::Classifier *wc, const SampleList &posSampleList, const SampleList &negSampleList)
{
  const int clWidth2 = wc->width() / 2, clHeight2 = wc->height() / 2;
  T *haarWc = static_cast<T *>(wc);
  Q_ASSERT(haarWc != 0);

  double *posWeights = new double[haarWc->bins.size()];
  double *negWeights = new double[haarWc->bins.size()];
  memset(posWeights, 0, sizeof(double) * haarWc->bins.size());
  memset(negWeights, 0, sizeof(double) * haarWc->bins.size());

  for (int i = 0; i < posSampleList.count(); i++)
  {
    Sample *posSample = posSampleList[i].data();
    int value = haarWc->compute(posSample->imagePool->
      integral(haarWc->preproc), clWidth2, clHeight2);
    posWeights[value * haarWc->bins.size() / 256] += posSample->weight;
  }

  for (int i = 0; i < negSampleList.count(); i++)
  {
    Sample *negSample = negSampleList[i].data();
    int value = haarWc->compute(negSample->imagePool->
      integral(haarWc->preproc), clWidth2, clHeight2);
    negWeights[value * haarWc->bins.size() / 256] += negSample->weight;
  }

  double z = 0.0;
  for (size_t i = 0; i < haarWc->bins.size(); i++)
  {
    double sEpsilon = objed::epsilon / (posSampleList.count() + negSampleList.count());
    haarWc->bins[i] = 0.5 * std::log((posWeights[i] + sEpsilon) / (negWeights[i] + sEpsilon));
    z += 2 * std::sqrt(posWeights[i] * negWeights[i]);
  }

  delete[] posWeights;
  delete[] negWeights;

  return z;
}

static double trainWcReal(objed::Classifier *wc,  const SampleList &posSampleList, const SampleList &negSampleList)
{
  if (dynamic_cast<objed::Haar1StumpClassifier *>(wc) != 0)
    return trainHaarStumpWc<objed::Haar1StumpClassifier>(wc, posSampleList, negSampleList);
  if (dynamic_cast<objed::Haar2StumpClassifier *>(wc) != 0)
    return trainHaarStumpWc<objed::Haar2StumpClassifier>(wc, posSampleList, negSampleList);
  if (dynamic_cast<objed::Haar3StumpClassifier *>(wc) != 0)
    return trainHaarStumpWc<objed::Haar3StumpClassifier>(wc, posSampleList, negSampleList);
  if (dynamic_cast<objed::Haar1PwClassifier *>(wc) != 0)
    return trainHaarPwWc<objed::Haar1PwClassifier>(wc, posSampleList, negSampleList);
  if (dynamic_cast<objed::Haar2PwClassifier *>(wc) != 0)
    return trainHaarPwWc<objed::Haar2PwClassifier>(wc, posSampleList, negSampleList);
  if (dynamic_cast<objed::Haar3PwClassifier *>(wc) != 0)
    return trainHaarPwWc<objed::Haar3PwClassifier>(wc, posSampleList, negSampleList);

  Q_ASSERT(false);
  return 1.0;
}

static QSharedPointer<objed::Classifier> realAdaBoost(WcList &wcList, SampleList &positiveSamples, 
  SampleList &negativeSamples, const QString &progressLabel)
{
  Q_ASSERT(wcList.count() > 0);
  Q_ASSERT(positiveSamples.count() > 0);
  Q_ASSERT(negativeSamples.count() > 0);

  if (wcList.isEmpty() == true)
    throw ObjedException("No weak classifier");
  if (positiveSamples.isEmpty() == true || negativeSamples.isEmpty() == true)
    throw ObjedException("No positive or negative samples");

  QVector<double> zList(wcList.count());
  int trainedWcCount = 0, wcCount = wcList.count();

#pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < wcCount; i++)
  {
    if (trainedWcCount % 100 == 0)
    {
      int progress = 100 * trainedWcCount / wcCount;
      ObjedConsole::printProgress(progressLabel, progress);
    }

    zList[i] = trainWcReal(wcList[i].data(), positiveSamples, negativeSamples);

#pragma omp atomic
    trainedWcCount++;
  }

  double bestZ = 1.0 + objed::epsilon;
  objed::Classifier *bestWc = 0;

  for (int i = 0; i < zList.count(); i++)
  {
    if (zList[i] < bestZ)
    {
      bestWc = wcList[i].data();
      bestZ = zList[i];
    }
  }

  if (bestZ > 1.0 || bestWc == 0)
    throw ObjedException("Cannot train next weak classifier (bestZ > 1.0 || bestWc == 0)");

  ObjedConsole::printProgress(progressLabel, 100);
  return QSharedPointer<objed::Classifier>(bestWc->clone(), objed::Classifier::destroy);
}

void TrainScProcessor::trainScRealAdaBoost(objed::Classifier *&classifier, SampleList &positiveSamples, SampleList &negativeSamples)
{
  if (classifier == 0)
    classifier = new objed::AdditiveClassifier(classifierWidth, classifierHeight);

  objed::AdditiveClassifier *sc = dynamic_cast<objed::AdditiveClassifier *>(classifier);
  Q_ASSERT(sc != 0);

  setSampleWeights(positiveSamples, negativeSamples, sc);
  QSharedPointer<objed::Classifier> bestWc = realAdaBoost(wcList, positiveSamples, negativeSamples, 
    QString("Training strong classifier iteration %0").arg(sc->clList.size() + 1));
  sc->clList.push_back(bestWc->clone());
}

void TrainScProcessor::trainScRealAdaBoostMax(objed::Classifier *&classifier, SampleList &positiveSamples, SampleList &negativeSamples)
{
  if (classifier == 0)
  {
    objed::MaxClassifier *maxSc = new objed::MaxClassifier(classifierWidth, classifierHeight);
    maxSc->clList.resize(methodParams.toInt());
    classifier = maxSc;
  }


  objed::MaxClassifier *maxSc = dynamic_cast<objed::MaxClassifier *>(classifier);
  Q_ASSERT(maxSc != 0);

  const int x = maxSc->width() / 2, y = maxSc->height() / 2;
  const int scCount = maxSc->clList.size();

  for (int iSc = 0; iSc < scCount; iSc++)
  {
    if (maxSc->clList[iSc] == 0)
      maxSc->clList[iSc] = new objed::AdditiveClassifier(classifierWidth, classifierHeight);

    objed::AdditiveClassifier *sc = dynamic_cast<objed::AdditiveClassifier *>(maxSc->clList[iSc]);
    Q_ASSERT(sc != 0);

    SampleList currPositiveSamples;
    foreach (QSharedPointer<Sample> posSample, positiveSamples)
    {
      float maxResult = 0.0f;
      maxSc->prepare(posSample->imagePool);
      maxSc->evaluate(&maxResult, x, y);

      float result = 0.0f;
      sc->evaluate(&result, x, y);

      if (result >= maxResult)
        currPositiveSamples.append(posSample);
    }

    if (currPositiveSamples.count() == 0)
    {
      ObjedConsole::printWarning(QString("No positive samples for training Sc%0").arg(iSc + 1));
      continue;
    }

    setSampleWeights(currPositiveSamples, negativeSamples, sc);
    QSharedPointer<objed::Classifier> bestWc = realAdaBoost(wcList, positiveSamples, negativeSamples, 
      QString("Training strong classifier iteration %0-%1").arg(iSc + 1).arg(sc->clList.size() + 1));
    sc->clList.push_back(bestWc->clone());
  }

}