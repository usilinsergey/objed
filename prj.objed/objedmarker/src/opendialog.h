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
#ifndef OPENDIALOG_H_INCLUDED
#define OPENDIALOG_H_INCLUDED

#include <QFileDialog>
#include <QCompleter>
#include <QLineEdit>

class ObjedMarkerOpenDialog : public QFileDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(ObjedMarkerOpenDialog)

public:
  ObjedMarkerOpenDialog(QWidget *parent = 0);
  virtual ~ObjedMarkerOpenDialog();

public slots:
  QString datasetPath() const;
  QString idealMarkupName() const;
  QString runMarkupName() const;

private slots:
  void updateCompleter(const QString &path);
  void saveSettings();

private:
  bool dialogAdjusted;
  QLineEdit *idealMarkupNameLine;
  QLineEdit *runMarkupNameLine;
  QCompleter *completer;
};

#endif  // OPENDIALOG_H_INCLUDED