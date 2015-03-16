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

#include <QColorDialog>
#include <QPushButton>

#include "editorpropsdialog.h"

const QColor MarkupEditorPropertiesDialog::defaultIdealObjectPenColor(0xFF, 0x00, 0x00, 0xFF);
const QColor MarkupEditorPropertiesDialog::defaultIdealObjectBrushColor(0xFF, 0x00, 0x00, 0x1F);
const int    MarkupEditorPropertiesDialog::defaultIdealObjectWidth = 0;

const QColor MarkupEditorPropertiesDialog::defaultRunObjectPenColor(0x00, 0x00, 0xFF, 0xFF);
const QColor MarkupEditorPropertiesDialog::defaultRunObjectBrushColor(0x00, 0x00, 0xFF, 0x1F);
const int    MarkupEditorPropertiesDialog::defaultRunObjectWidth = 0;

const QColor MarkupEditorPropertiesDialog::defaultStarZonePenColor(0xFF, 0xFF, 0x00, 0xFF);
const QColor MarkupEditorPropertiesDialog::defaultStarZoneBrushColor(0xFF, 0xFF, 0x00, 0x1F);
const int    MarkupEditorPropertiesDialog::defaultStarZoneWidth = 0;

const bool   MarkupEditorPropertiesDialog::defaultRepeatStar = true;

static QColor string2color(const QString &string)
{
  if (string.length() >= 8)
    return QColor::fromRgba(string.right(8).toUInt(0, 16));
  else
    return QColor::fromRgb(string.right(6).toUInt(0, 16));
}

static QString color2string(const QColor &color)
{
  return QString::number(color.rgba(), 16).toUpper().prepend("#");
}

MarkupEditorPropertiesDialog::MarkupEditorPropertiesDialog(QWidget *parent) :
QDialog(parent)
{
  setupUi(this);
  layout()->setSizeConstraint(QLayout::SetFixedSize);

  connect(chooseIdealObjectPenButton, SIGNAL(clicked()), SLOT(chooseColor()));
  connect(chooseIdealObjectBrushButton, SIGNAL(clicked()), SLOT(chooseColor()));
  connect(chooseRunObjectPenButton, SIGNAL(clicked()), SLOT(chooseColor()));
  connect(chooseRunObjectBrushButton, SIGNAL(clicked()), SLOT(chooseColor()));
  connect(chooseStarZonePenButton, SIGNAL(clicked()), SLOT(chooseColor()));
  connect(chooseStarZoneBrushButton, SIGNAL(clicked()), SLOT(chooseColor()));
  
  QPushButton *restoreButton = buttonBox->button(QDialogButtonBox::RestoreDefaults);
  connect(restoreButton, SIGNAL(clicked()), SLOT(restoreDefaults()));
  connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

  QMetaObject::invokeMethod(this, "restoreDefaults");
}

MarkupEditorPropertiesDialog::~MarkupEditorPropertiesDialog()
{
  return;
}

void MarkupEditorPropertiesDialog::chooseColor()
{
  QToolButton *chooseButton = qobject_cast<QToolButton *>(QObject::sender());
  QLineEdit *colorLine = 0; QLabel *colorPreview = 0;

  if (chooseButton == chooseIdealObjectPenButton)
  {
    colorPreview = idealObjectPenPreview;
    colorLine = idealObjectPenLine;
  }
  else if (chooseButton == chooseIdealObjectBrushButton)
  {
    colorPreview = idealObjectBrushPreview;
    colorLine = idealObjectBrushLine;
  }
  else if (chooseButton == chooseRunObjectPenButton)
  {
    colorPreview = runObjectPenPreview;
    colorLine = runObjectPenLine;
  }
  else if (chooseButton == chooseRunObjectBrushButton)
  {
    colorPreview = runObjectBrushPreview;
    colorLine = runObjectBrushLine;
  }
  else if (chooseButton == chooseStarZonePenButton)
  {
    colorPreview = starZonePenPreview;
    colorLine = starZonePenLine;
  }
  else if (chooseButton == chooseStarZoneBrushButton)
  {
    colorPreview = starZoneBrushPreview;
    colorLine = starZoneBrushLine;
  }
  else
    return;

  QColor color = QColorDialog::getColor(string2color(colorLine->text()), this, 
    QString(), QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
  
  if (color.isValid() == true) 
  {
    colorPreview->setPalette(QPalette(color));
    colorLine->setText(color2string(color));
  }
}

void MarkupEditorPropertiesDialog::restoreDefaults()
{
  setIdealObjectEditorProperties(defaultIdealObjectPenColor, 
    defaultIdealObjectBrushColor, defaultIdealObjectWidth);

  setRunObjectEditorProperties(defaultRunObjectPenColor,
    defaultRunObjectBrushColor, defaultRunObjectWidth);

  setStarZoneEditorProperties(defaultStarZonePenColor,
  defaultStarZoneBrushColor, defaultStarZoneWidth);
}

void MarkupEditorPropertiesDialog::setIdealObjectEditorProperties(const QColor &penColor, const QColor &brushColor, int width)
{
  idealObjectPenPreview->setPalette(QPalette(penColor));
  idealObjectPenLine->setText(color2string(penColor));

  idealObjectBrushPreview->setPalette(QPalette(brushColor));
  idealObjectBrushLine->setText(color2string(brushColor));

  idealObjectWidthSlider->setValue(width);
}

void MarkupEditorPropertiesDialog::setRunObjectEditorProperties(const QColor &penColor, const QColor &brushColor, int width)
{
  runObjectPenPreview->setPalette(QPalette(penColor));
  runObjectPenLine->setText(color2string(penColor));

  runObjectBrushPreview->setPalette(QPalette(brushColor));
  runObjectBrushLine->setText(color2string(brushColor));

  runObjectWidthSlider->setValue(width);
}

void MarkupEditorPropertiesDialog::setStarZoneEditorProperties(const QColor &penColor, const QColor &brushColor, int width)
{
  starZonePenPreview->setPalette(QPalette(penColor));
  starZonePenLine->setText(color2string(penColor));

  starZoneBrushPreview->setPalette(QPalette(brushColor));
  starZoneBrushLine->setText(color2string(brushColor));

  starZoneWidthSlider->setValue(width);
}

void MarkupEditorPropertiesDialog::setAdditionalProperties(bool repeatStar)
{
  repeatStarCheckBox->setChecked(repeatStar);
}

QColor MarkupEditorPropertiesDialog::idealObjectPenColor() const
{
  return string2color(idealObjectPenLine->text());
}

QColor MarkupEditorPropertiesDialog::idealObjectBrushColor() const
{
  return string2color(idealObjectBrushLine->text());
}

int MarkupEditorPropertiesDialog::idealObjectWidth() const
{
  return idealObjectWidthSlider->value();
}

QColor MarkupEditorPropertiesDialog::runObjectPenColor() const
{
  return string2color(runObjectPenLine->text());
}

QColor MarkupEditorPropertiesDialog::runObjectBrushColor() const
{
  return string2color(runObjectBrushLine->text());
}

int MarkupEditorPropertiesDialog::runObjectWidth() const
{
  return runObjectWidthSlider->value();
}

QColor MarkupEditorPropertiesDialog::starZonePenColor() const
{
  return string2color(starZonePenLine->text());
}

QColor MarkupEditorPropertiesDialog::starZoneBrushColor() const
{
  return string2color(starZoneBrushLine->text());
}

int MarkupEditorPropertiesDialog::starZoneWidth() const
{
  return starZoneWidthSlider->value();
}

bool MarkupEditorPropertiesDialog::isRepeatStar() const
{
  return repeatStarCheckBox->isChecked();
}