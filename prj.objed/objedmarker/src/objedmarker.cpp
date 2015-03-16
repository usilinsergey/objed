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

#include <QProgressDialog>
#include <QFontMetrics>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>

#include <objedutils/objedcompare.h>
#include <objedutils/objedmarkup.h>
#include <objedutils/objedimage.h>
#include <objedutils/objedsys.h>
#include <objedutils/objedexp.h>

#include "objedmarker.h"
#include "opendialog.h"

static void updateStatusBarLabel(QLabel *label, const QString &prefix, const QString &text)
{
  QString fullText = QString("<b>%0: </b>%1").arg(prefix).arg(text);
  QFont baseFont = label->font(), boldFont(baseFont.family(), baseFont.pointSize(), QFont::Bold);
  int minLabelWidth = QFontMetrics(boldFont).width(prefix) + QFontMetrics(baseFont).width(text) + 20;

  label->setMinimumWidth(minLabelWidth);
  label->setText(fullText);
}

ObjedMarker::ObjedMarker(QWidget *parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) :
QMainWindow(parent, flags)
{
  setupUi(this);

  QSettings settings;
  if (settings.contains("MainWindow/Geometry"))
    setGeometry(settings.value("MainWindow/Geometry").toRect());
  if (settings.contains("MainWindow/State"))
    restoreState(settings.value("MainWindow/State").toByteArray());

  gammaBox->setValue(settings.value("AdjustImage/Gamma", 1.0).toDouble());

  compareThresholdBox->setValue(settings.value("Compare/Threshold").toDouble());

  showAllDifferencesAction->setChecked(settings.value("Compare/ShowAllDifferences").toBool());

  setupMarkupEditor();
  setupMenuActions();
  setupStatusBar();
}

ObjedMarker::~ObjedMarker()
{
  QSettings settings;
  settings.setValue("MainWindow/Geometry", geometry());
  settings.setValue("MainWindow/State", saveState());

  settings.setValue("AdjustImage/Gamma", gammaBox->value());

  settings.setValue("Compare/Threshold", compareThresholdBox->value());

  settings.setValue("Compare/ShowAllDifferences", showAllDifferencesAction->isChecked());
}

void ObjedMarker::onOpenDatasetAction()
{
  ObjedMarkerOpenDialog openDialog;
  if (openDialog.exec() == QDialog::Rejected)
    return;

  imageList->clear();
  datasetDirLabel->clear();
  idealMarkupNameLabel->clear();
  runMarkupNameLabel->clear();
  QApplication::processEvents();

  updateStatusBarLabel(datasetDirLabel, tr("Dataset"), openDialog.datasetPath());
  updateStatusBarLabel(idealMarkupNameLabel, tr("Ideal"), openDialog.idealMarkupName());
  updateStatusBarLabel(runMarkupNameLabel, tr("Run"), openDialog.runMarkupName());

  datasetDir.setPath(openDialog.datasetPath());
  idealMarkupName = openDialog.idealMarkupName();
  runMarkupName = openDialog.runMarkupName();

  QStringList imageNameFilters = ObjedSys::imageFilters();
  QStringList imageNameList = datasetDir.entryList(imageNameFilters);

  imageList->addItems(imageNameList);

  nextImageAction->setEnabled(imageNameList.count() > 0);
  previousImageAction->setEnabled(imageNameList.count() > 0);
  firstImageAction->setEnabled(imageNameList.count() > 0);
  lastImageAction->setEnabled(imageNameList.count() > 0);
  redrawImageAction->setEnabled(imageNameList.count() > 0);

  nextDifferenceAction->setEnabled(imageNameList.count() > 0 && !idealMarkupName.isEmpty() && !runMarkupName.isEmpty());
  previousDifferenceAction->setEnabled(imageNameList.count() > 0 && !idealMarkupName.isEmpty() && !runMarkupName.isEmpty());
  showAllDifferencesAction->setEnabled(imageNameList.count() > 0 && !idealMarkupName.isEmpty() && !runMarkupName.isEmpty());

  if (imageNameList.count() > 0)
    imageList->setCurrentRow(0);
}

void ObjedMarker::onGoImageAction()
{
  if (imageList->count() <= 0)
    return;

  QObject *sender = QObject::sender();
  int index = imageList->currentRow();

  index = sender == firstImageAction ? 0 : sender == previousImageAction ? 
    index - 1 : sender == nextImageAction ? index + 1 : imageList->count() - 1;
  imageList->setCurrentRow(qBound(0, index, imageList->count() - 1));
}

void ObjedMarker::onGoDifferenceAction()
{
  if (imageList->count() <= 0)
    return;

  if (idealMarkupName.isEmpty() || runMarkupName.isEmpty())
    return;

  int startIndex = imageList->currentRow();
  int deltaIndex = QObject::sender() == nextDifferenceAction ? +1 : -1;

  QDir idealMarkupDir(datasetDir.absoluteFilePath(idealMarkupName));
  QDir realMarkupDir(datasetDir.absoluteFilePath(runMarkupName));

  QProgressDialog progressDialog(tr("Finding difference between ideal and real markups..."), QString(), 0, 0, this);
  for (int i = startIndex + deltaIndex; i >= 0 && i < imageList->count(); i += deltaIndex)
  {
    QApplication::processEvents();
    if (progressDialog.wasCanceled())
      return;
    
    QString markupName = imageList->item(i)->text() + ".json";
    const ObjedMarkup idealMarkup(idealMarkupDir.absoluteFilePath(markupName));
    const ObjedMarkup realMarkup(realMarkupDir.absoluteFilePath(markupName));

    if (showAllDifferencesAction->isChecked() || idealMarkup.isStarred())
    {
      if (idealMarkup.objects().count() != realMarkup.objects().count())
      {
        imageList->setCurrentRow(i);
        return;
      }
    }

    double threshold = compareThresholdBox->value();
    ObjedCompare::Result results = ObjedCompare::compare(realMarkup, idealMarkup, threshold);
    if (results.falsePositive != 0 || results.falseNegative != 0)
    {
      imageList->setCurrentRow(i);
      return;
    }
  }

  QMessageBox::information(this, tr("Go Difference"), tr("Reached the end of the dataset"));
}

void ObjedMarker::onRedrawImageAction()
{
  onImageChanged();
}

void ObjedMarker::onAboutAction()
{
  QString title = tr("About ObjedMarker");
  QString text = tr("This application is designed to markup datasets");

  QString fullText = "<h3>%0 (ver. %1)</h3<p>%2</p>";
  fullText = fullText.arg(QApplication::applicationName());
  fullText = fullText.arg(QApplication::applicationVersion());
  fullText = fullText.arg(text.simplified());
  QMessageBox::about(this, title, fullText);
}

void ObjedMarker::onImageChanged()
{
  graphicsView->scene()->clear();
  if (imageList->currentItem() == 0)
    return;

  try
  {
    QString imageName = imageList->currentItem()->text();
    QSharedPointer<ObjedImage> image = ObjedImage::create(datasetDir.absoluteFilePath(imageName));

    image->gammaCorrection(gammaBox->value());

    QString idealPath = idealMarkupName.isEmpty() ? QString() : 
      datasetDir.absoluteFilePath(idealMarkupName + "/" + imageName + ".json");
    QString runPath = runMarkupName.isEmpty() ? QString() : 
      datasetDir.absoluteFilePath(runMarkupName + "/" + imageName + ".json");

    markupEditor->update(image->toQPixmap(), idealPath, runPath);
    graphicsView->fitInView(graphicsView->sceneRect(), Qt::KeepAspectRatio);
  }
  catch (ObjedException ex)
  {
    statusBar()->showMessage(ex.details(), 3000);
  }
}

void ObjedMarker::onGammaChanged()
{
  if (QObject::sender() == gammaBox)
    gammaSlider->setValue(qRound(gammaBox->value() * 10));
  else if (QObject::sender() == gammaSlider)
    gammaBox->setValue(gammaSlider->value() / 10.0);
}

void ObjedMarker::onCompareThresholdChanged()
{
  if (QObject::sender() == compareThresholdBox)
    compareThresholdSlider->setValue(qRound(compareThresholdBox->value() * 10));
  else if (QObject::sender() == compareThresholdSlider)
    compareThresholdBox->setValue(compareThresholdSlider->value() / 10.0);
}

void ObjedMarker::setupMarkupEditor()
{
  graphicsView->setScene(new QGraphicsScene(this));
  markupEditor = new ObjedMarkupEditor(graphicsView->scene(), this);

  editorInfoLayout->setWidget(0, QFormLayout::FieldRole, markupEditor->starImageInfo());
  editorInfoLayout->setWidget(1, QFormLayout::FieldRole, markupEditor->lockStarStateInfo());
  editorInfoLayout->setWidget(2, QFormLayout::FieldRole, markupEditor->selectedObjectInfo());
}

void ObjedMarker::setupMenuActions()
{
  viewMenu->addAction(editorInfoDock->toggleViewAction());
  viewMenu->addAction(adjustImageDock->toggleViewAction());
  viewMenu->addAction(compareDock->toggleViewAction());
  viewMenu->addAction(imageListDock->toggleViewAction());
  
  openDatasetAction->setShortcut(QKeySequence::Open);
  exitAction->setShortcut(QKeySequence::Quit);

  editMenu->addAction(markupEditor->undoAction());
  editMenu->addAction(markupEditor->redoAction());
  editMenu->addSeparator();
  editMenu->addAction(markupEditor->increaseObjectSizeAction());
  editMenu->addAction(markupEditor->decreaseObjectSizeAction());
  editMenu->addSeparator();
  editMenu->addAction(markupEditor->starImageAction());
  editMenu->addAction(markupEditor->lockStarStateAction());

  firstImageAction->setShortcut(QKeySequence("Ctrl+A"));
  lastImageAction->setShortcut(QKeySequence("Ctrl+D"));
  previousImageAction->setShortcut(QKeySequence("A"));
  nextImageAction->setShortcut(QKeySequence("D"));
  previousDifferenceAction->setShortcut(QKeySequence("Shift+A"));
  nextDifferenceAction->setShortcut(QKeySequence("Shift+D"));

  optionsMenu->addAction(markupEditor->objectSizePropertiesAction());
  optionsMenu->addAction(markupEditor->editorPropertiesAction());

  redrawImageAction->setShortcut(QKeySequence::Refresh);
}

void ObjedMarker::setupStatusBar()
{
  datasetDirLabel = new QLabel(this); datasetDirLabel->setAlignment(Qt::AlignHCenter);
  datasetDirLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  updateStatusBarLabel(datasetDirLabel, tr("Dataset"), QString());

  idealMarkupNameLabel = new QLabel(this); idealMarkupNameLabel->setAlignment(Qt::AlignHCenter);
  idealMarkupNameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  updateStatusBarLabel(idealMarkupNameLabel, tr("Ideal"), QString());

  runMarkupNameLabel = new QLabel(this); runMarkupNameLabel->setAlignment(Qt::AlignHCenter);
  runMarkupNameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  updateStatusBarLabel(runMarkupNameLabel, tr("Run"), QString());

  statusBar()->addPermanentWidget(datasetDirLabel);
  statusBar()->addPermanentWidget(idealMarkupNameLabel);
  statusBar()->addPermanentWidget(runMarkupNameLabel);
}
