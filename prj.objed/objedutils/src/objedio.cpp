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

#include <objedutils/objedconf.h>
#include <objedutils/objedexp.h>
#include <objedutils/objedio.h>

#include <objed/objed.h>

QSharedPointer<objed::Classifier> ObjedIO::loadClassifier(const QString &classifierPath)
{
  objed::Classifier *classifier = objed::Classifier::create(classifierPath.toStdString());
  if (classifier == 0 || classifier->width() <= 0 || classifier->height() <= 0)
  {
    QString classifierName = QFileInfo(classifierPath).fileName();
    throw ObjedException(QString("Cannot load classifier %0").arg(classifierName));
  }

  return QSharedPointer<objed::Classifier>(classifier, objed::Classifier::destroy);
}

void ObjedIO::saveClassifier(const objed::Classifier *classifier, const QString &classifierPath)
{
  if (classifierPath.isEmpty() == true)
    throw ObjedException(QString("Invalid classifier path to save"));

  Json::StyledWriter dataWriter;
  Json::Value data = classifier->serialize();
  std::string strData = dataWriter.write(data);

  QFile classifierFile(classifierPath);
  if (classifierFile.open(QIODevice::WriteOnly) == false)
  {
    QString classifierName = QFileInfo(classifierPath).fileName();
    throw ObjedException(QString("Cannot open classifier file %0").arg(classifierName));
  }

  classifierFile.write(strData.c_str());
}

QSharedPointer<objed::Detector> ObjedIO::loadDetector(const QString &detectorPath)
{
  objed::Detector *detector = objed::Detector::create(detectorPath.toStdString());
  if (detector == 0)
  {
    QString classifierName = QFileInfo(detectorPath).fileName();
    throw ObjedException(QString("Cannot load detector %0").arg(classifierName));
  }

  return QSharedPointer<objed::Detector>(detector, objed::Detector::destroy);
}
