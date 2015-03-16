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

#include <QFileInfo>
#include <QFile>
#include <QDir>

#include <objedutils/objedmarkup.h>
#include <objedutils/objedexp.h>

#include <json-cpp/json.h>

ObjedMarkup::ObjedMarkup(bool autoSave) : autoSave(autoSave), starred(false)
{
  return;
}

ObjedMarkup::ObjedMarkup(const QString &markupPath, bool autoSave) : autoSave(autoSave), starred(false)
{
  open(markupPath);
}

ObjedMarkup::~ObjedMarkup()
{
  close();
}

void ObjedMarkup::open(const QString &markupPath)
{
  close();

  this->starred = false;
  this->objectList.clear();
  this->markupPath = markupPath;

  QFile markupFile(markupPath);
  markupFile.setFileName(markupPath);
  if (markupFile.open(QIODevice::ReadOnly) == false)
    return;

  QByteArray data = markupFile.readAll();
  if (data.length() <= 0)
    return;

  Json::Value root;  Json::Reader reader;
  if (reader.parse(data.constData(), root) == false)
    return;

  starred = root["starred"].asBool();
  Json::Value objects = root["objects"];

  if (objects.isArray())
  {
    for (size_t i = 0; i < objects.size(); i++)
    {
      Json::Value jsonObject = objects[i];

      if (jsonObject.isArray() && jsonObject.size() >= 4)
      {
        int x = jsonObject[0u].asInt();
        int y = jsonObject[1u].asInt();
        int width = jsonObject[2u].asInt();
        int height = jsonObject[3u].asInt();
        objectList.append(QRect(x, y, width, height));
      }
    }
  }
}

void ObjedMarkup::save(const QString &markupPath)
{
  if (markupPath.isEmpty() == false)
    this->markupPath = markupPath;

  if (this->markupPath.isEmpty() == true)
    return;

  if (QFileInfo(this->markupPath).absoluteDir().exists() == false) 
    QDir().mkpath(QFileInfo(this->markupPath).absolutePath());

  QFile markupFile(this->markupPath);
  if (markupFile.open(QIODevice::WriteOnly) == false)
    throw ObjedException(QString("Cannot save markup to %0").arg(this->markupPath));

  Json::Value root;
  root["starred"] = starred;
  foreach (QRect object, objectList)
  {
    Json::Value jsonRect;
    jsonRect.append(object.x());
    jsonRect.append(object.y());
    jsonRect.append(object.width());
    jsonRect.append(object.height());
    root["objects"].append(jsonRect);
  }

  Json::FastWriter writer;
  markupFile.write(writer.write(root).c_str());
}

void ObjedMarkup::close()
{
  starred = false;
  objectList.clear();
  markupPath.clear();
}

void ObjedMarkup::setStarred(bool starred)
{
  if (this->starred == starred)
    return;

  this->starred = starred;
  if (autoSave) save();
}

void ObjedMarkup::appendObject(const QRect &object)
{
  if (object.isEmpty() || objectList.contains(object))
    return;

  objectList.append(object);
  if (autoSave) save();
}

void ObjedMarkup::removeObject(const QRect &object)
{
  if (object.isEmpty() || !objectList.contains(object))
    return;

  objectList.removeAll(object);
  if (autoSave) save();
}

void ObjedMarkup::modifyObject(const QRect &oldObject, const QRect &newObject)
{
  if (oldObject.isEmpty() || newObject.isEmpty() || !objectList.contains(oldObject))
    return;
  if (oldObject == newObject)
    return;

  objectList.removeAll(oldObject);
  objectList.append(newObject);
  if (autoSave) save();
}

bool ObjedMarkup::isOpened() const
{
  return markupPath.isEmpty() == false;
}

bool ObjedMarkup::isStarred() const
{
  return starred;
}

QList<QRect> ObjedMarkup::objects() const
{
  return objectList;
}
