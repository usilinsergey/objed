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
#ifndef OBJEDCUTUTILS_H_INCLUDED
#define OBJEDCUTUTILS_H_INCLUDED

#include <QObject>
#include <QList>
#include <QSize>
#include <QDir>

#include <objedutils/objedimage.h>
#include <objedutils/objedconfig.h>
#include <objedutils/objedmarkup.h>

#include <objed/objedutils.h>
#include <objed/objed.h>

class ObjedCutProcessor
{
public:
  virtual void process(ObjedImage *image, const ObjedMarkup *markup, const QString &imageName) = 0;
  virtual void summarize() = 0;
  virtual ~ObjedCutProcessor() {}
};

class ObjedCutCutting : public ObjedCutProcessor
{
  Q_DISABLE_COPY(ObjedCutCutting)

public:
  ObjedCutCutting(ObjedConfig *config);
  virtual ~ObjedCutCutting();

public:
  virtual void process(ObjedImage *image, const ObjedMarkup *markup, const QString &imageName);
  virtual void summarize();

private:
  int minObjectWidth, maxObjectWidth;
  int minObjectHeight, maxObjectHeight;
  double leftMargin, rightMargin;
  double topMargin, bottomMargin;
  QDir positiveDataset, negativeDataset;
  QString outputFormat;

private:
  int objectCount;
  int negativeCount;
};

class ObjedCutStatistics : public ObjedCutProcessor
{
  Q_DISABLE_COPY(ObjedCutStatistics)

public:
  ObjedCutStatistics(ObjedConfig *config);
  virtual ~ObjedCutStatistics();

public:
  virtual void process(ObjedImage *image, const ObjedMarkup *markup, const QString &imageName);
  virtual void summarize();

private:
  QList<QSize> objectSizeList;
  int starredImageCount;
  int totalImageCount;
};

#endif  // OBJEDCUTUTILS_H_INCLUDED