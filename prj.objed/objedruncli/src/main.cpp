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

#include <objedutils/objedconfig.h>
#include <objedutils/objedsys.h>

#include "objedruncli.h"

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  app.setOrganizationName("Objed");
  app.setApplicationName("ObjedRunCLI");
  app.setApplicationVersion(ObjedSys::version());

  QTextStream out(stdout);
  out << QString("%0 (ver. %1)").arg(app.applicationName()).arg(app.applicationVersion()) << endl;
  out << "Command-line interface application for running objed classifiers on datasets" << endl << endl;

  ObjedConfig config;
  config.registerValue("DetectorPath", "Specifies detector path in canonized form (e.g. C:/detector.json)",         QVariant());
  config.registerValue("MinimumPower", "Specifies the minimum detection power",                                     QVariant(0));
  config.registerValue("IdealMarkupName", "Specifies the name of ideal object markup (the same as in objedmarker)", QVariant());
  config.registerValue("SaveRealMarkup", "Specifies whether real markup (run result) should be save or not",        QVariant(true));
  config.registerValue("SaveDebugInfo", "Specifies whether debug info should be save or not",                       QVariant(false));
  config.registerValue("Threshold", "Specifies the comparison threshold between real and ideal objects",            QVariant(0.5));
  config.registerValue("LogPath", "Specifies the log path in canonized form (if empty log is not saved)",           QVariant());
  config.registerListValue("DatasetList", "Specifies the list of markuped directories in canonized form",          QVariantList());

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
    QString appName = QFileInfo(app.arguments().first()).fileName();
    out << "The application can be used in a number of ways:" << endl << endl;
    out << "1. Use objedruncli to run objed classifier using configuration file:" << endl;
    out << "   " << appName << " <configName>" << endl << endl;
    out << "2. Use objedruncli to generate a template configuration file:" << endl;
    out << "   " << appName << " -g <configName>" << endl <<endl;
    return 0;
  }

  ObjedRunCLI objedRunCLI;
  return objedRunCLI.main(&config);
}

