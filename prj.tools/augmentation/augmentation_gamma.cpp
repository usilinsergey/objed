#include <QDir>

#include <objedutils/objedimage.h>
#include <objedutils/objedconsole.h>

#include "augmentation_gamma.h"

QString AugmentationGammaProcessor::type()
{
  return QString("gamma");
}

QList<QCommandLineOption> AugmentationGammaProcessor::options()
{
  QCommandLineOption gamma_coefs_opt("gamma-coefs",
    "Specifies the list of applied gamma coefficients separated by semicolon e.g. '0.5;1.0;1.5'", "coefs", "1.0");
  return QList<QCommandLineOption>() << gamma_coefs_opt;
}

AugmentationGammaProcessor::AugmentationGammaProcessor(QCommandLineParser &cmdParser)
{
  QString gamma_coefs_str = cmdParser.value("gamma-coefs");
  foreach(QString gamma_coef_str, gamma_coefs_str.split(";", QString::SkipEmptyParts))
    gamma_coefs.append(gamma_coef_str.toDouble());
}

AugmentationGammaProcessor::~AugmentationGammaProcessor()
{
  return;
}

void AugmentationGammaProcessor::process(const QString &inputImagePath, const QString &outputDirPath) const
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

  for (int i = 0; i < gamma_coefs.length(); i++)
  {
    QSharedPointer<ObjedImage> gamma_image = ObjedImage::create(image.data(), QRect(0, 0, image->width(), image->height()));
    gamma_image->gammaCorrection(gamma_coefs[i]);

    QString gamma_image_name = QString("%0_g%1.%2").arg(imageName).arg(gamma_coefs[i]).arg(imageExt);
    gamma_image->save(outputDir.absoluteFilePath(gamma_image_name));
  }
}

