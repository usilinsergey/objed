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

#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>
#include <QFileInfo>

#include <objedutils/objedconfig.h>
#include <objedutils/objedsys.h>

#include "objedtrainutils.h"
#include "objedtraincli.h"

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  app.setOrganizationName("Objed");
  app.setApplicationName("ObjedTrainCLI");
  app.setApplicationVersion(ObjedSys::version());

  QTextStream out(stdout);
  out << QString("%0 (ver. %1)").arg(app.applicationName()).arg(app.applicationVersion()) << endl;
  out << QString("Command-line interface application for training classifiers using Boosting Approach") << endl << endl;

  ObjedConfig config;
  config.registerValue("ClassifierType", "Specifies the target type of classifier ('cascadeClassifier' or 'treeClassifier')",    QVariant("cascadeClassifier"));
  config.registerValue("ClassifierPath", "Specifies output classifier path in canonized form (e.g. C:/classifier.json)",         QVariant());
  config.registerValue("ClassifierWidth", "Specifies output classifier width (must be positive odd number)",                     QVariant());
  config.registerValue("ClassifierHeight", "Specifies output classifier height (must be positive odd number)",                   QVariant());
  config.registerValue("MinimumScale", "Specifies the minimum object scale value (must be greater or equal to 1.0)",             QVariant(1.0));
  config.registerValue("MaximumScale", "Specifies the maximum object scale value (must be greater or equal to MinimumScale)",    QVariant(1.0));
  config.registerValue("StepScale", "Specifies the scale step (must be greater then 1.0)",                                       QVariant(1.1));
  config.registerValue("NegativeCount", "Specifies the negative sample count used for training (must be positive number)",       QVariant(1000));
  config.registerValue("PositiveCountThreshold", "Specifies the minimum threshold of positive sample count needed for training", QVariant(0));
  config.registerValue("NegativeCountThreshold", "Specifies the minimum threshold of negative sample count needed for training", QVariant(0));
  config.registerValue("WeightShift", "Specifies initial distribution of weights (total initial weight of positive samples)",    QVariant(0.5));
  config.registerValue("LevelCount", "Specifies count of levels to be trained (if 0 then infinite count)",                       QVariant(0));
  config.registerValue("WcCount", "Specifies count of weak classifiers for each level (if 0 then infinite count)",               QVariant(0));
  config.registerValue("FalseNegativeRate", "Specifies allowable rate of false negative (must be in range between 0.0 and 1.0)", QVariant(0.0));
  config.registerValue("FalsePositiveRate", "Specifies allowable rate of false positive (must be in range between 0.0 and 1.0)", QVariant(0.0));
  config.registerValue("StoppingCriterion", "Specifies the stopping criterion for level training ('Count', 'Rate', or 'Any')",   QVariant("Any")); 
  config.registerValue("Method", "Specifies training method ('RealAdaBoost', 'RealAdaBoostMax')",                                QVariant("RealAdaBoost"));
  config.registerValue("MethodParams", "Specifies additional parameters for training method",                                    QVariant(""));
  config.registerValue("TempIniPath", "Specifies the path to the temporary data file (INI-file), not used if empty",             QVariant(""));
  config.registerValue("LogPath", "Specifies the log path in canonized form (if empty log is not saved)",                        QVariant());
  config.registerListValue("PositiveDatasetList", "Specifies the list of directories in canonized form with positive icons",     QVariantList());
  config.registerListValue("NegativeDatasetList", "Specifies the list of directories in canonized form with negative images",    QVariantList());
  config.registerListValue("WcLineList", "Specifies available weak classifier as floows:\n" + WcMaker::help().join("\n"),        QVariantList());
  
  if (app.arguments().count() == 2)
  {
    if (config.parseConfig(app.arguments().last()) == false)
    {
      out << "Cannot parse configuration file" << endl;
      return -1;
    }
  }
  else if (app.arguments().count() == 3 && app.arguments().value(1).toLower() == "-g")
  {
    if (config.saveTemplate(app.arguments().last()) == false)
    {
      out << "Cannot save template configuration file" << endl;
      return -1;
    }
    
    out << "Template configuration file has been generated" << endl;
    return 0;
  }
  else
  {
    out << "The application can be used in a number of ways:" << endl << endl;
    out << "1. Use objedtraincli to build classifier using configuration file:" << endl;
    out << "   " << app.arguments().first() << " <configName>" << endl << endl;
    out << "2. Use objedtraincli to generate a template configuration file:" << endl;
    out << "   " << app.arguments().first() << " -g <configName>" << endl <<endl;
    return 0;
  }

  ObjedTrainCLI objedTrainCLI;
  return objedTrainCLI.main(&config);
}