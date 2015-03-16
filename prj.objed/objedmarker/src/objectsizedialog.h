#pragma once
#ifndef OBJECTSIZEDIALOG_H_INCLUDED
#define OBJECTSIZEDIALOG_H_INCLUDED

#include "ui_objectsizedialog.h"

class ObjedMarkerObjectSizeDialog : public QDialog, public Ui::ObjedMarkerObjectSizeDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(ObjedMarkerObjectSizeDialog)

public:
  ObjedMarkerObjectSizeDialog(QWidget *parent = 0);
  virtual ~ObjedMarkerObjectSizeDialog();

public:
  static const QSize defaultBaseObjectSize;
  static const QSize defaultMinimumObjectSize;
  static const QSize defaultMaximumObjectSize;
  static const bool  defaultBaseObjectSizeEnabled;

public:
  void setBaseObjectSize(const QSize &size);
  void setMinimumObjectSize(const QSize &size);
  void setMaximumObjectSize(const QSize &size);
  void setBaseObjectSizeEnabled(bool state);

public:
  QSize baseObjectSize() const;
  QSize minimumObjectSize() const;
  QSize maximumObjectSize() const;
  bool  isBaseObjectSizeEnabled() const;

private slots:
  void restoreDefaults();
};

#endif  // OBJECTSIZEDIALOG_H_INCLUDED