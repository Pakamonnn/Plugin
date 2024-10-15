#include "DialogBGRtoHSV.h"
#include "ui_DialogBGRtoHSV.h"

DialogBGRtoHSV::DialogBGRtoHSV(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogBGRtoHSV)
{
  ui->setupUi(this);
}

DialogBGRtoHSV::~DialogBGRtoHSV()
{
  delete ui;
}

QJsonObject DialogBGRtoHSV::saveState() {
  QJsonObject param_js_data;

  return param_js_data;
}

void DialogBGRtoHSV::restoreState(QJsonObject param_js_data) {

}
