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

#pragma once
#ifndef OBJEDCONFIG_H_INCLUDED
#define OBJEDCONFIG_H_INCLUDED

#include <QByteArray>
#include <QVariant>
#include <QString>
#include <QHash>
#include <QList>

class ObjedConfig
{
public:
  ObjedConfig();
  virtual ~ObjedConfig();

public:
  bool parseConfig(const QString &configPath);
  bool saveTemplate(const QString &templatePath) const;
  QByteArray generateTemplate() const;

public:
  void registerValue(const QString &name, const QString &description = QString(), const QVariant &default = QVariant());
  void registerListValue(const QString &name, const QString &description = QString(), const QVariantList &default = QVariantList());
  
public:
  QVariant value(const QString &name);
  QVariantList listValue(const QString &name);

private:
  QList<QString> valuesOrder;
  QList<QString> listValuesOrder;
  QHash<QString, QVariant> values;
  QHash<QString, QVariantList> listValues;
  QHash<QString, QString> descriptions;
};

#endif  // OBJEDCONFIG_H_INCLUDED