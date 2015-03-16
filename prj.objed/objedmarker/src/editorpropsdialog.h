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
#ifndef EDITORPROPSDIALOG_H_INCLUDED
#define EDITORPROPSDIALOG_H_INCLUDED

#include "ui_editorpropsdialog.h"

class MarkupEditorPropertiesDialog : public QDialog, public Ui::MarkupEditorPropertiesDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(MarkupEditorPropertiesDialog)

public:
  MarkupEditorPropertiesDialog(QWidget *parent);
  virtual ~MarkupEditorPropertiesDialog();

public:
  static const QColor defaultIdealObjectPenColor;
  static const QColor defaultIdealObjectBrushColor;
  static const int    defaultIdealObjectWidth;

  static const QColor defaultRunObjectPenColor;
  static const QColor defaultRunObjectBrushColor;
  static const int    defaultRunObjectWidth;

  static const QColor defaultStarZonePenColor;
  static const QColor defaultStarZoneBrushColor;
  static const int    defaultStarZoneWidth;

  static const bool   defaultRepeatStar;

public:
  void setIdealObjectEditorProperties(const QColor &penColor, const QColor &brushColor, int width);
  void setRunObjectEditorProperties(const QColor &penColor, const QColor &brushColor, int width);
  void setStarZoneEditorProperties(const QColor &penColor, const QColor &brushColor, int width);
  void setAdditionalProperties(bool repeatStar);

public:
  QColor idealObjectPenColor() const;
  QColor idealObjectBrushColor() const;
  int    idealObjectWidth() const;

  QColor runObjectPenColor() const;
  QColor runObjectBrushColor() const;
  int    runObjectWidth() const;

  QColor starZonePenColor() const;
  QColor starZoneBrushColor() const;
  int    starZoneWidth() const;

  bool   isRepeatStar() const;

private slots:
  void chooseColor();
  void restoreDefaults();
};

#endif  // EDITORPROPSDIALOG_H_INCLUDED