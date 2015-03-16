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
#ifndef MARKUPEDITOR_H_INCLUDED
#define MARKUPEDITOR_H_INCLUDED

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QUndoCommand>
#include <QUndoStack>
#include <QPixmap>
#include <QAction>
#include <QObject>
#include <QEvent>
#include <QSize>

class ObjedMarkup;
class ObjedMarkupEditor;

class StarImageCommand : public QUndoCommand
{
public:
  StarImageCommand(bool starred, ObjedMarkupEditor *editor);
  virtual void redo();
  virtual void undo();

private:
  ObjedMarkupEditor *editor;
  bool starred;
};

class AppendObjectCommand : public QUndoCommand
{
public:
  AppendObjectCommand(const QRect &object, ObjedMarkupEditor *editor);
  virtual void redo();
  virtual void undo();

private:
  ObjedMarkupEditor *editor;
  QRect object;
};

class RemoveObjectCommand : public QUndoCommand
{
public:
  RemoveObjectCommand(const QRect &object, ObjedMarkupEditor *editor);
  virtual void redo();
  virtual void undo();

private:
  ObjedMarkupEditor *editor;
  QRect object;
};

class ModifyObjectCommand : public QUndoCommand
{
public:
  ModifyObjectCommand(const QRect &oldObject, const QRect &newObject, ObjedMarkupEditor *editor);
  virtual void redo();
  virtual void undo();

private:
  ObjedMarkupEditor *editor;
  QRect oldObject, newObject;
};

class IdealObjectItem : public QGraphicsRectItem
{
  Q_DISABLE_COPY(IdealObjectItem)

public:
  IdealObjectItem(const QRect &objectRect, 
    const QColor &penColor, const QColor &brushColor, int width);
  virtual ~IdealObjectItem();

public:
  enum { Type = UserType + 1 };
  int type() const { return Type; }
};

class RunObjectItem : public QGraphicsRectItem
{
  Q_DISABLE_COPY(RunObjectItem)

public:
  RunObjectItem(const QRect &objectRect, 
    const QColor &penColor, const QColor &brushColor, int width);
  virtual ~RunObjectItem();

public:
  enum { Type = UserType + 2 };
  int type() const { return Type; }
};

class ObjedMarkupEditor : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(ObjedMarkupEditor)

  friend StarImageCommand;
  friend AppendObjectCommand;
  friend RemoveObjectCommand;
  friend ModifyObjectCommand;

public:
  ObjedMarkupEditor(QGraphicsScene *scene, QObject *parent = 0);
  virtual ~ObjedMarkupEditor();

public slots:
  void update(const QPixmap &image, const QString &idealPath, const QString &runPath);

public:
  QAction * undoAction() const;
  QAction * redoAction() const;
  QAction * increaseObjectSizeAction() const;
  QAction * decreaseObjectSizeAction() const;
  QAction * starImageAction() const;
  QAction * lockStarStateAction() const;

  QAction * objectSizePropertiesAction() const;
  QAction * editorPropertiesAction() const;

  QWidget * selectedObjectInfo() const;
  QWidget * starImageInfo() const;
  QWidget * lockStarStateInfo() const;

private slots:
  void onResizeObjectAction();
  void onStarImageAction();
  void onLockStarStateAction();

  void onObjectSizePropertiesAction();
  void onEditorPropertiesAction();

private:  
  bool eventFilter(QObject *object, QEvent *event);
  void updateSelectedObjectLabel();
  void updateLockStarStateLabel();
  void updateStarImageLabel();
  void redraw();

private:
  ObjedMarkup *idealMarkup;
  ObjedMarkup *runMarkup;

private:
  int idealObjectWidth, runObjectWidth, starZoneWidth;
  QColor idealObjectPenColor, idealObjectBrushColor;
  QColor runObjectPenColor, runObjectBrushColor;
  QColor starZonePenColor, starZoneBrushColor;
  bool repeatStar;

private:
  QSize minimumObjectSize;
  QSize maximumObjectSize;
  QSize baseObjectSize;
  QSize lastObjectSize;

private:
  QGraphicsScene *scene;
  IdealObjectItem *selectedIdealObject;
  QGraphicsPixmapItem *lockedItem;
  QGraphicsPixmapItem *starItem;
  
  bool lastStarred;
  bool lockedStarState;

private:
  QAction *undo, *redo;
  QAction *increaseObjectSize;
  QAction *decreaseObjectSize;
  QAction *starImage, *lockStarState;
  QAction *objectSizeProperties, *editorProperties;

private:
  QWidget *selectedObjectWidget;
  QWidget *starImageWidget;
  QWidget *lockStarStateWidget;
  
private:
  QUndoStack undoStack;
};

#endif  // MARKUPEDITOR_H_INCLUDED