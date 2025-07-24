#ifndef MARKERDIALOG_H
#define MARKERDIALOG_H

#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>

class MarkerDialog : public QDialog
{
  Q_OBJECT
  public:
    MarkerDialog(double defaultTime = 0.0,QWidget* parent = nullptr);
    QString label() const;
    double time() const;

  private:
    QLineEdit *_labelEdit;
    QDoubleSpinBox *_timeEdit;

};

#endif // MARKERDIALOG_H
