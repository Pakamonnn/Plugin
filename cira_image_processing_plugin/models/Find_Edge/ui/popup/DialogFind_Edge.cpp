#include "DialogFind_Edge.h"
#include "ui_DialogFind_Edge.h"

DialogFind_Edge::DialogFind_Edge(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogFind_Edge)
{
  ui->setupUi(this);
}

DialogFind_Edge::~DialogFind_Edge()
{
  delete ui;
}

QJsonObject DialogFind_Edge::saveState() {
  QJsonObject param_js_data;

  param_js_data["ComboBox"] = ui->comboBox->currentText();
  param_js_data["Kernel"] = ui->spinBox->value();

  return param_js_data;
}

void DialogFind_Edge::restoreState(QJsonObject param_js_data) {

  ui->comboBox->setCurrentText(param_js_data["ComboBox"].toString());
  ui->spinBox->setValue(param_js_data["Kernel"].toInt());
}

void DialogFind_Edge::on_spinBox_valueChanged(int arg1)
{
  if( arg1 % 2 == 0)
      {
          ui->spinBox->setValue( arg1 - 1 );
      }
}

