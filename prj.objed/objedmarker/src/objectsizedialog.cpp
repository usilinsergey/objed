#include <QPushButton>

#include "objectsizedialog.h"
#include <limits>

const QSize ObjedMarkerObjectSizeDialog::defaultBaseObjectSize(10, 10);
const QSize ObjedMarkerObjectSizeDialog::defaultMinimumObjectSize(1, 1);
const QSize ObjedMarkerObjectSizeDialog::defaultMaximumObjectSize(
  std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
const bool ObjedMarkerObjectSizeDialog::defaultBaseObjectSizeEnabled(true);

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

void ObjedMarkerObjectSizeDialog::setBaseObjectSizeEnabled(bool state)
{
  baseObjectSizeGroupBox->setChecked(state);
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

bool ObjedMarkerObjectSizeDialog::isBaseObjectSizeEnabled() const
{
  return baseObjectSizeGroupBox->isChecked();
}

void ObjedMarkerObjectSizeDialog::restoreDefaults()
{
  setBaseObjectSize(defaultBaseObjectSize);
  setMinimumObjectSize(defaultMinimumObjectSize);
  setMaximumObjectSize(defaultMaximumObjectSize);
  setBaseObjectSizeEnabled(defaultBaseObjectSizeEnabled);
}
