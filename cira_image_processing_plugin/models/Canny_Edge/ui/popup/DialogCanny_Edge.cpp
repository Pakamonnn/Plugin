#include "DialogCanny_Edge.h"
#include "ui_DialogCanny_Edge.h"

DialogCanny_Edge::DialogCanny_Edge(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogCanny_Edge)
{
  ui->setupUi(this);
}

DialogCanny_Edge::~DialogCanny_Edge()
{
  delete ui;
}

QJsonObject DialogCanny_Edge::saveState() {
  QJsonObject param_js_data;

  param_js_data["Lower"] = ui->spinBox->value();
  param_js_data["Upper"] = ui->spinBox_2->value();
  param_js_data["Kernel"] = ui->spinBox_3->value();

  return param_js_data;
}

void DialogCanny_Edge::restoreState(QJsonObject param_js_data) {

  ui->spinBox->setValue(param_js_data["Lower"].toInt());
  ui->spinBox_2->setValue(param_js_data["Upper"].toInt());
  ui->spinBox_3->setValue(param_js_data["Kernel"].toInt());
}

void DialogCanny_Edge::on_spinBox_3_valueChanged(int arg1)
{
  if( arg1 % 2 == 0)
      {
          ui->spinBox_3->setValue( arg1 - 1 );
      }
}
