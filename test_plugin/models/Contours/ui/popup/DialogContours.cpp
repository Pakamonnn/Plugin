#include "DialogContours.h"
#include "ui_DialogContours.h"

#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <cira_lib_bernoulli/datatype/FlowData.hpp>
#include <cira_lib_bernoulli/asm_opencv/asmOpenCV.h>

DialogContours::DialogContours(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogContours)
{
  ui->setupUi(this);
  
  //cira_std::connectFlowSlot("<flowName>", this, SLOT(getFlow(std::shared_ptr<FlowData>)));
  
}

DialogContours::~DialogContours()
{
  delete ui;
}

/*void DialogContours::getFlow(std::shared_ptr<FlowData> flow) {
  FlowDataType data = flow->getFlowData();
  QJsonObject payload = data._payload_js_data["payload"].toObject();
  cv::Mat mat_im = data._img;
  
}*/

QJsonObject DialogContours::saveState() {
  QJsonObject param_js_data;

  param_js_data["contourtype"] = ui->comboBox_contourtype->currentIndex();
  param_js_data["boundingtype"] = ui->comboBox_boundingtype->currentIndex();
  param_js_data["puttext"] = ui->groupBox_puttext->isChecked();
  param_js_data["sizetext"] = ui->doubleSpinBox_sizetext->value();
  param_js_data["thickness"] = ui->spinBox_thickness->value();
  param_js_data["methodcontour"] = ui->comboBox_methodcontour->currentIndex();

  return param_js_data;
}

void DialogContours::restoreState(QJsonObject param_js_data) {

  ui->comboBox_contourtype->setCurrentIndex(param_js_data["contourtype"].toInt());
  ui->comboBox_boundingtype->setCurrentIndex(param_js_data["boundingtype"].toInt());
  ui->groupBox_puttext->setChecked(param_js_data["puttext"].toBool());
  ui->doubleSpinBox_sizetext->setValue(param_js_data["sizetext"].toInt());
  ui->spinBox_thickness->setValue(param_js_data["thickness"].toInt());
  ui->comboBox_methodcontour->setCurrentIndex(param_js_data["methodcontour"].toInt());

}
