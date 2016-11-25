#pragma once
#ifndef AUGMENTATION_H_INClUDED
#define AUGMENTATION_H_INClUDED

#include <QCommandLineOption>

class AugmentationProcessor
{
public:
  virtual void process(const QString &inputImagePath, const QString &outputDirPath) const = 0;
  virtual ~AugmentationProcessor() {}
};

#endif  // AUGMENTATION_H_INClUDED