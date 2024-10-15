#include "DialogRotation.h"
#include "ui_DialogRotation.h"

DialogRotation::DialogRotation(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogRotation)
{
  ui->setupUi(this);

}

DialogRotation::~DialogRotation()
{
  delete ui;
}

QJsonObject DialogRotation::saveState() {
  QJsonObject param_js_data;

  param_js_data["Degree"] = ui->spinBox->value();
  param_js_data["CheckBox"] = ui->groupBox->isChecked();
  param_js_data["SliderBar"] = ui->horizontalSlider->value();

  return param_js_data;
}

void DialogRotation::restoreState(QJsonObject param_js_data) {

  ui->spinBox->setValue(param_js_data["Degree"].toInt());
  ui->groupBox->setChecked(param_js_data["CheckBox"].toBool());
  ui->horizontalSlider->setValue(param_js_data["SliderBar"].toInt());

}

void DialogRotation::on_groupBox_clicked(bool checked)
{
  if(checked == true){
    ui->label->setEnabled(false);
    ui->spinBox->setEnabled(false);
  }
  else{
    ui->label->setEnabled(true);
    ui->spinBox->setEnabled(true);
  }


}

void DialogRotation::on_horizontalSlider_valueChanged(int value)
{
    ui->label_2->setNum(value);
}
