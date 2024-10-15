#include "DialogFlip.h"
#include "ui_DialogFlip.h"

DialogFlip::DialogFlip(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogFlip)
{
  ui->setupUi(this);
}

DialogFlip::~DialogFlip()
{
  delete ui;
}

QJsonObject DialogFlip::saveState() {
  QJsonObject param_js_data;

  param_js_data["ComboBox"] = ui->comboBox->currentText();

  return param_js_data;
}

void DialogFlip::restoreState(QJsonObject param_js_data) {
  ui->comboBox->setCurrentText(param_js_data["ComboBox"].toString());
}
