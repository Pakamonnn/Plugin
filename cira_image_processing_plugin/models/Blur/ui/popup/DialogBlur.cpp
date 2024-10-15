#include "DialogBlur.h"
#include "ui_DialogBlur.h"

DialogBlur::DialogBlur(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogBlur)
{
  ui->setupUi(this);
}

DialogBlur::~DialogBlur()
{
  delete ui;
}

QJsonObject DialogBlur::saveState() {
  QJsonObject param_js_data;

  param_js_data["Kernel"] = ui->spinBox->value();
  param_js_data["ComboBox"] = ui->comboBox->currentText();
  //param_js_data["GroupBox"] = ui->groupBox->isChecked();
  //param_js_data["Threshold_Bar"] = ui->horizontalSlider->value();

  return param_js_data;
}

void DialogBlur::restoreState(QJsonObject param_js_data) {

  ui->spinBox->setValue(param_js_data["Kernel"].toInt());
  ui->comboBox->setCurrentText(param_js_data["ComboBox"].toString());

}

void DialogBlur::on_spinBox_valueChanged(int arg1)
{
  if( arg1 % 2 == 0)
      {
          ui->spinBox->setValue( arg1 - 1 );
      }
}
