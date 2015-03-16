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

#include <QStringListModel>
#include <QSettings>
#include <QLayout>
#include <QLabel>
#include <QDir>

#include "opendialog.h"

ObjedMarkerOpenDialog::ObjedMarkerOpenDialog(QWidget *parent) : QFileDialog(parent)
{
  setWindowTitle("Choose Dataset Directory");
  setOption(QFileDialog::DontResolveSymlinks);
  setOption(QFileDialog::DontUseNativeDialog);
  setOption(QFileDialog::ShowDirsOnly);
  setFileMode(QFileDialog::Directory);

  idealMarkupNameLine = new QLineEdit(this);
  runMarkupNameLine = new QLineEdit(this);

  layout()->addWidget(new QLabel("Ideal Markup:", this));
  layout()->addWidget(idealMarkupNameLine);

  layout()->addWidget(new QWidget(this));

  layout()->addWidget(new QLabel("Run Markup:", this));
  layout()->addWidget(runMarkupNameLine);

  QSettings settings;
  if (settings.contains("OpenDialog/State"))
    this->restoreState(settings.value("OpenDialog/State").toByteArray());
  if (settings.contains("OpenDialog/LastDatasetPath"))
    selectFile(settings.value("OpenDialog/LastDatasetPath").toString());
  if (settings.contains("OpenDialog/LastIdealMarkupName"))
    idealMarkupNameLine->setText(settings.value("OpenDialog/LastIdealMarkupName").toString());
  if (settings.contains("OpenDialog/LastRunMarkupName"))
    runMarkupNameLine->setText(settings.value("OpenDialog/LastRunMarkupName").toString());

  completer = new QCompleter(QDir(datasetPath()).entryList(QDir::Dirs), this);
  idealMarkupNameLine->setCompleter(completer);
  runMarkupNameLine->setCompleter(completer);

  connect(this, SIGNAL(directoryEntered(const QString &)), SLOT(updateCompleter(const QString &)));
  connect(this, SIGNAL(accepted()), SLOT(saveSettings()));
}

ObjedMarkerOpenDialog::~ObjedMarkerOpenDialog()
{
  return;
}

QString ObjedMarkerOpenDialog::datasetPath() const
{
  return selectedFiles().join("");
}

QString ObjedMarkerOpenDialog::idealMarkupName() const
{
  return idealMarkupNameLine->text();
}

QString ObjedMarkerOpenDialog::runMarkupName() const
{
  return runMarkupNameLine->text();
}

void ObjedMarkerOpenDialog::updateCompleter(const QString &path)
{
  QStringListModel *model = qobject_cast<QStringListModel *>(completer->model());
  model->setStringList(QDir(path).entryList(QDir::Dirs));
}

void ObjedMarkerOpenDialog::saveSettings()
{
  QSettings settings;
  settings.setValue("OpenDialog/State", saveState());
  settings.setValue("OpenDialog/LastDatasetPath", datasetPath());
  settings.setValue("OpenDialog/LastIdealMarkupName", idealMarkupName());
  settings.setValue("OpenDialog/LastRunMarkupName", runMarkupName());
}

