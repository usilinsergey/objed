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

#pragma once
#ifndef OBJEDTRAINUTILS_H_INCLUDED
#define OBJEDTRAINUTILS_H_INCLUDED

#include <QSharedPointer>
#include <QStringList>
#include <QList>
#include <QSize>

#include <objedutils/objedimage.h>
#include <objedutils/objedsys.h>

#include <objed/objed.h>

class Sample;
typedef QList<QSharedPointer<Sample> > SampleList;
typedef QList<QSharedPointer<objed::Classifier> > WcList;

class Sample
{
public:
  Sample(const ObjedImage *image, const QString &path = QString());
  virtual ~Sample();

public:
  objed::ImagePool *imagePool;
  QString sourceImagePath;
  double weight;

private:
  Sample(const Sample &);
  Sample &operator=(const Sample &);
};

class WcMaker
{
public:
  static WcList make(const QString &wcLine, const QSize &size);
  static QStringList help();

private:
  static WcList makeHaar1StumpWc(const QVariantMap &wcParams, const QSize &size);
  static WcList makeHaar2StumpWc(const QVariantMap &wcParams, const QSize &size);
  static WcList makeHaar3StumpWc(const QVariantMap &wcParams, const QSize &size);
  static WcList makeHaar1PwWc(const QVariantMap &wcParams, const QSize &size);
  static WcList makeHaar2PwWc(const QVariantMap &wcParams, const QSize &size);
  static WcList makeHaar3PwWc(const QVariantMap &wcParams, const QSize &size);
};

class ParityCascadeClassifier : public objed::Classifier
{
public:
  ParityCascadeClassifier(int width, int height);
  virtual ~ParityCascadeClassifier();

public:
  virtual int width() const;
  virtual int height() const;
  virtual bool prepare(objed::ImagePool *imagePool);
  virtual bool evaluate(float *result, int x, int y, objed::DebugInfo *debugInfo) const;
  virtual Json::Value serialize() const;
  virtual std::string type() const;
  virtual objed::Classifier * clone() const;

public:
  int clWidth, clHeight;
  std::vector<int> parityList;
  std::vector<objed::Classifier *> clList;
};

#endif  // OBJEDTRAINUTILS_H_INCLUDED
