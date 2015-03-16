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

#include <QTextStream>
#include <QStringList>
#include <QSettings>
#include <QFile>

#include <objedutils/objedconfig.h>

ObjedConfig::ObjedConfig()
{
  return;
}

ObjedConfig::~ObjedConfig()
{
  return;
}

bool ObjedConfig::parseConfig(const QString &configPath)
{
  if (QFile::exists(configPath) == false)
    return false;

  QSettings config(configPath, QSettings::IniFormat);
  
  foreach(QString name, values.keys())
  {
    if (config.contains(name) == false)
      continue;

    values.insert(name, config.value(name));
  }

  foreach(QString name, listValues.keys())
  {
    if (config.childGroups().contains(name) == false)
      continue;

    QVariantList listValue;
    for (int i = 0; i < config.value(name + "/size").toInt(); i++)
      listValue.append(config.value(name + "/" + QString::number(i + 1)));

    listValues.insert(name, listValue);
  }

  return true;
}

bool ObjedConfig::saveTemplate(const QString &templatePath) const
{
  QFile templateFile(templatePath);
  if (templateFile.open(QIODevice::WriteOnly) == false)
    return false;

  templateFile.write(generateTemplate());
  return true;
}

QByteArray ObjedConfig::generateTemplate() const
{
  QByteArray templateData;
  QTextStream out(&templateData, QIODevice::WriteOnly);

  out << "; This is a configuration file for objed framework" << endl;
  out << "; This file is fully compliant with the .INI File Format" << endl;
  out << ";" << endl;
  out << "; Objed Configuration Format supports two types of elements: simple and arrays" << endl;
  out << ";" << endl;
  out << "; Simple elements should be listed in the section General, e.g.:" << endl;
  out << ";   [General]" << endl ;
  out << ";   keyname1 = value1" << endl;
  out << ";   keyname2 = value2" << endl;
  out << ";" << endl;
  out << "; Array elements should be specified as a separate section e.g.:" << endl;
  out << ";   [ArrayName]" << endl;
  out << ";   size = 2" << endl;
  out << ";   1 = value1" << endl;
  out << ";   2 = value2" << endl;
  out << endl;

  out << "[General]" << endl;
  foreach(QString name, valuesOrder)
  {
    if (descriptions.contains(name) == true)
    {
      QString description = descriptions.value(name);
      out << description.replace(QString("\n"), QString("\n; ")).prepend("; ") << endl;
    }

    QVariant default = values.value(name);
    out << name << " = " << default.toString() << endl;
  }

  foreach(QString name, listValuesOrder)
  {
    out << endl;

    if (descriptions.contains(name) == true)
    {
      QString description = descriptions.value(name);
      out << description.replace(QString("\n"), QString("\n; ")).prepend("; ") << endl;
    }

    QVariantList default = listValues.value(name);
    out << "[" << name << "]" << endl << "size = " << default.count() << endl;
    for (int i = 0; i < default.count(); i++) out << i << " = " << default.value(i).toString() << endl;
  }

  return templateData;
}

void ObjedConfig::registerValue(const QString &name, const QString &description, const QVariant &default)
{
  valuesOrder.append(name);
  values.insert(name, default);
  descriptions.insert(name, description);
}

void ObjedConfig::registerListValue(const QString &name, const QString &description, const QVariantList &default)
{
  listValuesOrder.append(name);
  listValues.insert(name, default);
  descriptions.insert(name, description);
}

QVariant ObjedConfig::value(const QString &name)
{
  Q_ASSERT(values.contains(name));
  return values.value(name);
}

QVariantList ObjedConfig::listValue(const QString &name)
{
  Q_ASSERT(listValues.contains(name));
  return listValues.value(name);
}