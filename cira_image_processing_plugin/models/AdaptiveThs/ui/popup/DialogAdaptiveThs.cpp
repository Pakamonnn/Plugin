#include "DialogAdaptiveThs.h"
#include "ui_DialogAdaptiveThs.h"

DialogAdaptiveThs::DialogAdaptiveThs(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogAdaptiveThs)
{
  ui->setupUi(this);
}

DialogAdaptiveThs::~DialogAdaptiveThs()
{
  delete ui;
}

QJsonObject DialogAdaptiveThs::saveState() {
  QJsonObject param_js_data;

  param_js_data["ComboBox"] = ui->comboBox->currentText();
  param_js_data["blockSize"] = ui->spinBox->value();
  param_js_data["c"] = ui->spinBox_2->value();

  return param_js_data;
}

void DialogAdaptiveThs::restoreState(QJsonObject param_js_data) {
  ui->comboBox->setCurrentText(param_js_data["ComboBox"].toString());
  ui->spinBox->setValue(param_js_data["blockSize"].toInt());
  ui->spinBox_2->setValue(param_js_data["c"].toInt());
}

void DialogAdaptiveThs::on_spinBox_valueChanged(int arg1)
{
  if( arg1 % 2 == 0)
      {
          ui->spinBox->setValue( arg1 - 1 );
      }
}
