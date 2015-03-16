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

#include <objedutils/objedsys.h>
#include <objedutils/objedconf.h>

#include <QCoreApplication>
#include <QSettings>
#include <QRegExp>
#include <QHash>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

QDir ObjedSys::rootDir()
{
  if (QCoreApplication::instance() != 0)
    return QDir(QCoreApplication::applicationDirPath());

#ifdef Q_OS_WIN
  HMODULE hModule = 0;
  BOOL ret = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)&__FUNCTION__, &hModule);
  if (ret == FALSE)
    return QDir::current();

#ifdef UNICODE
  wchar_t libraryPathBuf[MAX_PATH] = {0};
#else
  char libraryPathBuf[MAX_PATH] = {0};
#endif

  DWORD nLen = GetModuleFileName(hModule, libraryPathBuf, MAX_PATH);
  if (nLen == MAX_PATH)
    return QDir::current();

#ifdef UNICODE
  QFileInfo libraryInfo(QString::fromWCharArray(libraryPathBuf));
#else
  QFileInfo libraryInfo(QString::fromLocal8Bit(libraryPathBuf));
#endif

  return libraryInfo.dir();

#else
  return QDir::current();
#endif
}

QString ObjedSys::version()
{
  return QString("%0.%1").arg(OBJED_MAJOR_VERSION).arg(OBJED_MINOR_VERSION);
}

QString ObjedSys::resolvePath(const QString &path)
{
  QString resolvedPath = path;
  QHash<QString, QString> variableList;
  variableList.insert("ObjedRootDir", rootDir().absolutePath());

  QSettings settings(rootDir().absoluteFilePath("objedsys.ini"), QSettings::IniFormat);
  int variableCount = settings.beginReadArray("Variables");

  for (int i = 0; i < variableCount; i++)
  {
    settings.setArrayIndex(i);
    const QString variableName = QString("${%0}").arg(settings.value("Name").toString());
    const QString variableValue = settings.value("Value").toString();
    variableList.insert(variableName, variableValue);
  }

  QRegExp regExp("\\$\\{\\w+\\}");
  while(regExp.indexIn(resolvedPath) != -1)
  {
    QString variableName = regExp.cap();
    QString variableValue = variableList.value(variableName);
    resolvedPath.replace(variableName, variableValue);
  }

  return resolvedPath;
}

QStringList ObjedSys::imageFilters()
{
  QStringList filters;
  filters << "*.tif" << "*.tiff" << "*.jpg" << "*.jpeg" << "*.png";
  return filters;
}

