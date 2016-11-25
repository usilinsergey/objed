#include <QDir>

#include <objedutils/objedimage.h>
#include <objedutils/objedconsole.h>

#include "augmentation_rotate.h"

QString AugmentationRotateProcessor::type()
{
  return QString("rotate");
}

QList<QCommandLineOption> AugmentationRotateProcessor::options()
{
  QCommandLineOption angles_opt("rotate-angles",
    "Specifies the list of angles in degrees separated by semicolon e.g. '0.5;1.0;1.5'", "angles", "0.0");
  QCommandLineOption color_opt("rotate-bg",
    "Specifies the background in string format e.g. '#FFFFFF'", "bg", "#FFFFFF");
  return QList<QCommandLineOption>() << angles_opt << color_opt;
}

AugmentationRotateProcessor::AugmentationRotateProcessor(QCommandLineParser &cmdParser)
{
  QString angles_str = cmdParser.value("rotate-angles");
  foreach(QString angle_str, angles_str.split(";", QString::SkipEmptyParts))
    angles.append(angle_str.toDouble());

  QString bg_str = cmdParser.value("rotate-bg");
  bg.setNamedColor(QColor::isValidColor(bg_str) ? bg_str : "#FFFFFF");
}

AugmentationRotateProcessor::~AugmentationRotateProcessor()
{
  return;
}

void AugmentationRotateProcessor::process(const QString &inputImagePath, const QString &outputDirPath) const
{
  QDir outputDir(outputDirPath);
  QString imageName = QFileInfo(inputImagePath).baseName();
  QString imageExt = QFileInfo(inputImagePath).suffix();

  QSharedPointer<ObjedImage> image = ObjedImage::create(inputImagePath);
  if (image.isNull())
  {
    ObjedConsole::printError(QString("Cannot load image %0").arg(inputImagePath));
    return;
  }

  for (int i = 0; i < angles.length(); i++)
  {
    QSharedPointer<ObjedImage> rotated_image = ObjedImage::create(image.data(), QRect(0, 0, image->width(), image->height()));
    rotated_image->rotate(angles[i], bg);

    QString rotated_image_name = QString("%0_r%1.%2").arg(imageName).arg(angles[i]).arg(imageExt);
    rotated_image->save(outputDir.absoluteFilePath(rotated_image_name));
  }
}

