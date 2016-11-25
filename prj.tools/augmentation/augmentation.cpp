#include <QCoreApplication>
#include <QCommandLineParser>
#include <QSharedPointer>
#include <QDir>

#include <objedutils/objedconsole.h>
#include <objedutils/objedsys.h>

#include "augmentation.h"
#include "augmentation_gamma.h"
#include "augmentation_rotate.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  QCommandLineParser cmdParser;
  
  cmdParser.setApplicationDescription(QString("The application process augmentation of positive training dataset"));

  QStringList augmentation_types;
  augmentation_types << AugmentationGammaProcessor::type();
  augmentation_types << AugmentationRotateProcessor::type();
  cmdParser.addOption(QCommandLineOption("type", QString("Specifies type of augmentation (%0)").arg(augmentation_types.join(", ")), "type"));

  cmdParser.addOptions(AugmentationGammaProcessor::options());
  cmdParser.addOptions(AugmentationRotateProcessor::options());

  cmdParser.addPositionalArgument("input-dir", "Input image directory", "<image-dir>");
  cmdParser.addPositionalArgument("output-dir", "Output image directory", "<image-dir>");
  cmdParser.addHelpOption();
  cmdParser.process(app);

  QStringList pa = cmdParser.positionalArguments();
  if (cmdParser.positionalArguments().count() != 2)
    cmdParser.showHelp();

  QDir inputImageDir(cmdParser.positionalArguments().first());
  QStringList imagePathList = inputImageDir.entryList(ObjedSys::imageFilters());
  if (imagePathList.count() == 0)
  {
    ObjedConsole::printError("There are no images into input directory");
    QCoreApplication::exit();
  }

  QString outputImageDirPath = cmdParser.positionalArguments().last();
  if (QDir(outputImageDirPath).exists() == false)
  {
    ObjedConsole::printError("The output directory is not exists");
    QCoreApplication::exit();
  }

  QString type = cmdParser.value("type");
  QSharedPointer<AugmentationProcessor> processor;
  if (type == AugmentationGammaProcessor::type())
    processor.reset(new AugmentationGammaProcessor(cmdParser));
  else if (type == AugmentationRotateProcessor::type())
    processor.reset(new AugmentationRotateProcessor(cmdParser));
  else
    cmdParser.showHelp();

  for (int i = 0; i < imagePathList.count(); i++)
  {
    int progress = 100 * i / imagePathList.count();
    QString imagePath = inputImageDir.absoluteFilePath(imagePathList[i]);
    ObjedConsole::printProgress(QString("Processing %0").arg(imagePath), progress);
    processor->process(imagePath, outputImageDirPath);
  }

  ObjedConsole::printProgress(QString("Input images have been processed"), 100);
  return 0;
}