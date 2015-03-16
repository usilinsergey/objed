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

#ifdef _OPENMP
#include <omp.h>
#endif  // _OPENMP

#ifdef Q_OS_WIN
#include <windows.h>
#endif // Q_OS_WIN

static int maxLineLength()
{
#ifdef Q_OS_WIN
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {0};
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
  return consoleInfo.dwSize.X - 14 - 1;
#else  // Q_OS_WIN
  return 80 - 14 - 1;
#endif // Q_OS_WIN
}

static inline QString timeString()
{
  return QTime::currentTime().toString("[HH:mm:ss.zzz]");
}

static inline QString progressString(int progress)
{
  return QString("[%0%]").arg(qBound(0, progress, 100), 3);
}

bool ObjedConsole::needNewLineCharacter = false;
QTextStream ObjedConsole::out(stdout);
QTextStream ObjedConsole::in(stdin);
QTextStream ObjedConsole::log;
QFile ObjedConsole::logFile;

void ObjedConsole::setLogPath(const QString &logPath)
{
#pragma omp critical(console)
  {
    logFile.close();
    logFile.setFileName(logPath);
    logFile.open(QIODevice::Append);
    log.setDevice(&logFile);
  }
}

void ObjedConsole::printInfo(const QString &textLine)
{
#pragma omp critical(console)
  {
    QString fullTextLine = QString("%0%1").
      arg(textLine.left(maxLineLength()), - maxLineLength(), '.').arg(timeString());

    if (needNewLineCharacter == true)
      out << endl;

    out << fullTextLine << endl << flush;

    if (log.device() != 0)
      log << fullTextLine << endl << flush;

    needNewLineCharacter = false;
  }
}

void ObjedConsole::printProgress(const QString &textLine, int progress)
{
#pragma omp critical(console)
  {
    QString fullTextLine = QString("%0 %1%2").arg(progressString(progress)).
      arg(textLine.left(maxLineLength() - 7), - maxLineLength() + 7, '.').arg(timeString());

    needNewLineCharacter = progress < 100;

    out << "\r" << fullTextLine << flush;

    if (needNewLineCharacter == false) 
    {
      out << endl << flush;

      if (log.device() != 0)
        log << fullTextLine << endl << flush;
    }
  }
}

void ObjedConsole::printWarning(const QString &textLine)
{
#ifdef Q_OS_WIN
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {0}; 
  HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

#pragma omp critical(console)
  {
    GetConsoleScreenBufferInfo(hStdout, &consoleInfo);
    WORD wAttributes = 0xFFF0 & consoleInfo.wAttributes;
    wAttributes |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    SetConsoleTextAttribute(hStdout, wAttributes);
  }
#endif // Q_OS_WIN

  printInfo(textLine);

#ifdef Q_OS_WIN
#pragma omp critical(console)
  {
    SetConsoleTextAttribute(hStdout, consoleInfo.wAttributes);
  }
#endif // Q_OS_WIN
}

void ObjedConsole::printError(const QString &textLine)
{
#ifdef Q_OS_WIN
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {0}; 
  HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

#pragma omp critical(console)
  {
    GetConsoleScreenBufferInfo(hStdout, &consoleInfo);
    WORD wAttributes = 0xFFF0 & consoleInfo.wAttributes;
    wAttributes |= FOREGROUND_RED | FOREGROUND_INTENSITY;
    SetConsoleTextAttribute(hStdout, wAttributes);
  }
#endif // Q_OS_WIN

  printInfo(textLine);

#ifdef Q_OS_WIN
#pragma omp critical(console)
  {
    SetConsoleTextAttribute(hStdout, consoleInfo.wAttributes);
  }
#endif // Q_OS_WIN
}

QString ObjedConsole::scanTextLine()
{
  QString line;

#pragma omp critical(console)
  {
    line = in.readLine();
  }

  return line;
}