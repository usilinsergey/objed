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

#include <QGraphicsSceneMouseEvent>
#include <QSettings>
#include <QDebug>

#include <objedutils/objedmarkup.h>
#include <objedutils/objedexp.h>

#include "editorpropsdialog.h"
#include "objectsizedialog.h"
#include "markupeditor.h"
#include "objedmarker.h"

StarImageCommand::StarImageCommand(bool starred, ObjedMarkupEditor *editor) : 
QUndoCommand(starred ? QObject::tr("Star Image") : QObject::tr("Unstar Image")), editor(editor), starred(starred)
{
  Q_ASSERT(editor != 0);
  Q_ASSERT(editor->idealMarkup != 0);
}

void StarImageCommand::redo()
{
  editor->lastStarred = starred;
  editor->idealMarkup->setStarred(starred);
  editor->updateStarImageLabel();
}

void StarImageCommand::undo()
{
  editor->lastStarred = !starred;
  editor->idealMarkup->setStarred(!starred);
  editor->updateStarImageLabel();
}

AppendObjectCommand::AppendObjectCommand(const QRect &object, ObjedMarkupEditor *editor) :
QUndoCommand(QObject::tr("Append Object")), editor(editor), object(object)
{
  Q_ASSERT(editor != 0);
  Q_ASSERT(editor->idealMarkup != 0);
}

void AppendObjectCommand::redo()
{
  editor->idealMarkup->appendObject(object);
  editor->redraw();
}

void AppendObjectCommand::undo()
{
  editor->idealMarkup->removeObject(object);
  editor->redraw();
}

RemoveObjectCommand::RemoveObjectCommand(const QRect &object, ObjedMarkupEditor *editor) :
QUndoCommand(QObject::tr("Remove Object")), editor(editor), object(object)
{
  Q_ASSERT(editor != 0);
  Q_ASSERT(editor->idealMarkup != 0);
}

void RemoveObjectCommand::redo()
{
  editor->idealMarkup->removeObject(object);
  editor->redraw();
}

void RemoveObjectCommand::undo()
{
  editor->idealMarkup->appendObject(object);
  editor->redraw();
}

ModifyObjectCommand::ModifyObjectCommand(const QRect &oldObject, const QRect &newObject, ObjedMarkupEditor *editor) :
QUndoCommand(QObject::tr("Modify Object")), editor(editor), oldObject(oldObject), newObject(newObject)
{
  Q_ASSERT(editor != 0);
  Q_ASSERT(editor->idealMarkup != 0);
}

void ModifyObjectCommand::redo()
{
  editor->idealMarkup->modifyObject(oldObject, newObject);
  editor->redraw();
}

void ModifyObjectCommand::undo()
{
  editor->idealMarkup->modifyObject(newObject, oldObject);
  editor->redraw();
}
IdealObjectItem::IdealObjectItem(const QRect &objectRect, 
  const QColor &penColor, const QColor &brushColor, int width) : 
QGraphicsRectItem(QRectF(objectRect))
{
  this->setPen(QPen(penColor, static_cast<double>(width)));
  this->setBrush(QBrush(brushColor));

  this->setTransformOriginPoint(rect().x() + rect().width() / 2,
    rect().y() + rect().height() / 2);
}

IdealObjectItem::~IdealObjectItem()
{
  return;
}


RunObjectItem::RunObjectItem(const QRect &objectRect, 
  const QColor &penColor, const QColor &brushColor, int width) :
QGraphicsRectItem(QRectF(objectRect))
{
  this->setPen(QPen(penColor, static_cast<double>(width)));
  this->setBrush(QBrush(brushColor));
}

RunObjectItem::~RunObjectItem()
{
  return;
}

ObjedMarkupEditor::ObjedMarkupEditor(QGraphicsScene *scene, QObject *parent) : 
QObject(parent), scene(scene), selectedIdealObject(0), starItem(0), lockedItem(0), lockedStarState(false)
{
  scene->installEventFilter(this);

  idealMarkup = new ObjedMarkup();
  runMarkup = new ObjedMarkup();

  QSettings settings;
  minimumObjectSize = settings.value("MarkupEditor/MinimumObjectSize",
    ObjedMarkerObjectSizeDialog::defaultMinimumObjectSize).toSize();
  maximumObjectSize = settings.value("MarkupEditor/MaximumObjectSize",
    ObjedMarkerObjectSizeDialog::defaultMaximumObjectSize).toSize();
  baseObjectSize = settings.value("MarkupEditor/BaseObjectSize", 
    ObjedMarkerObjectSizeDialog::defaultBaseObjectSize).toSize();
  lastObjectSize = settings.value("MarkupEditor/LastObjectSize",
    ObjedMarkerObjectSizeDialog::defaultBaseObjectSize).toSize();

  idealObjectPenColor = settings.value("MarkupEditor/IdealObjectPenColor", 
    MarkupEditorPropertiesDialog::defaultIdealObjectPenColor).value<QColor>();
  idealObjectBrushColor = settings.value("MarkupEditor/IdealObjectBrushColor", 
    MarkupEditorPropertiesDialog::defaultIdealObjectBrushColor).value<QColor>();
  idealObjectWidth = settings.value("MarkupEditor/IdealObjectWidth", 
    MarkupEditorPropertiesDialog::defaultIdealObjectWidth).value<int>();

  runObjectPenColor = settings.value("MarkupEditor/RunObjectPenColor", 
    MarkupEditorPropertiesDialog::defaultRunObjectPenColor).value<QColor>();
  runObjectBrushColor = settings.value("MarkupEditor/RunObjectBrushColor", 
    MarkupEditorPropertiesDialog::defaultRunObjectBrushColor).value<QColor>();
  runObjectWidth = settings.value("MarkupEditor/RunObjectWidth", 
    MarkupEditorPropertiesDialog::defaultRunObjectWidth).value<int>();

  starZonePenColor = settings.value("MarkupEditor/StarZonePenColor", 
    MarkupEditorPropertiesDialog::defaultStarZonePenColor).value<QColor>();
  starZoneBrushColor = settings.value("MarkupEditor/StarZoneBrushColor", 
    MarkupEditorPropertiesDialog::defaultStarZoneBrushColor).value<QColor>();
  starZoneWidth = settings.value("MarkupEditor/StarZoneWidth", 
    MarkupEditorPropertiesDialog::defaultStarZoneWidth).value<int>();

  repeatStar = settings.value("MarkupEditor/RepeatStar",
    MarkupEditorPropertiesDialog::defaultRepeatStar).value<bool>();

  undo = undoStack.createUndoAction(this, tr("&Undo"));
  undo->setIcon(QIcon(":/actionIcons/undo"));
  undo->setShortcut(QKeySequence::Undo);

  redo = undoStack.createRedoAction(this, tr("&Redo"));
  redo->setIcon(QIcon(":/actionIcons/redo"));
  redo->setShortcut(QKeySequence::Redo);

  increaseObjectSize = new QAction(QIcon(
    ":/actionIcons/increaseObject"), tr("&Increase Object Size"), this);
  increaseObjectSize->setShortcut(QKeySequence("W"));

  decreaseObjectSize = new QAction(QIcon(
    ":/actionIcons/decreaseObject"), tr("&Decrease Object Size"), this);
  decreaseObjectSize->setShortcut(QKeySequence("S"));

  starImage = new QAction(QIcon(
    ":/actionIcons/starImage"), tr("&Star Image"), this);
  starImage->setShortcut(QKeySequence("Space"));

  lockStarState = new QAction(QIcon(
    ":/actionIcons/lockStarState"), tr("&Lock Star State"), this);
  lockStarState->setShortcut(QKeySequence("Shift+Space"));

  objectSizeProperties = new QAction(QIcon(
    ":/actionIcons/objectSize"), tr("&Object Size Properties..."), this);
  editorProperties = new QAction(QIcon(
    ":/actionIcons/editor"), tr("&Editor Properties..."), this);

  QWidget *objedmarker = qobject_cast<QWidget *>(parent);
  selectedObjectWidget = new QLabel(objedmarker);
  starImageWidget = new QWidget(objedmarker);
  starImageWidget->setLayout(new QHBoxLayout(starImageWidget));
  starImageWidget->layout()->setContentsMargins(0, 0, 0, 0);
  starImageWidget->layout()->addWidget(new QLabel(starImageWidget));
  starImageWidget->layout()->addWidget(new QLabel(starImageWidget));
  ((QHBoxLayout *)starImageWidget->layout())->addStretch();
  lockStarStateWidget = new QWidget(objedmarker);
  lockStarStateWidget->setLayout(new QHBoxLayout(lockStarStateWidget));
  lockStarStateWidget->layout()->setContentsMargins(0, 0, 0, 0);
  lockStarStateWidget->layout()->addWidget(new QLabel(lockStarStateWidget));
  lockStarStateWidget->layout()->addWidget(new QLabel(lockStarStateWidget));
  ((QHBoxLayout *)lockStarStateWidget->layout())->addStretch();

  connect(increaseObjectSize, SIGNAL(triggered()), SLOT(onResizeObjectAction()));
  connect(decreaseObjectSize, SIGNAL(triggered()), SLOT(onResizeObjectAction()));
  connect(starImage, SIGNAL(triggered()), SLOT(onStarImageAction()));
  connect(lockStarState, SIGNAL(triggered()), SLOT(onLockStarStateAction()));

  connect(objectSizeProperties, SIGNAL(triggered()), SLOT(onObjectSizePropertiesAction()));
  connect(editorProperties, SIGNAL(triggered()), SLOT(onEditorPropertiesAction()));

  update(QPixmap(), QString(), QString());
}

ObjedMarkupEditor::~ObjedMarkupEditor()
{
  QSettings settings;
  settings.setValue("MarkupEditor/MinimumObjectSize", minimumObjectSize);
  settings.setValue("MarkupEditor/MaxumumObjectSize", maximumObjectSize);
  settings.setValue("MarkupEditor/BaseObjectSize", baseObjectSize);
  settings.setValue("MarkupEditor/LastObjectSize", lastObjectSize);

  settings.setValue("MarkupEditor/IdealObjectPenColor", idealObjectPenColor);
  settings.setValue("MarkupEditor/IdealObjectBrushColor", idealObjectBrushColor);
  settings.setValue("MarkupEditor/IdealObjectWidth", idealObjectWidth);

  settings.setValue("MarkupEditor/RunObjectPenColor", runObjectPenColor);
  settings.setValue("MarkupEditor/RunObjectBrushColor", runObjectBrushColor);
  settings.setValue("MarkupEditor/RunObjectWidth", runObjectWidth);

  settings.setValue("MarkupEditor/StarZonePenColor", starZonePenColor);
  settings.setValue("MarkupEditor/StarZoneBrushColor", starZoneBrushColor);
  settings.setValue("MarkupEditor/StarZoneWidth", starZoneWidth);

  settings.setValue("MarkupEditor/StarZoneWidth", repeatStar);
}

void ObjedMarkupEditor::update(const QPixmap &image, const QString &idealPath, const QString &runPath)
{
  try
  {
    undoStack.clear();
    scene->clear();

    selectedIdealObject = 0;
    lockedItem = 0;
    starItem = 0;

    if (image.isNull())
      return;

    scene->setSceneRect(0.0, 0.0, image.width(), image.height());
    scene->addPixmap(image);

    idealMarkup->open(idealPath);
    runMarkup->open(runPath);

    increaseObjectSize->setEnabled(idealMarkup->isOpened());
    decreaseObjectSize->setEnabled(idealMarkup->isOpened());
    starImage->setEnabled(idealMarkup->isOpened());
    lockStarState->setEnabled(idealMarkup->isOpened());

    if (lockedStarState && idealMarkup->isStarred() != lastStarred)
      undoStack.push(new StarImageCommand(lastStarred, this));
    else
      lastStarred = idealMarkup->isStarred();

    updateSelectedObjectLabel();
    updateLockStarStateLabel();
    updateStarImageLabel();
    redraw();
  }
  catch (ObjedException ex)
  {
    ObjedMarker *marker = qobject_cast<ObjedMarker *>(parent());
    if (marker != 0) marker->statusBar()->showMessage(ex.details(), 3000);
  }
}

QAction * ObjedMarkupEditor::undoAction() const
{
  return undo;
}

QAction * ObjedMarkupEditor::redoAction() const
{
  return redo;
}

QAction * ObjedMarkupEditor::increaseObjectSizeAction() const
{
  return increaseObjectSize;
}

QAction * ObjedMarkupEditor::decreaseObjectSizeAction() const
{
  return decreaseObjectSize;
}

QAction * ObjedMarkupEditor::starImageAction() const
{
  return starImage;
}

QAction * ObjedMarkupEditor::lockStarStateAction() const
{
  return lockStarState;
}

QAction * ObjedMarkupEditor::objectSizePropertiesAction() const
{
  return objectSizeProperties;
}

QAction * ObjedMarkupEditor::editorPropertiesAction() const
{
  return editorProperties;
}

QWidget * ObjedMarkupEditor::selectedObjectInfo() const
{
  return selectedObjectWidget;
}

QWidget * ObjedMarkupEditor::starImageInfo() const
{
  return starImageWidget;
}

QWidget * ObjedMarkupEditor::lockStarStateInfo() const
{
  return lockStarStateWidget;
}

void ObjedMarkupEditor::onResizeObjectAction()
{
  if (selectedIdealObject == 0)
    return;

  qreal currScale = selectedIdealObject->scale();
  qreal currWidth = selectedIdealObject->rect().width() * currScale;
  qreal currHeight = selectedIdealObject->rect().height() * currScale;

  qreal delta = QObject::sender() == increaseObjectSize ? 1.0 : -1.0;
  qreal newScale = baseObjectSize.width() < baseObjectSize.height() ? 
    (currWidth + delta) /  selectedIdealObject->rect().width() : 
    (currHeight + delta) /  selectedIdealObject->rect().height();
  qreal newWidth = selectedIdealObject->rect().width() * newScale;
  qreal newHeight = selectedIdealObject->rect().height() * newScale;

  if (newWidth < minimumObjectSize.width() || newWidth > maximumObjectSize.width())
    return;
  if (newHeight < minimumObjectSize.height() || newHeight > maximumObjectSize.height())
    return;

  selectedIdealObject->setScale(newScale);

  updateSelectedObjectLabel();
}

void ObjedMarkupEditor::onStarImageAction()
{
  try
  {
    undoStack.push(new StarImageCommand(!idealMarkup->isStarred(), this));
    updateStarImageLabel();
  }
  catch (ObjedException ex)
  {
    ObjedMarker *marker = qobject_cast<ObjedMarker *>(parent());
    if (marker != 0) marker->statusBar()->showMessage(ex.details(), 3000);
  }
}

void ObjedMarkupEditor::onLockStarStateAction()
{
  lockedStarState = !lockedStarState;
  updateLockStarStateLabel();
}

void ObjedMarkupEditor::onObjectSizePropertiesAction()
{
  QWidget *objedmarker = qobject_cast<QWidget *>(parent());
  ObjedMarkerObjectSizeDialog objectSizeDialog(objedmarker);
  objectSizeDialog.setBaseObjectSize(baseObjectSize);
  objectSizeDialog.setMinimumObjectSize(minimumObjectSize);
  objectSizeDialog.setMaximumObjectSize(maximumObjectSize);
  if (objectSizeDialog.exec() == QDialog::Rejected)
    return;

  baseObjectSize = objectSizeDialog.baseObjectSize();
  minimumObjectSize = objectSizeDialog.minimumObjectSize();
  maximumObjectSize = objectSizeDialog.maximumObjectSize();
  lastObjectSize = baseObjectSize;
}

void ObjedMarkupEditor::onEditorPropertiesAction()
{
  QWidget *objedmarker = qobject_cast<QWidget *>(parent());
  MarkupEditorPropertiesDialog editorPropertiesDialog(objedmarker);

  editorPropertiesDialog.setIdealObjectEditorProperties(
    idealObjectPenColor, idealObjectBrushColor, idealObjectWidth);
  editorPropertiesDialog.setRunObjectEditorProperties(
    runObjectPenColor, runObjectBrushColor, runObjectWidth);
  editorPropertiesDialog.setStarZoneEditorProperties(
    starZonePenColor, starZoneBrushColor, starZoneWidth);
  editorPropertiesDialog.setAdditionalProperties(repeatStar);

  if (editorPropertiesDialog.exec() == QDialog::Rejected)
    return;

  idealObjectPenColor = editorPropertiesDialog.idealObjectPenColor();
  idealObjectBrushColor = editorPropertiesDialog.idealObjectBrushColor();
  idealObjectWidth = editorPropertiesDialog.idealObjectWidth();

  runObjectPenColor = editorPropertiesDialog.runObjectPenColor();
  runObjectBrushColor = editorPropertiesDialog.runObjectBrushColor();
  runObjectWidth = editorPropertiesDialog.runObjectWidth();

  starZonePenColor = editorPropertiesDialog.starZonePenColor();
  starZoneBrushColor = editorPropertiesDialog.starZoneBrushColor();
  starZoneWidth = editorPropertiesDialog.starZoneWidth();

  repeatStar = editorPropertiesDialog.isRepeatStar();
}

bool ObjedMarkupEditor::eventFilter(QObject *object, QEvent *event)
{
  try
  {
    QGraphicsSceneMouseEvent *mouseEvent = 
      dynamic_cast<QGraphicsSceneMouseEvent *>(event);

    if (object != scene || mouseEvent == 0) 
      return QObject::eventFilter(object, event);

    QEvent::Type eventType = mouseEvent->type();
    Qt::MouseButtons mouseButtons = mouseEvent->buttons();

    if (eventType == QEvent::GraphicsSceneMousePress && mouseButtons == Qt::LeftButton)
    {
      QGraphicsItem *item = scene->itemAt(mouseEvent->scenePos(), QTransform());
      if (item == 0 || item->type() != IdealObjectItem::Type)
      {
        QRect object(mouseEvent->scenePos().x() - lastObjectSize.width() / 2.0,
          mouseEvent->scenePos().y() - lastObjectSize.height() / 2.0,
          lastObjectSize.width(), lastObjectSize.height());

        if (item != 0 && item->type() == RunObjectItem::Type)
          object = qgraphicsitem_cast<RunObjectItem *>(item)->rect().toRect();

        undoStack.push(new AppendObjectCommand(object, this));
        item = scene->itemAt(mouseEvent->scenePos(), QTransform());
      }

      selectedIdealObject = qgraphicsitem_cast<IdealObjectItem *>(item);

      if (selectedIdealObject != 0)
      {
        selectedIdealObject->setPen(QPen(selectedIdealObject->pen().color(), 
          selectedIdealObject->pen().width(), Qt::DashLine));
      }

      updateSelectedObjectLabel();

      return true;
    }
    if (eventType == QEvent::GraphicsSceneMousePress && mouseButtons == Qt::RightButton)
    {
      QGraphicsItem *item = scene->itemAt(mouseEvent->scenePos(), QTransform());
      if (qgraphicsitem_cast<IdealObjectItem *>(item) != 0)
      {
        IdealObjectItem *objectItem = qgraphicsitem_cast<IdealObjectItem *>(item);
        QRect objectRect = objectItem->rect().translated(objectItem->pos()).toRect();
        undoStack.push(new RemoveObjectCommand(objectRect, this));
      }

      return true;
    }
    else if (event->type() == QEvent::GraphicsSceneMouseMove  && mouseButtons == Qt::LeftButton)
    {
      QGraphicsSceneMouseEvent *mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent *>(event);
      if (selectedIdealObject == 0)
        return true;

      QPointF deltaPos = mouseEvent->scenePos() - mouseEvent->lastScenePos();
      selectedIdealObject->moveBy(deltaPos.x(), deltaPos.y());

      updateSelectedObjectLabel();

      return true;
    }
    else if (event->type() == QEvent::GraphicsSceneMouseRelease)
    {
      if (selectedIdealObject == 0)
        return true;

      qreal dwd = selectedIdealObject->rect().size().width() * (selectedIdealObject->scale() - 1.0);
      qreal dht = selectedIdealObject->rect().size().height() * (selectedIdealObject->scale() - 1.0);

      QRect oldObjectRect = selectedIdealObject->rect().toRect();
      QRect newObjectRect = selectedIdealObject->rect().translated(selectedIdealObject->pos()).
        adjusted(-dwd / 2, -dht / 2, dwd / 2, dht / 2).toRect();

      lastObjectSize = (selectedIdealObject->rect().size() + QSizeF(dwd, dht)).toSize();

      selectedIdealObject->setPen(QPen(selectedIdealObject->pen().color(), 
        selectedIdealObject->pen().width(), Qt::SolidLine));
      selectedIdealObject = 0;

      updateSelectedObjectLabel();

      if (oldObjectRect != newObjectRect)
        undoStack.push(new ModifyObjectCommand(oldObjectRect, newObjectRect, this));

      return true;
    }
  }
  catch (ObjedException ex)
  {
    ObjedMarker *marker = qobject_cast<ObjedMarker *>(parent());
    if (marker != 0) marker->statusBar()->showMessage(ex.details(), 3000);
  }

  return QObject::eventFilter(object, event);
}

void ObjedMarkupEditor::updateSelectedObjectLabel()
{
  QLabel *selectedObjectLabel = qobject_cast<QLabel *>(selectedObjectWidget);
  if (selectedObjectLabel == 0) 
    return;

  if (selectedIdealObject == 0)
  {
    selectedObjectLabel->clear();
    return;
  }

  qreal dwd = selectedIdealObject->rect().size().width() * (selectedIdealObject->scale() - 1.0);
  qreal dht = selectedIdealObject->rect().size().height() * (selectedIdealObject->scale() - 1.0);
  QRect objectRect = selectedIdealObject->rect().translated(selectedIdealObject->pos()).
    adjusted(-dwd / 2, -dht / 2, dwd / 2, dht / 2).toRect();

  QString selectedObjectText("(%0, %1; %2 X %3)");
  selectedObjectText = selectedObjectText.arg(objectRect.x()).
    arg(objectRect.y()).arg(objectRect.width()).arg(objectRect.height());
  selectedObjectLabel->setText(selectedObjectText);
}

void ObjedMarkupEditor::updateLockStarStateLabel()
{
  QList<QLabel *> labelList = lockStarStateWidget->findChildren<QLabel *>();
  QLabel *iconLabel = labelList.first(), *textLabel = labelList.last();
  if (iconLabel == 0 || textLabel == 0)
    return;

  const int sceneWidth = scene->sceneRect().width();
  const int sceneHeight = scene->sceneRect().height();
  const int minSceneSize = qMin(sceneWidth, sceneHeight);
  const QSize lockSize(qMax(1, qRound(minSceneSize * 0.05)), qMax(1, qRound(minSceneSize * 0.05)));
  const QPointF lockShift(sceneWidth - 2 * lockSize.width(), lockSize.height());

  delete lockedItem; lockedItem = 0;

  if (idealMarkup->isOpened() == true && lockedStarState == true)
  {
    QPixmap lockPixmap = QIcon(":/actionIcons/lockStarState").pixmap(16, QIcon::Active, QIcon::On);
    textLabel->setText(tr("Locked")); iconLabel->setPixmap(lockPixmap);

    if (scene != 0 && repeatStar == true)
    {
      lockedItem = scene->addPixmap(lockPixmap.scaled(lockSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
      lockedItem->moveBy(lockShift.x(), lockShift.y());
    }
  }
  else if (idealMarkup->isOpened() == true && lockedStarState == false)
  {
    QPixmap lockPixmap = QIcon(":/actionIcons/lockStarState").pixmap(16, QIcon::Disabled, QIcon::On);
    textLabel->setText(tr("Unlocked")); iconLabel->setPixmap(lockPixmap);

    if (scene != 0 && repeatStar == true)
    {
      lockedItem = scene->addPixmap(lockPixmap.scaled(lockSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
      lockedItem->moveBy(lockShift.x(), lockShift.y());
   }
  }
  else 
  {
    QPixmap lockPixmap = QIcon(":/actionIcons/lockStarState").pixmap(16, QIcon::Disabled, QIcon::On);
    textLabel->setText(tr("Unknown")); iconLabel->setPixmap(lockPixmap);
  }
}

void ObjedMarkupEditor::updateStarImageLabel()
{
  QList<QLabel *> labelList = starImageWidget->findChildren<QLabel *>();
  QLabel *iconLabel = labelList.first(), *textLabel = labelList.last();
  if (iconLabel == 0 || textLabel == 0)
    return;

  const int sceneWidth = scene->sceneRect().width();
  const int sceneHeight = scene->sceneRect().height();
  const int minSceneSize = qMin(sceneWidth, sceneHeight);
  const QSize starSize(qMax(1, qRound(minSceneSize * 0.05)), qMax(1, qRound(minSceneSize * 0.05)));
  const QPointF starShift(sceneWidth - 4 * starSize.width(), starSize.height());

  delete starItem; starItem = 0;

  if (idealMarkup->isOpened() == true && idealMarkup->isStarred() == true)
  {
    QPixmap starPixmap = QIcon(":/actionIcons/starImage").pixmap(16, QIcon::Active, QIcon::On);
    textLabel->setText(tr("Starred")); iconLabel->setPixmap(starPixmap);
    
    if (scene != 0 && repeatStar == true)
    {
      starItem = scene->addPixmap(starPixmap.scaled(starSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
      starItem->moveBy(starShift.x(), starShift.y());
    }
  }
  else if (idealMarkup->isOpened() == true && idealMarkup->isStarred() == false)
  {
    QPixmap starPixmap = QIcon(":/actionIcons/starImage").pixmap(16, QIcon::Disabled, QIcon::On);
    textLabel->setText(tr("Not Starred")); iconLabel->setPixmap(starPixmap);

    if (scene != 0 && repeatStar == true)
    {
      starItem = scene->addPixmap(starPixmap.scaled(starSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
      starItem->moveBy(starShift.x(), starShift.y());
    }
  }
  else 
  {
    QPixmap starPixmap = QIcon(":/actionIcons/starImage").pixmap(16, QIcon::Disabled, QIcon::On);
    textLabel->setText(tr("Unknown")); iconLabel->setPixmap(starPixmap);
  }
}

void ObjedMarkupEditor::redraw()
{
  if (scene == 0)
    return;

  foreach(QGraphicsItem *item, scene->items())
  {
    if (item->type() != QGraphicsPixmapItem::Type &&
      item->type() != QGraphicsSimpleTextItem::Type)
        delete item;
  }

  if (runMarkup->isOpened())
  {
    QList<QRect> runObjects = runMarkup->objects();
    foreach (const QRect &runObject, runObjects)
    {
      RunObjectItem *item = new RunObjectItem(runObject, 
        runObjectPenColor, runObjectBrushColor, runObjectWidth);
      scene->addItem(item);
    }
  }

  if (idealMarkup->isOpened())
  {
    QList<QRect> idealObjects = idealMarkup->objects();
    foreach (const QRect &idealObject, idealObjects)
    {
      IdealObjectItem *item = new IdealObjectItem(idealObject, 
        idealObjectPenColor, idealObjectBrushColor, idealObjectWidth);
      scene->addItem(item);
    }
  }
}
