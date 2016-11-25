#pragma once
#ifndef AUGMENTATION_ROTATE_H_INCLUDED
#define AUGMENTATION_ROTATE_H_INCLUDED

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QString>
#include <QColor>
#include <QList>

#include "augmentation.h"

class AugmentationRotateProcessor : public AugmentationProcessor
{
public:
  static QString type();
  static QList<QCommandLineOption> options();

public:
  AugmentationRotateProcessor(QCommandLineParser &cmdParser);
  virtual ~AugmentationRotateProcessor();

public:
  virtual void process(const QString &inputImagePath, const QString &outputDirPath) const;

private:
  QList<double> angles;
  QColor bg;
};

#endif  // AUGMENTATION_ROTATE_H_INCLUDED