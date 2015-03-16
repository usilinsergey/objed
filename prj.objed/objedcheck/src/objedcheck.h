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
#ifndef OBJEDCHECK_H_INCLUDED
#define OBJEDCHECK_H_INCLUDED

#include <QSharedPointer>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QList>
#include <QDir>

#include "ui_objedcheck.h"

namespace objed
{
  class Detector;
}

class ImageViewDock;
class ObjedClusterer;

class ObjedCheck : public QMainWindow, public Ui::ObjedCheck
{
  Q_OBJECT

public:
  ObjedCheck(QWidget *parent = 0, Qt::WindowFlags flags = 0);
  virtual ~ObjedCheck();

private slots:
  void onOpenImageAction();
  void onOpenImageDirAction();
  void onOpenImageViewAction();
  void onChangeImageAction();
  void onRefreshAction();
  void onAboutAction();

private slots:
  void onChooseDetectorClicked();
  void onClearDetectorClicked();

private slots:
  void refreshTimer();
  void info(const QString &msg);
  void error(const QString &msg);

private slots:
  void updateDetectorSettings();
  void processImage(const QString &imageName);
  bool eventFilter(QObject *object, QEvent *event);

private:
  QList<ImageViewDock *> imageViewList;
  QSharedPointer<objed::Detector> detector;
  QElapsedTimer timer;
  QDir imageDir;
};

#endif  // OBJEDCHECK_H_INCLUDED