#include <QCoreApplication>
#include <QTemporaryFile>
#include <QDir>

#include <objedutils/objedconsole.h>
#include <objedutils/objedconf.h>
#include <objedutils/objedexp.h>
#include <objedutils/objedio.h>

#include <objed/src/additivecl.h>
#include <objed/src/linearcl.h>

#include "visualizer.h"

int Visualizer::falseNodeCount = 0;
int Visualizer::trueNodeCount = 0;

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  if (app.arguments().count() < 3)
  {
    QTextStream out(stdout);
    out << "Visualizer. Visualize objed classifier as a graph (using graphviz)" << endl;
    out << "Usage: visualizer <classifier-path> <output-image-path> [<graphviz-dir-path>]" << endl;
    return -1;
  }

  QString classifierPath = app.arguments()[1];
  QString outputImagePath = app.arguments()[2];

  Visualizer visualizer(app.arguments().count() > 3 ? app.arguments()[3] : "");
  return visualizer.main(classifierPath, outputImagePath);
}

Visualizer::Visualizer(const QString &graphvizDirPath)
{
#ifdef Q_OS_WIN
  if (graphvizDirPath.isEmpty() == false)
  {
    dotProcess.setWorkingDirectory(graphvizDirPath);
  }
  else
  {
    QStringList graphvizDirPathList;
    QDir programFilesDir(qgetenv("ProgramFiles"));
    foreach (QString dirName, programFilesDir.entryList(QStringList("Graphviz*"), QDir::Dirs))
      graphvizDirPathList.append(programFilesDir.absoluteFilePath(dirName));

    if (graphvizDirPathList.count() > 0)
      dotProcess.setWorkingDirectory(graphvizDirPathList.last() + "/bin");
  }
#else //Q_OS_WIN
  if (graphvizDirPath.isEmpty() == false)
  {
    dotProcess.setWorkingDirectory(graphvizDirPath);
  }
#endif //Q_OS_WIN
}

Visualizer::~Visualizer()
{
  return;
}

int Visualizer::main(const QString &classifierPath, const QString &outputImagePath)
{
  QDir appDir(QCoreApplication::applicationDirPath());

  try
  {
    QTemporaryFile digraphFile; 
    if (digraphFile.open() == false)
      throw ObjedException("Cannot create temporary file");

    QTextStream out(&digraphFile);
    out << "digraph Cascade {" << endl;

    QSharedPointer<objed::Classifier> classifier = ObjedIO::loadClassifier(classifierPath);
    if (classifier.isNull() == true) 
      throw ObjedException("Cannot load classifier");

    if (classifier->type() == objed::CascadeClassifier::typeStatic())
      visualizeCascade(QTextStream(&digraphFile), classifier.dynamicCast<objed::CascadeClassifier>().data());
    else if (classifier->type() == objed::TreeClassifier::typeStatic())
      visualizeTree(QTextStream(&digraphFile), classifier.dynamicCast<objed::TreeClassifier>().data());
    else
      throw ObjedException("Unsupported classifier type");

    out << "}" << endl;
        
    QStringList dotArguments;
    dotArguments.append(QString("-T%0").arg(QFileInfo(outputImagePath).suffix().toLower()));
    dotArguments.append(QString("-o%0").arg(appDir.absoluteFilePath(outputImagePath)));
    dotArguments.append(digraphFile.fileName());

    QString dotPath = dotProcess.workingDirectory().isEmpty() ? 
      "dot" : QDir(dotProcess.workingDirectory()).absoluteFilePath("dot");

    dotProcess.start(dotPath, dotArguments);
    if (dotProcess.waitForFinished() == false) 
      throw ObjedException("Cannot run dot process");

    QByteArray dotOutput = dotProcess.readAllStandardOutput();
    if (dotOutput.length() > 0) ObjedConsole::printWarning(dotOutput.trimmed());

    QByteArray dotError = dotProcess.readAllStandardError();
    if (dotError.length() > 0) ObjedConsole::printWarning(dotError.trimmed());
  }
  catch (ObjedException ex)
  {
    ObjedConsole::printError(ex.details());
  }

  return 0;
}

int Visualizer::computeWCCount(const objed::Classifier *classifier)
{
  if (classifier == 0)
    throw ObjedException("Invalid classifier (classifier == null)");

  if (classifier->type() == objed::LinearClassifier::typeStatic())
    return dynamic_cast<const objed::LinearClassifier *>(classifier)->clList.size();
  if (classifier->type() == objed::AdditiveClassifier::typeStatic())
    return dynamic_cast<const objed::AdditiveClassifier *>(classifier)->clList.size();

  throw ObjedException("Unsupported strong classifier");
}

void Visualizer::connect(QTextStream &out, const QString &node1, const QString &node2, bool type)
{
  out << QString("%0 -> %1 [label = \" %3\"];").arg(node1).arg(node2).arg(type ? "T" : "F");
}

void Visualizer::connect(QTextStream &out, const QString &node1, bool type)
{
  QString nodeId = type ? QString("true%0").arg(trueNodeCount++) : QString("false%0").arg(falseNodeCount++);
  QString nodeColor = type ? QString("darkgreen") : QString("red");
  QString nodeLabel = type ? QString("True") : QString("False");
  
  out << QString("%0 [label = \"%1\", shape = ellipse, color = %2, fontcolor = %2];").
    arg(nodeId).arg(nodeLabel).arg(nodeColor) << endl;

  connect(out, node1, nodeId, type);
}

int Visualizer::visualizeCascade(QTextStream &out, objed::CascadeClassifier *cascade)
{
  if (cascade == 0 || cascade->clList.size() == 0)
    throw ObjedException("The cascade has no levels");

  size_t i = 0;
  for (i = 0; i < cascade->clList.size() - 1; i++)
  {
    int wcCount = computeWCCount(cascade->clList[i]);
    out << QString("sc%0 [label = \"Sc (%1 Wc)\"];").arg(i).arg(wcCount) << endl;
    connect(out, QString("sc%0").arg(i), false);
    connect(out, QString("sc%0").arg(i), QString("sc%0").arg(i + 1), true);
  }
  int wcCount = computeWCCount(cascade->clList[i]);
  out << QString("sc%0 [label = \"Sc (%1 Wc)\"];").arg(i).arg(wcCount) << endl;
  connect(out, QString("sc%0").arg(i), false);
  connect(out, QString("sc%0").arg(i), true);

  return 0;
}

int Visualizer::visualizeTree(QTextStream &out, objed::TreeClassifier *tree)
{
  static int scIndex = 0;

  if (tree == 0 || tree->centralCl == 0)
    throw ObjedException("The tree has no nodes");

  const int myIndex = scIndex++;
  const int wcCount = computeWCCount(tree->centralCl);
  out << QString("sc%0 [label = \"Sc (%1 Wc)\"];").arg(myIndex).arg(wcCount) << endl;

  if (tree->leftCl != 0)
  {
    int leftIndex = visualizeTree(out, dynamic_cast<objed::TreeClassifier *>(tree->leftCl));
    connect(out, QString("sc%0").arg(myIndex), QString("sc%0").arg(leftIndex), false);
  }
  else
  {
    connect(out, QString("sc%0").arg(myIndex), false);
  }

  if (tree->rightCl != 0)
  {
    int rightIndex = visualizeTree(out, dynamic_cast<objed::TreeClassifier *>(tree->rightCl));
    connect(out, QString("sc%0").arg(myIndex), QString("sc%0").arg(rightIndex), true);
    
  }
  else
  {
    connect(out, QString("sc%0").arg(myIndex), true);
  }

  return myIndex;
}
