#include <QCoreApplication>

#include <objedutils/objedconf.h>
#include <objedutils/objedconsole.h>
#include <objedutils/objedexp.h>
#include <objedutils/objedio.h>

#include <objed/src/cascadecl.h>
#include <objed/src/treecl.h>

#include "cascade2tree.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  if (app.arguments().count() < 3)
  {
    QTextStream out(stdout);
    out << "Cascade2Tree. Converts CascadeClassifier to TreeClassifier" << endl;
    out << "Usage: cascade2tree <cascade-path> <tree-path>" << endl;
    return -1;
  }

  QString cascadePath = app.arguments()[1];
  QString treePath = app.arguments()[2];

  Cascade2Tree cascade2Tree;
  return cascade2Tree.main(cascadePath, treePath);
}

int Cascade2Tree::main(const QString &cascadePath, const QString &treePath)
{
  try
  {
    QSharedPointer<objed::CascadeClassifier> cascade = 
      ObjedIO::loadClassifier(cascadePath).dynamicCast<objed::CascadeClassifier>();
    if (cascade.isNull() == true || cascade->type() != objed::CascadeClassifier::typeStatic())
      throw ObjedException("Cannot load cascade classifier");
    if (cascade->clList.size() == 0)
      throw ObjedException("Cascade classifier has no sc");

    QSharedPointer<objed::TreeClassifier> tree(
      new objed::TreeClassifier(cascade->width(), cascade->height()));

    objed::TreeClassifier *currTree = tree.data();

    for (size_t i = 0; i < cascade->clList.size() - 1; i++)
    {
      currTree->centralCl = cascade->clList[i]->clone();
      currTree->rightCl = new objed::TreeClassifier(currTree->width(), currTree->height());
      currTree = dynamic_cast<objed::TreeClassifier *>(currTree->rightCl);
    }

    currTree->centralCl = cascade->clList.back()->clone();

    ObjedIO::saveClassifier(tree.data(), treePath);
    ObjedConsole::printInfo("Tree classifier has been successfully saved");
  }
  catch (ObjedException ex)
  {
    ObjedConsole::printError(ex.details());
  }

  return 0;
}

