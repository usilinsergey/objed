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

#include <objedutils/objedcompare.h>

ObjedCompare::Result::Result() :
truePositive(0), falsePositive(0), falseNegative(0), imageCount(0)
{
  return;
}

ObjedCompare::Result::Result(int tp, int fp, int fn, int count /*= 1*/) :
truePositive(tp), falsePositive(fp), falseNegative(fn), imageCount(count)
{
  return;
}

ObjedCompare::Result & ObjedCompare::Result::operator+=(const Result &other)
{
  truePositive += other.truePositive;
  falsePositive += other.falsePositive;
  falseNegative += other.falseNegative;
  imageCount += other.imageCount;
  return *this;
}

ObjedCompare::Result ObjedCompare::Result::operator+(const Result &other) const
{
  return Result(truePositive + other.truePositive,falsePositive + other.falsePositive,
    falseNegative + other.falseNegative, imageCount + other.imageCount);
}

bool ObjedCompare::Result::operator==(const Result &other) const
{
  return truePositive == other.truePositive && falsePositive == other.falsePositive &&
    falseNegative == other.falseNegative && imageCount == other.imageCount;
}

bool ObjedCompare::Result::operator!=(const Result &other) const
{
  return truePositive != other.truePositive && falsePositive != other.falsePositive &&
    falseNegative != other.falseNegative && imageCount != other.imageCount;
}

ObjedCompare::Result ObjedCompare::compare(const ObjedMarkup &realMarkup, const ObjedMarkup &idealMarkup, double threshold)
{
  Result results(0, 0, 0);

  QList<QRect> realObjectList = realMarkup.objects();
  QList<QRect> idealObjectList = idealMarkup.objects();
  bool isStarred = idealMarkup.isStarred();

  while (realObjectList.count() > 0 && idealObjectList.count() > 0)
  {
    QRect realObject = realObjectList.takeFirst();
    bool isGood = false;

    for (int i = 0; i < idealObjectList.count(); i++)
    {
      QRect idealObject = idealObjectList.takeFirst();
      QRect united = realObject.united(idealObject);
      QRect intersected = realObject.intersected(idealObject);

      double unitedSq = united.width() * united.height();
      double intersectedSq = intersected.width() * intersected.height();

      if (intersectedSq / unitedSq > threshold)
      {
        isGood = true;
        break;
      }
      else
      {
        idealObjectList.append(idealObject);
      }
    }

    results.truePositive += (isGood == true);
    results.falsePositive += (isGood == false && isStarred == true);
  }

  results.falsePositive += (isStarred == true) * realObjectList.count();
  results.falseNegative += idealObjectList.count();

  return results;
}