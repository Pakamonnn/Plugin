#include "DialogHistogram.h"
#include "ui_DialogHistogram.h"

#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <cira_lib_bernoulli/datatype/FlowData.hpp>
#include <cira_lib_bernoulli/asm_opencv/asmOpenCV.h>

DialogHistogram::DialogHistogram(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogHistogram)
{
  ui->setupUi(this);
  
  //cira_std::connectFlowSlot("<flowName>", this, SLOT(getFlow(std::shared_ptr<FlowData>)));
  
}

DialogHistogram::~DialogHistogram()
{
  delete ui;
}

/*void DialogHistogram::getFlow(std::shared_ptr<FlowData> flow) {
  FlowDataType data = flow->getFlowData();
  QJsonObject payload = data._payload_js_data["payload"].toObject();
  cv::Mat mat_im = data._img;
  
}*/

QJsonObject DialogHistogram::saveState() {
  QJsonObject param_js_data;

  param_js_data["histogram"] = ui->checkBox_histogram->isChecked();
  param_js_data["clahe"] = ui->groupBox_clahe->isChecked();
  param_js_data["cliplimit"] = ui->spinBox_cliplimit->value();
  param_js_data["gridsize"] = ui->comboBox_gridsize->currentIndex();
  param_js_data["graph"] = ui->comboBox_graph->currentText();

  return param_js_data;
}

void DialogHistogram::restoreState(QJsonObject param_js_data) {

  ui->checkBox_histogram->setChecked(param_js_data["histogram"].toBool());
  ui->groupBox_clahe->setChecked(param_js_data["clahe"].toBool());
  ui->spinBox_cliplimit->setValue(param_js_data["cliplimit"].toInt());
  ui->comboBox_gridsize->setCurrentIndex(param_js_data["gridsize"].toInt());
  ui->comboBox_graph->setCurrentText(param_js_data["graph"].toString());

}

void DialogHistogram::on_groupBox_clahe_clicked(bool checked){
  if(checked==true){
    ui->checkBox_histogram->setEnabled(false);
  }else{
    ui->checkBox_histogram->setEnabled(true);
  }
}
