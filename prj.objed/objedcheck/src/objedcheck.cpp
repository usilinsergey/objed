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

#include <QtConcurrent/QtConcurrent>

#include <QGraphicsTextItem>
#include <QProgressDialog>
#include <QGraphicsScene>
#include <QProgressBar>
#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QSettings>
#include <QFuture>
#include <QEvent>

#include <objedutils/objedimage.h>
#include <objedutils/objedsys.h>
#include <objedutils/objedexp.h>
#include <objedutils/objedio.h>

#include <objed/objed.h>

#include "imageviewdock.h"
#include "objedcheck.h"

ObjedCheck::ObjedCheck(QWidget *parent /*= 0*/, Qt::WindowFlags flags /*= 0*/) :
QMainWindow(parent, flags)
{
  setupUi(this);
  installEventFilter(this);

  //////////////////////////////////////////////////////////////////////////
  // Setup MenuBar
  imageListDock->toggleViewAction()->setShortcut(QString("Alt+1"));
  viewMenu->addAction(imageListDock->toggleViewAction());

  propertiesDock->toggleViewAction()->setShortcut(QString("Alt+2"));
  viewMenu->addAction(propertiesDock->toggleViewAction());

  consoleDock->toggleViewAction()->setShortcut(QString("Alt+3"));
  viewMenu->addAction(consoleDock->toggleViewAction());

  //////////////////////////////////////////////////////////////////////////
  // Setup Preview
  preview->setScene(new QGraphicsScene(preview));

  //////////////////////////////////////////////////////////////////////////
  // Load Settings
  QSettings settings;
  if (settings.contains("MainWindow/Geometry")) setGeometry(settings.value("MainWindow/Geometry").toRect());
  if (settings.contains("MainWindow/State")) restoreState(settings.value("MainWindow/State").toByteArray());
  detectorLine->setText(settings.value("Properties/Detector").toString());
  minPowerBox->setValue(settings.value("Properties/MinPower").toInt());

  QMetaObject::invokeMethod(this, "updateDetectorSettings", Qt::QueuedConnection);
}

ObjedCheck::~ObjedCheck()
{
  QSettings settings;
  settings.setValue("MainWindow/Geometry", geometry());
  settings.setValue("MainWindow/State", saveState());
  settings.setValue("Properties/Detector", detectorLine->text());
  settings.setValue("Properties/MinPower",  minPowerBox->value());
}

void ObjedCheck::onOpenImageAction()
{
  QSettings settings;
  QString imagePath = settings.value("LastImagePath").toString();
  QString imageDirPath = QFileInfo(imagePath).absoluteDir().absolutePath();
  QString filter = QString("Images (%0)").arg( ObjedSys::imageFilters().join(' '));
  QFileDialog::Options options = QFileDialog::DontUseNativeDialog | QFileDialog::DontResolveSymlinks;
  imagePath = QFileDialog::getOpenFileName(this, QString(), imageDirPath, filter, 0, options);
  if (imagePath.isEmpty() == true)
    return;

  settings.setValue("LastImagePath", imagePath);
  imageDir.setPath(QFileInfo(imagePath).absoluteDir().path());

  this->preview->scene()->clear();
  this->imageList->clear();

  this->imageList->addItem(QFileInfo(imagePath).fileName());
  if (this->imageList->count() > 0) 
    this->imageList->setCurrentRow(0);
}

void ObjedCheck::onOpenImageDirAction()
{
  QSettings settings;
  QString imageDirPath = settings.value("LastImageDirPath").toString();
  QFileDialog::Options options = QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly;
  imageDirPath = QFileDialog::getExistingDirectory(this, QString(), imageDirPath, options);
  if (imageDirPath.isEmpty() == true)
    return;

  settings.setValue("LastImageDirPath", imageDirPath);
  imageDir.setPath(imageDirPath);

  this->preview->scene()->clear();
  this->imageList->clear();

  this->imageList->addItems(imageDir.entryList(ObjedSys::imageFilters()));
  if (this->imageList->count() > 0) 
    this->imageList->setCurrentRow(0);
}

void ObjedCheck::onOpenImageViewAction()
{
  QString title = "Open Image View";
  QString label = "Please, enter the image id:";
  QString imageIdLine = QInputDialog::getText(this, title, label, QLineEdit::Normal);
  if (imageIdLine.isEmpty() == true)
    return;

  ImageViewDock *imageViewDock = new ImageViewDock(this);
  imageViewDock->setImageView(imageIdLine);

  addDockWidget(Qt::LeftDockWidgetArea, imageViewDock);
  imageViewList.append(imageViewDock);

  QApplication::processEvents();
  imageViewDock->setFloating(true);
}

void ObjedCheck::onChangeImageAction()
{
  if (this->imageList->count() <= 0)
    return;

  QObject *sender = QObject::sender();
  int index = this->imageList->currentRow();

  index = sender == firstImageAction ? 0 : sender == previousImageAction ? 
    index - 1 : sender == nextImageAction ? index + 1 : imageList->count() - 1;
  this->imageList->setCurrentRow(qBound(0, index, imageList->count() - 1));
}

void ObjedCheck::onRefreshAction()
{
  updateDetectorSettings();
  if (this->imageList->currentItem() != 0)
    processImage(this->imageList->currentItem()->text());
}

void ObjedCheck::onAboutAction()
{
  QString title = tr("About ObjedCheck");
  QString text = tr("This application is designed to check classifiers");

  QString fullText = "<h3>%0 (ver. %1)</h3<p>%2</p>";
  fullText = fullText.arg(QApplication::applicationName());
  fullText = fullText.arg(QApplication::applicationVersion());
  fullText = fullText.arg(text.simplified());
  QMessageBox::about(this, title, fullText);
}

void ObjedCheck::onChooseDetectorClicked()
{
  QSettings settings;
  QString detectorDirPath = settings.value("LastDetectorDirPath").toString();

  QFileDialog::Options options = QFileDialog::DontUseNativeDialog | QFileDialog::DontResolveSymlinks;
  QString detectorPath = QFileDialog::getOpenFileName(this, QString(), detectorDirPath, 
    QString("Detector Files (*.json)"), 0, options);
  if (detectorPath.isEmpty() == true)
    return;

  detectorDirPath = QFileInfo(detectorPath).absolutePath();
  settings.setValue("LastDetectorDirPath", detectorDirPath);

  detectorLine->setText(detectorPath);
  updateDetectorSettings();
}

void ObjedCheck::onClearDetectorClicked()
{
  detectorLine->clear();
  updateDetectorSettings();
}

void ObjedCheck::refreshTimer()
{
  timer.restart();
}

void ObjedCheck::info(const QString &msg)
{
  console->setTextColor(Qt::black);
  console->append(QString("%0 (%1 ms)").arg(msg).arg(timer.elapsed()));
}

void ObjedCheck::error(const QString &msg)
{
  console->setTextColor(Qt::red);
  console->append(QString("%0 (%1 ms)").arg(msg).arg(timer.elapsed()));
}


void ObjedCheck::updateDetectorSettings()
{
  try
  {
    refreshTimer();

    if (detectorLine->text().isEmpty() == false)
    {
      QFuture<QSharedPointer<objed::Detector> > loadClFuture;
      loadClFuture = QtConcurrent::run(ObjedIO::loadDetector, detectorLine->text());

      QProgressDialog progressDialog(this);
      progressDialog.setLabelText("Loading detector");
      progressDialog.setAutoClose(false);
      progressDialog.setRange(0, 0);
      progressDialog.show();

      QCoreApplication::processEvents();
      while (loadClFuture.isFinished() == false)
      {
        QCoreApplication::processEvents();
        if (progressDialog.wasCanceled())
          return;
      }
      
      detector = loadClFuture.result();

//       clusterer.clear();
//       if (clusterMode->currentIndex() > 0)
//         clusterer = ObjedClusterer::create(clusterMode->currentText(), clusterParams->text());
    }
  }
  catch(ObjedException ex)
  {
    error(ex.details());
  }
}

void ObjedCheck::processImage(const QString &imageName)
{
  this->console->clear();
  this->preview->scene()->clear();
  if (imageName.isEmpty() == true)
    return;

  objed::DetectionList detectionList;
  QSharedPointer<ObjedImage> image;

  try
  {
    //////////////////////////////////////////////////////////////////////////
    // Loading image
    refreshTimer();
    image = ObjedImage::create(imageDir.absoluteFilePath(imageName));
    info(QString("Image %0 has been loaded").arg(imageName));

    //////////////////////////////////////////////////////////////////////////
    // Detecting objects
    if (detector.isNull() == false)
    {
      refreshTimer();
      detectionList = detector->detect(image->image());
      info(QString("%0 object(s) have been detected").arg(detectionList.size()));
    }
  }
  catch (ObjedException ex)
  {
    error(ex.details());
  }
  //////////////////////////////////////////////////////////////////////////
  // Draw Image
  preview->scene()->addPixmap(image->toQPixmap());
  preview->setSceneRect(0, 0, image->width(), image->height());
  preview->fitInView(preview->sceneRect(), Qt::KeepAspectRatio);

  //////////////////////////////////////////////////////////////////////////
  // Draw Detections
  for (size_t i = 0; i < detectionList.size(); i++)
  {
    if (detectionList[i].power < minPowerBox->value())
      continue;

    preview->scene()->addRect(detectionList[i].x, detectionList[i].y,
      detectionList[i].width, detectionList[i].height, QPen(Qt::red));
    QGraphicsTextItem *text = preview->scene()->addText(QString::number(detectionList[i].power));
    text->setPos(detectionList[i].x + 0.5, detectionList[i].y + 0.5);
    text->setFont(QFont("Courier", 6)); text->setDefaultTextColor(Qt::red);
  }

  //////////////////////////////////////////////////////////////////////////
  // Draw ImageView
  QSharedPointer<objed::ImagePool> imagePool(
    objed::ImagePool::create(), objed::ImagePool::destroy);
  imagePool->update(image->image());

  foreach(ImageViewDock *imageView, imageViewList)
    if (imageView->isHidden()) imageViewList.removeAll(imageView);
  foreach(ImageViewDock *imageView, imageViewList)
    imageView->refreshImageView(imagePool.data());
}

bool ObjedCheck::eventFilter(QObject *object, QEvent *event)
{
  if (object == this && event->type() == QEvent::Resize)
    this->preview->fitInView(this->preview->sceneRect(), Qt::KeepAspectRatio);
  return QMainWindow::eventFilter(object, event);
}
