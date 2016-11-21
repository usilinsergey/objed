#pragma once
#ifndef VISUALIZER_H_INClUDED
#define VISUALIZER_H_INClUDED

#include <QTextStream>
#include <QProcess>

#include <objed/src/cascadecl.h>
#include <objed/src/treecl.h>

class Visualizer
{
public:
  Visualizer(const QString &graphvizDirPath = "");
  virtual ~Visualizer();

public:
  int main(const QString &classifierPath, const QString &outputImagePath);

private:
  static int computeWCCount(const objed::Classifier *classifier);

private:
  static void connect(QTextStream &out, const QString &node1, const QString &node2, bool type);
  static void connect(QTextStream &out, const QString &node1, bool type);

private:
  static int visualizeCascade(QTextStream &out, objed::CascadeClassifier *cascade);
  static int visualizeTree(QTextStream &out, objed::TreeClassifier *tree);
private:
  static int falseNodeCount, trueNodeCount;
  QProcess dotProcess;
};

#endif  // VISUALIZER_H_INClUDED