#include "DialogResize.h"
#include "ui_DialogResize.h"

DialogResize::DialogResize(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogResize)
{
  ui->setupUi(this);
}

DialogResize::~DialogResize()
{
  delete ui;
}

QJsonObject DialogResize::saveState() {
  QJsonObject param_js_data;

  param_js_data["W"] = ui->spinBox->value();
  param_js_data["H"] = ui->spinBox_2->value();
  param_js_data["CheckBox"] = ui->checkBox->isChecked();
  return param_js_data;
}

void DialogResize::restoreState(QJsonObject param_js_data) {

  ui->spinBox->setValue(param_js_data["W"].toInt());
  ui->spinBox_2->setValue(param_js_data["H"].toInt());
  ui->checkBox->setChecked(param_js_data["CheckBox"].toBool());

}


