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

#include <QSettings>

#include <objedutils/objedconsole.h>
#include <objedutils/objedconfig.h>
#include <objedutils/objedconf.h>
#include <objedutils/objedexp.h>
#include <objedutils/objedio.h>

#include <objed/src/cascadecl.h>
#include <objed/src/treecl.h>

#include "objedtrainutils.h"
#include "objedtraincli.h"
#include "datasetproc.h"
#include "trainscproc.h"

static QStringList getLeaveList(const QString &tempIniPath)
{
  if (tempIniPath.isEmpty())
    return QStringList();

  QSettings tempSettings(tempIniPath, QSettings::IniFormat);
  int count = tempSettings.beginReadArray("Leaves");
  
  QStringList leaveList;
  for (int i = 0; i < count; i++)
  {
    tempSettings.setArrayIndex(i);
    leaveList.append(tempSettings.value("label").toString());
  }

  tempSettings.endArray();
  return leaveList;
}

static void updateLeaveList(const QString &tempIniPath, const QStringList &leaveList)
{
  if (tempIniPath.isEmpty())
    return;

  QSettings tempSettings(tempIniPath, QSettings::IniFormat);
  tempSettings.beginWriteArray("Leaves", leaveList.count());

  for (int i = 0; i < leaveList.count(); i++)
  {
    tempSettings.setArrayIndex(i);
    tempSettings.setValue("label", leaveList[i]);
  }

  tempSettings.endArray();
}

static QSharedPointer<objed::Classifier> getClassifierByLabel(const objed::TreeClassifier *tree, const QString &label)
{
  const objed::TreeClassifier *currentTree = tree;
  QString currentLabel = label;

  do 
  {
    if (currentLabel.startsWith(QChar('l')) == true)
      currentTree = dynamic_cast<const objed::TreeClassifier *>(currentTree->leftCl);
    else if (currentLabel.startsWith(QChar('r')) == true)
      currentTree = dynamic_cast<const objed::TreeClassifier *>(currentTree->rightCl);
  } while (currentLabel.remove(0, 1).isEmpty() == false && currentTree != 0);

  if (currentTree != 0 && currentTree->centralCl != 0)
    return QSharedPointer<objed::Classifier>(currentTree->centralCl->clone());
  else
    return QSharedPointer<objed::Classifier>();
}

static void updateClassifierByLabel(objed::TreeClassifier *tree, const objed::Classifier *classifier, const QString &label)
{
  objed::TreeClassifier *currentTree = tree;
  QString currentLabel = label;

  do 
  {
    if (currentLabel.startsWith(QChar('l')) == true)
    {
      if (currentTree->leftCl == 0) 
        currentTree->leftCl = new objed::TreeClassifier(tree->width(), tree->height());
      currentTree = dynamic_cast<objed::TreeClassifier *>(currentTree->leftCl);
    }
    else if (currentLabel.startsWith(QChar('r')) == true)
    {
      if (currentTree->rightCl == 0) 
        currentTree->rightCl = new objed::TreeClassifier(tree->width(), tree->height());
      currentTree = dynamic_cast<objed::TreeClassifier *>(currentTree->rightCl);
    }
  } while (currentLabel.remove(0, 1).isEmpty() == false);

  if (currentTree->centralCl != 0)
    objed::Classifier::destroy(currentTree->centralCl);

  currentTree->centralCl = classifier->clone();
}

static QSharedPointer<ParityCascadeClassifier> buildParityCascade(const objed::TreeClassifier *tree, const QString &label)
{
  if (tree == 0)
    return QSharedPointer<ParityCascadeClassifier>();

  QSharedPointer<ParityCascadeClassifier> parityCascade(
    new ParityCascadeClassifier(tree->width(), tree->height()));

  const objed::TreeClassifier *currentTree = tree;
  QString currentLabel = label;
  int currentParity = +1;

  do
  {
    if (currentLabel.startsWith("Cr") == true)
    {
      parityCascade->parityList.push_back(+1);
      parityCascade->clList.push_back(currentTree->centralCl->clone());
      currentTree = dynamic_cast<const objed::TreeClassifier*>(currentTree->rightCl);
    }
    else if (currentLabel.startsWith("Cl") == true)
    {
      parityCascade->parityList.push_back(-1);
      parityCascade->clList.push_back(currentTree->centralCl->clone());
      currentTree = dynamic_cast<const objed::TreeClassifier*>(currentTree->leftCl);
    }
  } while (currentLabel.remove(0, 1).length() > 1);

  return parityCascade;
}

int ObjedTrainCLI::main(ObjedConfig *config)
{
  QString logPath = config->value("LogPath").toString();
  if (logPath.isEmpty() == false) ObjedConsole::setLogPath(logPath);

  QString classifierType = config->value("ClassifierType").toString();
  if (classifierType.toStdString() == objed::CascadeClassifier::typeStatic())
  {
    return trainCascadeClassifier(config);
  }
  else if (classifierType.toStdString() == objed::TreeClassifier::typeStatic())
  {
    return trainTreeClassifier(config);
  }

  ObjedConsole::printError(QString("Invalid classifier type '%0'").arg(classifierType));
  return -1;
}

int ObjedTrainCLI::trainCascadeClassifier(ObjedConfig *config)
{
  int classifierWidth = config->value("ClassifierWidth").toInt();
  int classifierHeight = config->value("ClassifierHeight").toInt();
  QString classifierPath = config->value("ClassifierPath").toString();

  int positiveSamplesThreshold = config->value("PositiveCountThreshold").toInt();
  int negativeSamplesThreshold = config->value("NegativeCountThreshold").toInt();

  int levelCountThreshold = config->value("LevelCount").toInt();
  int levelCount = 0;

  try
  {
    DatasetProcessor datasetProc(config);
    TrainScProcessor trainScProc(config);

    QSharedPointer<objed::CascadeClassifier> cascade;

    if (QFileInfo(classifierPath).exists() == true)
    {
      cascade = ObjedIO::loadClassifier(classifierPath).dynamicCast<objed::CascadeClassifier>();
      if (cascade.isNull() == true || cascade->type() != objed::CascadeClassifier::typeStatic())
        throw ObjedException(QString("Cannot load cascade classifier"));
      if (cascade->width() != classifierWidth || cascade->height() != classifierHeight)
        throw ObjedException(QString("Classifier has invalid size"));
    }
    else
    {
      cascade.reset(new objed::CascadeClassifier(classifierWidth, classifierHeight));
      if (cascade.isNull() == true || cascade->type() != objed::CascadeClassifier::typeStatic())
        throw ObjedException(QString("Cannot create cascade classifier"));
    }

    while (true)
    {
      QString label = QString("(%0)").arg(QString(cascade->clList.size() + 1, 'C'));
      ObjedConsole::printInfo(QString("Training '%0' node of cascade classifier").arg(label));

      SampleList positiveSamples = datasetProc.preparePositiveSamples(cascade.data());
      if (positiveSamples.count() <= positiveSamplesThreshold)
      {
        ObjedConsole::printInfo("Positive sample count is less than threshold value");
        break;
      }

      SampleList negativeSamples = datasetProc.prepareNegativeSamples(cascade.data());
      if (negativeSamples.count() <= negativeSamplesThreshold)
      {
        ObjedConsole::printInfo("Negative sample count is less than threshold value");
        break;
      }

      QSharedPointer<objed::Classifier> sc = trainScProc.trainSc(positiveSamples, negativeSamples);
      cascade->clList.push_back(sc->clone());

      ObjedIO::saveClassifier(cascade.data(), classifierPath);

      if (levelCountThreshold > 0 && ++levelCount >= levelCountThreshold)
      {
        ObjedConsole::printInfo("Required count of levels have been trained");
        break;
      }
    }
  }
  catch (ObjedException ex)
  {
    ObjedConsole::printError(ex.details());
  }

  return 0;
}

int ObjedTrainCLI::trainTreeClassifier(ObjedConfig *config)
{
  int classifierWidth = config->value("ClassifierWidth").toInt();
  int classifierHeight = config->value("ClassifierHeight").toInt();
  QString classifierPath = config->value("ClassifierPath").toString();

  int positiveSamplesThreshold = config->value("PositiveCountThreshold").toInt();
  int negativeSamplesThreshold = config->value("NegativeCountThreshold").toInt();

  int levelCountThreshold = config->value("LevelCount").toInt();
  int levelCount = 0;

  QString tempIniPath = config->value("TempIniPath").toString();
  QStringList leaveList = getLeaveList(tempIniPath);

  try
  {
    DatasetProcessor datasetProc(config);
    TrainScProcessor trainScProc(config);

    QSharedPointer<objed::TreeClassifier> tree;

    if (QFileInfo(classifierPath).exists() == true)
    {
      tree = ObjedIO::loadClassifier(classifierPath).dynamicCast<objed::TreeClassifier>();
      if (tree.isNull() == true || tree->type() != objed::TreeClassifier::typeStatic())
        throw ObjedException(QString("Cannot load tree classifier"));
      if (tree->width() != classifierWidth || tree->height() != classifierHeight)
        throw ObjedException(QString("Classifier has invalid size"));
    }
    else
    {
      tree.reset(new objed::TreeClassifier(classifierWidth, classifierHeight));
      if (tree.isNull() == true || tree->type() != objed::TreeClassifier::typeStatic())
        throw ObjedException(QString("Cannot create tree classifier"));
    }

    while (true)
    {   
      QString label = "C";
      bool anyScTrained = false;

      while (true)
      {
        try
        {
          while (getClassifierByLabel(tree.data(), label).isNull() == false)
            label.append("rC");

          if (leaveList.contains(label))
            throw ObjedException(QString("Branch %0 is leave").arg(label));

          ObjedConsole::printInfo(QString("Training '%0' node of tree classifier").arg(label));
          QSharedPointer<ParityCascadeClassifier> parityCascade = buildParityCascade(tree.data(), label);

          SampleList positiveSamples = datasetProc.preparePositiveSamples(parityCascade.data());
          if (positiveSamples.count() <= positiveSamplesThreshold)
          {
            leaveList.append(label);
            throw ObjedException("Positive sample count is less than threshold value");
          }

          SampleList negativeSamples = datasetProc.prepareNegativeSamples(parityCascade.data());
          if (negativeSamples.count() <= negativeSamplesThreshold)
          {
            leaveList.append(label);
            throw ObjedException("Negative sample count is less than threshold value");
          }

          QSharedPointer<objed::Classifier> sc = trainScProc.trainSc(positiveSamples, negativeSamples);
          updateClassifierByLabel(tree.data(), sc.data(), label);
          anyScTrained = true;
        }
        catch (ObjedException ex)
        {
          ObjedConsole::printInfo(ex.details());
        }

        ObjedIO::saveClassifier(tree.data(), classifierPath);
        updateLeaveList(tempIniPath, leaveList);
        
        if (label.lastIndexOf("rC") > 0) 
          label = label.left(label.lastIndexOf("rC")) + "lC";
        else
          break;
      }

      if (levelCountThreshold > 0 && ++levelCount >= levelCountThreshold)
      {
        ObjedConsole::printInfo("Required count of levels have been trained");
        break;
      }

      if (anyScTrained == false)
      {
        ObjedConsole::printInfo("Tree classifier has been trained");
        break;
      }
    }
  }
  catch (ObjedException ex)
  {
    ObjedConsole::printError(ex.details());
  }

  return 0;
}
