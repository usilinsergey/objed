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

#include <QPushButton>

#include "objectsizedialog.h"

const QSize ObjedMarkerObjectSizeDialog::defaultBaseObjectSize(10, 10);
const QSize ObjedMarkerObjectSizeDialog::defaultMinimumObjectSize(1, 1);
const QSize ObjedMarkerObjectSizeDialog::defaultMaximumObjectSize(
  std::numeric_limits<int>::max(), std::numeric_limits<int>::max());

ObjedMarkerObjectSizeDialog::ObjedMarkerObjectSizeDialog(QWidget *parent) : 
QDialog(parent, Qt::Dialog | Qt::WindowStaysOnTopHint)
{
  setupUi(this);
  layout()->setSizeConstraint(QLayout::SetFixedSize);

  baseObjectWidth->setRange(1, std::numeric_limits<int>::max());
  baseObjectHeight->setRange(1, std::numeric_limits<int>::max());
  minimumObjectWidth->setRange(1, std::numeric_limits<int>::max());
  minimumObjectHeight->setRange(1, std::numeric_limits<int>::max());
  maximumObjectWidth->setRange(1, std::numeric_limits<int>::max());
  maximumObjectHeight->setRange(1, std::numeric_limits<int>::max());

  QPushButton *restoreButton = buttonBox->button(QDialogButtonBox::RestoreDefaults);
  connect(restoreButton, SIGNAL(clicked()), SLOT(restoreDefaults()));
  connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

  QMetaObject::invokeMethod(this, "restoreDefaults");
}

void ObjedMarkerObjectSizeDialog::setBaseObjectSize(const QSize &size)
{
  baseObjectWidth->setValue(size.width());
  baseObjectHeight->setValue(size.height());
}

void ObjedMarkerObjectSizeDialog::setMinimumObjectSize(const QSize &size)
{
  minimumObjectWidth->setValue(size.width());
  minimumObjectHeight->setValue(size.height());
}

void ObjedMarkerObjectSizeDialog::setMaximumObjectSize(const QSize &size)
{
  maximumObjectWidth->setValue(size.width());
  maximumObjectHeight->setValue(size.height());
}

ObjedMarkerObjectSizeDialog::~ObjedMarkerObjectSizeDialog()
{
  return;
}

QSize ObjedMarkerObjectSizeDialog::baseObjectSize() const
{
  return QSize(baseObjectWidth->value(), baseObjectHeight->value());
}

QSize ObjedMarkerObjectSizeDialog::minimumObjectSize() const
{
return QSize(minimumObjectWidth->value(), minimumObjectHeight->value());
}

QSize ObjedMarkerObjectSizeDialog::maximumObjectSize() const
{
return QSize(maximumObjectWidth->value(), maximumObjectHeight->value());
}

void ObjedMarkerObjectSizeDialog::restoreDefaults()
{
  setBaseObjectSize(defaultBaseObjectSize);
  setMinimumObjectSize(defaultMinimumObjectSize);
  setMaximumObjectSize(defaultMaximumObjectSize);
}
