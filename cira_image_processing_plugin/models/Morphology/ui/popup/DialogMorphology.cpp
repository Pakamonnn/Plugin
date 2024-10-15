#include "DialogMorphology.h"
#include "ui_DialogMorphology.h"

DialogMorphology::DialogMorphology(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogMorphology)
{
  ui->setupUi(this);
}

DialogMorphology::~DialogMorphology()
{
  delete ui;
}

QJsonObject DialogMorphology::saveState() {
  QJsonObject param_js_data;
  param_js_data["ComboBox"] = ui->comboBox->currentText();
  param_js_data["Kernel"] = ui->spinBox->value();
  param_js_data["Iterations"] = ui->spinBox_2->value();

  return param_js_data;
}

void DialogMorphology::restoreState(QJsonObject param_js_data) {
  ui->comboBox->setCurrentText(param_js_data["ComboBox"].toString());
  ui->spinBox->setValue(param_js_data["Kernel"].toInt());
  ui->spinBox_2->setValue(param_js_data["Iterations"].toInt());
}
