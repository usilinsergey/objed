#pragma once
#ifndef AUGMENTATION_GAMMA_H_INCLUDED
#define AUGMENTATION_GAMMA_H_INCLUDED

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QString>
#include <QList>

#include "augmentation.h"

class AugmentationGammaProcessor : public AugmentationProcessor
{
public:
  static QString type();
  static QList<QCommandLineOption> options();

public:
  AugmentationGammaProcessor(QCommandLineParser &cmdParser);
  virtual ~AugmentationGammaProcessor();

public:
  virtual void process(const QString &inputImagePath, const QString &outputDirPath) const;

private:
  QList<double> gamma_coefs;
};

#endif  // AUGMENTATION_GAMMA_H_INCLUDED