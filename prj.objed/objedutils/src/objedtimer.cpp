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

#include <objedutils/objedtimer.h>

#ifdef WIN32
#  include <windows.h>
#else  // WIN32
#  include <ctime>
#endif // WIN32

ObjedTimer::ObjedTimer() : startClock(0)
{
  startClock = clock();
}

ObjedTimer::~ObjedTimer()
{
  return;
}

void ObjedTimer::restart()
{
  startClock = clock();
}

float ObjedTimer::elapsed()
{
  long long stopClock = clock();
  return (stopClock - startClock + 0.0f) / clockPerSecond();
}

long long ObjedTimer::clock()
{
#ifdef WIN32
  LARGE_INTEGER t = {0};
  QueryPerformanceCounter(&t);
  return t.QuadPart;
#else  // WIN32
  return clock();
#endif // WIN32
}

long long ObjedTimer::clockPerSecond()
{
#ifdef WIN32
  LARGE_INTEGER t = {0};
  QueryPerformanceFrequency(&t);
  return t.QuadPart;
#else  // WIN32
  return CLOCKS_PER_SEC;
#endif // WIN32
}
