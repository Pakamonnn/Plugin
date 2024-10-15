#include "DialogGrayScale.h"
#include "ui_DialogGrayScale.h"

DialogGrayScale::DialogGrayScale(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogGrayScale)
{
  ui->setupUi(this);
}

DialogGrayScale::~DialogGrayScale()
{
  delete ui;
}

QJsonObject DialogGrayScale::saveState() {
  QJsonObject param_js_data;

  return param_js_data;
}

void DialogGrayScale::restoreState(QJsonObject param_js_data) {

}
