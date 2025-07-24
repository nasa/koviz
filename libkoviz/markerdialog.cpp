#include "markerdialog.h"

MarkerDialog::MarkerDialog(double defaultTime, QWidget *parent) :
    QDialog(parent)

{
    auto *layout = new QFormLayout(this);
    _labelEdit = new QLineEdit(this);
    _timeEdit = new QDoubleSpinBox(this);
    _timeEdit->setRange(-1e10, 1e10);
    _timeEdit->setDecimals(6);
    _timeEdit->setValue(defaultTime);

    layout->addRow("Label:", _labelEdit);
    layout->addRow("Time:", _timeEdit);

    auto *buttons = new QDialogButtonBox(
                               QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString MarkerDialog::label() const
{
    return _labelEdit->text();
}

double MarkerDialog::time() const
{
    return _timeEdit->value();
}
