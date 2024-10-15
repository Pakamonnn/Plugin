#include "DialogBinaryImage.h"
#include "ui_DialogBinaryImage.h"

DialogBinaryImage::DialogBinaryImage(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogBinaryImage)
{
  ui->setupUi(this);
}

DialogBinaryImage::~DialogBinaryImage()
{
  delete ui;
}

QJsonObject DialogBinaryImage::saveState() {
  QJsonObject param_js_data;

  param_js_data["Threshold"] = ui->spinBox->value();
  param_js_data["ComboBox"] = ui->comboBox->currentText();
  param_js_data["GroupBox"] = ui->groupBox->isChecked();
  param_js_data["Threshold_Bar"] = ui->horizontalSlider->value();

  return param_js_data;
}

void DialogBinaryImage::restoreState(QJsonObject param_js_data) {

  ui->spinBox->setValue(param_js_data["Threshold"].toInt());
  ui->comboBox->setCurrentText(param_js_data["ComboBox"].toString());
  ui->groupBox->setChecked(param_js_data["GroupBox"].toBool());
  ui->horizontalSlider->setValue(param_js_data["Threshold_Bar"].toInt());
}

void DialogBinaryImage::on_groupBox_clicked(bool checked)
{
  if(checked == true){
    ui->label->setEnabled(false);
    ui->label_2->setEnabled(false);
    ui->label_3->setEnabled(false);
    ui->spinBox->setEnabled(false);
  }
  else{
    ui->label->setEnabled(true);
    ui->label_2->setEnabled(true);
    ui->label_3->setEnabled(true);
    ui->spinBox->setEnabled(true);
  }

}

void DialogBinaryImage::on_horizontalSlider_valueChanged(int value)
{
    ui->label_7->setNum(value);
}
