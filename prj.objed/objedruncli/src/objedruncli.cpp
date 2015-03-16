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

#include <objedutils/objedconsole.h>
#include <objedutils/objedconfig.h>
#include <objedutils/objedexp.h>
#include <objedutils/objedio.h>

#include "objedrunutils.h"
#include "objedruncli.h"

int ObjedRunCLI::main(ObjedConfig *config)
{
  QString logPath = config->value("LogPath").toString();
  if (logPath.isEmpty() == false) ObjedConsole::setLogPath(logPath);

  try
  {
    ObjedRunProcessor processor(config);

    QVariantList datasetList = config->listValue("DatasetList");
    if (datasetList.isEmpty() == true) 
      throw ObjedException("Dataset list is empty");

    foreach (QVariant dataset, datasetList)
      processor.processDataset(dataset.toString());

    processor.printTotalStatistics();
  }
  catch (ObjedException ex)
  {
    ObjedConsole::printError(ex.details());
  }

  return 0;
}
