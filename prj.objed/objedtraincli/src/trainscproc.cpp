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
#include <objedutils/objedconfig.h>
#include <objedutils/objedconf.h>
#include <objedutils/objedexp.h>
#include <objedutils/objedio.h>

#include <objed/objedutils.h>
#include <objed/objed.h>

#include <objed/src/additivecl.h>
#include <objed/src/linearcl.h>
#include <objed/src/maxcl.h>

#include "trainscproc.h"

static int calculateDetectionCount(objed::Classifier *classifier, SampleList &sampleList)
{
  if (classifier == 0)
    return 0;

  long long count = 0;
  int clWd = classifier->width();
  int clHt = classifier->height();

  for (int i = 0; i < sampleList.count(); i++)
  {
    float result = objed::epsilon;
    classifier->prepare(sampleList[i]->imagePool);
    classifier->evaluate(&result, clWd / 2, clHt / 2);
    count += result > 0.0;
  }

  return count;
}

TrainScProcessor::TrainScProcessor(ObjedConfig *config) :
classifierWidth(0), classifierHeight(0), falseNegativeRate(0.0), falsePositiveRate(0.0), wcCountThreshold(0), weightShift(0.5)
{
  classifierWidth = config->value("ClassifierWidth").toInt();
  classifierHeight = config->value("ClassifierHeight").toInt();
    
  stoppingCriterion = config->value("StoppingCriterion").toString();
  falseNegativeRate = config->value("FalseNegativeRate").toDouble();
  falsePositiveRate = config->value("FalsePositiveRate").toDouble();
  if (falseNegativeRate < 0.0 || falseNegativeRate > 1.0)
    throw ObjedException("Invalid false negative rate");
  if (falsePositiveRate < 0.0 || falsePositiveRate > 1.0)
    throw ObjedException("Invalid false positive rate");
  
  wcCountThreshold = config->value("WcCount").toInt();
  if (wcCountThreshold < 0)
    throw ObjedException("Invalid weak classifier count");

  weightShift = config->value("WeightShift").toDouble();
  if (weightShift <= 0 || weightShift >= 1.0)
    throw ObjedException("Invalid WeightShift value");

  method = config->value("Method").toString();
  methodParams = config->value("MethodParams").toString();

  QVariantList wcLineList = config->listValue("WcLineList");
  for (int i = 0; i < wcLineList.count(); i++)
  {
    QString wcLine = wcLineList[i].toString();
    wcList.append(WcMaker::make(wcLine, QSize(classifierWidth, classifierHeight)));
  }

  if (wcList.isEmpty() == true) 
    throw ObjedException("There are no weak classifiers");
}

TrainScProcessor::~TrainScProcessor()
{
  return;
}

QSharedPointer<objed::Classifier> TrainScProcessor::trainSc(SampleList &positiveSamples, SampleList &negativeSamples)
{
  for (int i = 0; i < wcList.count(); i++)
  {
    if (i % 100 == 0)
    {
      int progress = 100 * i / wcList.count();
      ObjedConsole::printProgress("Preparing weak classifiers for training", progress);
    }

    for (int j = 0; j < positiveSamples.count(); j++)
      wcList[i]->prepare(positiveSamples[j]->imagePool);
    for (int j = 0; j < negativeSamples.count(); j++)
      wcList[i]->prepare(negativeSamples[j]->imagePool);
  }

  ObjedConsole::printProgress("Weak classifiers have been prepared", 100);

  objed::Classifier *classifier = 0;
  int iterationCount = 0;
  
  while (true)
  {
    if (method.toLower() == "realadaboost")
      trainScRealAdaBoost(classifier, positiveSamples, negativeSamples);
    else if (method.toLower() == "realadaboostmax")
      trainScRealAdaBoostMax(classifier, positiveSamples, negativeSamples);
    else
      throw ObjedException("Unknown training method");
    
    int currFalsePositiveCount = calculateDetectionCount(classifier, negativeSamples);
    int currFalseNegativeCount = positiveSamples.count() - calculateDetectionCount(classifier, positiveSamples);
    double currFalsePositiveRate = static_cast<double>(currFalsePositiveCount) / negativeSamples.count();
    double currFalseNegativeRate = static_cast<double>(currFalseNegativeCount) / positiveSamples.count();

    ObjedConsole::printInfo(QString("Current strong classifier quality: %0/%1").
      arg(currFalseNegativeRate, 0, 'f').arg(currFalsePositiveRate, 0, 'f'));

    bool countCriterion = wcCountThreshold > 0 && (++iterationCount) >= wcCountThreshold;
    bool rateCriterion = (currFalseNegativeRate <= (falseNegativeRate + std::numeric_limits<double>::epsilon())) &&
                         (currFalsePositiveRate <= (falsePositiveRate + std::numeric_limits<double>::epsilon()));
    bool totalCriterion = countCriterion || rateCriterion;

    if (stoppingCriterion.toLower() == "count")
      totalCriterion = countCriterion;
    else if (stoppingCriterion.toLower() == "rate")
      totalCriterion = rateCriterion;

    if (totalCriterion == true)
      break;
  }

  return QSharedPointer<objed::Classifier>(classifier, objed::Classifier::destroy);
}

void TrainScProcessor::resetSampleWeights(SampleList &positiveSamples, SampleList &negativeSamples)
{
  foreach (QSharedPointer<Sample> sample, positiveSamples)
    sample->weight = weightShift / positiveSamples.count();

  foreach (QSharedPointer<Sample> sample, negativeSamples)
    sample->weight = (1.0 - weightShift) / negativeSamples.count();
}

void TrainScProcessor::normalizeSampleWeights(SampleList &positiveSamples, SampleList &negativeSamples)
{
  double weightSum = 0.0;
  for (int i = 0; i < positiveSamples.count(); i++)
    weightSum += positiveSamples[i]->weight;
  for (int i = 0; i < negativeSamples.count(); i++)
    weightSum += negativeSamples[i]->weight;

  Q_ASSERT(weightSum > 0.0);
  for (int i = 0; i < positiveSamples.count(); i++)
    positiveSamples[i]->weight /= weightSum;
  for (int i = 0; i < negativeSamples.count(); i++)
    negativeSamples[i]->weight /= weightSum;
}

void TrainScProcessor::setSampleWeights(SampleList &positiveSamples, SampleList &negativeSamples, objed::Classifier *classifier)
{
  resetSampleWeights(positiveSamples, negativeSamples);
  if (classifier == 0)
    return;

  const int x = classifier->width() / 2;
  const int y = classifier->height() / 2;

  for (int i = 0; i < positiveSamples.count(); i++)
  {
    float result = 0.0f;
    classifier->prepare(positiveSamples[i]->imagePool);
    classifier->evaluate(&result, x, y);
    positiveSamples[i]->weight *= std::exp(-result);
  }

  for (int i = 0; i < negativeSamples.count(); i++)
  {
    float result = 0.0f;
    classifier->prepare(negativeSamples[i]->imagePool);
    classifier->evaluate(&result, x, y);
    negativeSamples[i]->weight *= std::exp(result);
  }

  normalizeSampleWeights(positiveSamples, negativeSamples);
}

float TrainScProcessor::calculateError(SampleList &positiveSamples, SampleList &negativeSamples, objed::Classifier *classifier)
{
  if (classifier == 0)
    return 0;

  resetSampleWeights(positiveSamples, negativeSamples);
  int x = classifier->width() / 2, y = classifier->height() / 2;
  float error = 0.0f;

  foreach (QSharedPointer<Sample> posSample, positiveSamples)
  {
    float result = 0.0f;
    classifier->prepare(posSample->imagePool);
    classifier->evaluate(&result, x, y);
    error += (result <= 0.0f) * posSample->weight;
  }

  foreach (QSharedPointer<Sample> negSample, negativeSamples)
  {
    float result = 0.0f;
    classifier->prepare(negSample->imagePool);
    classifier->evaluate(&result, x, y);
    error += (result <= 0.0f) * negSample->weight;
  }

  return error;
}

int TrainScProcessor::calculateErrorCount(SampleList &positiveSamples, SampleList &negativeSamples, objed::Classifier *classifier)
{
  if (classifier == 0)
    return 0;

  int x = classifier->width() / 2, y = classifier->height() / 2;
  int errorCount = 0;

  foreach (QSharedPointer<Sample> posSample, positiveSamples)
  {
    float result = 0.0f;
    classifier->prepare(posSample->imagePool);
    classifier->evaluate(&result, x, y);
    errorCount += (result <= 0.0f);
  }

  foreach (QSharedPointer<Sample> negSample, negativeSamples)
  {
    float result = 0.0f;
    classifier->prepare(negSample->imagePool);
    classifier->evaluate(&result, x, y);
    errorCount += (result <= 0.0f);
  }

  return errorCount;
}
