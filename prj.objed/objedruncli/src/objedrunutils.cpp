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

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

#include <objedutils/objedconsole.h>
#include <objedutils/objedopenmp.h>
#include <objedutils/objedimage.h>
#include <objedutils/objedexp.h>
#include <objedutils/objedsys.h>
#include <objedutils/objedio.h>

#include "objedrunutils.h"

ObjedRunProcessor::ObjedRunProcessor(ObjedConfig *config) : saveRealMarkup(true), minPower(0), threshold(0.0)
{
  detector = ObjedIO::loadDetector(config->value("DetectorPath").toString());
  if (detector.isNull() == true) throw ObjedException("Cannot load detector");

  idealMarkupName = config->value("IdealMarkupName").toString();
  realMarkupName = QString("%0-%1").arg(config->value("IdealMarkupName").toString());
  realMarkupName = realMarkupName.arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
  saveRealMarkup = config->value("SaveRealMarkup").toBool();
  saveDebugInfo = config->value("SaveDebugInfo").toBool();

  minPower = config->value("MinimumPower").toInt();
  threshold = config->value("Threshold").toDouble();
}

ObjedRunProcessor::~ObjedRunProcessor()
{
  return;
}

void ObjedRunProcessor::printTotalStatistics()
{
  ObjedConsole::printInfo(QString("Total statistics: TP: %0 FP: %1 FN: %2 IC: %3").
    arg(totalResult.truePositive).arg(totalResult.falsePositive).
    arg(totalResult.falseNegative).arg(totalResult.imageCount));
}

void ObjedRunProcessor::processDataset(const QString &datasetPath)
{
  QDir datasetDir(datasetPath);
  QDir realMarkupDir(datasetPath + "/" + realMarkupName);
  QDir idealMarkupDir(datasetPath + "/" + idealMarkupName);

  if (saveRealMarkup == true || saveDebugInfo == true)
    QDir().mkpath(realMarkupDir.absolutePath());

  QStringList imageFilters = ObjedSys::imageFilters();
  QStringList imageNameList = QDir(datasetPath).entryList(imageFilters);

  ObjedCompare::Result result;
  QVector<ObjedCompare::Result> resultList(ObjedOpenMP::maxThreadCount());
  ObjedOpenMP::DetectorList detectorList = ObjedOpenMP::multiplyDetector(detector.data());

  int processedImageCount = 0;

  #pragma omp parallel for
  for (int i = 0; i < imageNameList.count(); i++)
  {
    ObjedConsole::printProgress(QString("Processing dataset %0").
      arg(datasetDir.dirName()), 100 * processedImageCount / imageNameList.count());

    #pragma omp atomic
    processedImageCount++;

    try
    {
      QString imageName = imageNameList[i];
      QString imagePath = datasetDir.absoluteFilePath(imageName);
      QSharedPointer<ObjedImage> image = ObjedImage::create(imagePath);

      objed::DebugInfo *debugInfo = nullptr;
      if (saveDebugInfo) debugInfo = new objed::DebugInfo();

      objed::Detector *detector = detectorList[ObjedOpenMP::threadId()].data();
      objed::DetectionList detectionList = detector->detect(image->image(), debugInfo);

      ObjedMarkup idealMarkup(idealMarkupDir.absoluteFilePath(imageName + ".json"));
      ObjedMarkup realMarkup(realMarkupDir.absoluteFilePath(imageName + ".json"), saveRealMarkup);

      for (size_t i = 0; i < detectionList.size(); i++)
      {
        if (detectionList[i].power >= minPower) 
        {
          QRect realObject(detectionList[i].x, detectionList[i].y,
            detectionList[i].width, detectionList[i].height);
          realMarkup.appendObject(realObject);
        }
      }

      if (saveRealMarkup == true)
        realMarkup.save();

      if (saveDebugInfo)
      {
        Json::Value root;
        root["evaluation_count"] = debugInfo->int_data[objed::Detector::DBG_EVALUATION_COUNT];
        root["min_sc_count"] = debugInfo->int_data[objed::Detector::DBG_MIN_SC_COUNT];
        root["max_sc_count"] = debugInfo->int_data[objed::Detector::DBG_MAX_SC_COUNT];
        root["total_sc_count"] = debugInfo->int_data[objed::Detector::DBG_TOTAL_SC_COUNT];

        QFile debugInfoFile(realMarkupDir.absoluteFilePath(imageName + ".dbg.json"));
        if (debugInfoFile.open(QIODevice::WriteOnly))
        {
          Json::FastWriter writer;
          debugInfoFile.write(writer.write(root).c_str());
        }
      }

      ObjedCompare::Result &result = resultList[ObjedOpenMP::threadId()];
      result += ObjedCompare::compare(realMarkup, idealMarkup, threshold);
    }
    catch (ObjedException ex)
    {
      ObjedConsole::printError(ex.details());
    }
  }

  for (int i = 0; i < resultList.count(); i++)
    result += resultList[i];

  ObjedConsole::printProgress(QString("Dataset %0 result: TP: %1 FP: %2 FN: %3 IC: %4").
    arg(datasetDir.dirName()).arg(result.truePositive).arg(result.falsePositive).
    arg(result.falseNegative).arg(result.imageCount), 100);
  
  totalResult += result;

  if (saveDebugInfo)
  {
    ObjedConsole::printInfo("Compile total CSV debug file for dataset");
    QStringList dbgFileNameList = realMarkupDir.entryList(QStringList("*.dbg.json"));

    QFile dbgFile(realMarkupDir.absoluteFilePath("dbg.csv"));
    if (dbgFile.open(QIODevice::WriteOnly) == false)
    {
      ObjedConsole::printError("Cannot open dbg.csv");
    }
    else
    {
      QTextStream dbgFileStream(&dbgFile);
      dbgFileStream << "evaluation_count;min_sc_count;max_sc_count;total_sc_count;" << endl;

      foreach(QString dbgFileName, dbgFileNameList)
      {
        QFile dbgJsonFile(realMarkupDir.absoluteFilePath(dbgFileName));
        dbgJsonFile.open(QIODevice::ReadOnly);

        QJsonObject dbgJsonRoot = QJsonDocument::fromJson(dbgJsonFile.readAll()).object();
        dbgFileStream << dbgJsonRoot["evaluation_count"].toInt() << ";";
        dbgFileStream << dbgJsonRoot["min_sc_count"].toInt() << ";";
        dbgFileStream << dbgJsonRoot["max_sc_count"].toInt() << ";";
        dbgFileStream << dbgJsonRoot["total_sc_count"].toInt() << ";";
        dbgFileStream << endl;
      }
    }
  }
}
