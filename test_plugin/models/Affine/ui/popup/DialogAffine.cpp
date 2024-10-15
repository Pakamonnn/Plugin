#include "DialogAffine.h"
#include "ui_DialogAffine.h"

#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <cira_lib_bernoulli/datatype/FlowData.hpp>
#include <cira_lib_bernoulli/asm_opencv/asmOpenCV.h>


DialogAffine::DialogAffine(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogAffine)
{
  ui->setupUi(this);

  //cira_std::connectFlowSlot("<flowName>", this, SLOT(getFlow(std::shared_ptr<FlowData>)));

  //  window_id = QUuid::createUuid().toString().toStdString();

}

DialogAffine::~DialogAffine()
{
  delete ui;
}

/*void DialogAffine::getFlow(std::shared_ptr<FlowData> flow) {
  FlowDataType data = flow->getFlowData();
  QJsonObject payload = data._payload_js_data["payload"].toObject();
  cv::Mat mat_im = data._img;
  
}*/

QJsonObject DialogAffine::saveState() {
  QJsonObject param_js_data;

  param_js_data["x1"] = ui->spinBox_p1_x->value();
  param_js_data["y1"] = ui->spinBox_p1_y->value();
  param_js_data["x2"] = ui->spinBox_p2_x->value();
  param_js_data["y2"] = ui->spinBox_p2_y->value();
  param_js_data["x3"] = ui->spinBox_p3_x->value();
  param_js_data["y3"] = ui->spinBox_p3_y->value();
  param_js_data["x4"] = ui->spinBox_p4_x->value();
  param_js_data["y4"] = ui->spinBox_p4_y->value();
  param_js_data["combobox"] = ui->comboBox_type->currentIndex();


  return param_js_data;
}

void DialogAffine::restoreState(QJsonObject param_js_data) {

  ui->spinBox_p1_x->setValue(param_js_data["x1"].toInt());
  ui->spinBox_p1_y->setValue(param_js_data["y1"].toInt());
  ui->spinBox_p2_x->setValue(param_js_data["x2"].toInt());
  ui->spinBox_p2_y->setValue(param_js_data["y2"].toInt());
  ui->spinBox_p3_x->setValue(param_js_data["x3"].toInt());
  ui->spinBox_p3_y->setValue(param_js_data["y3"].toInt());
  ui->spinBox_p4_x->setValue(param_js_data["x4"].toInt());
  ui->spinBox_p4_y->setValue(param_js_data["y4"].toInt());
  ui->comboBox_type->setCurrentIndex(param_js_data["combobox"].toInt());
}

void DialogAffine::on_comboBox_type_currentIndexChanged(int index){
  if(index < 1){
    ui->spinBox_p4_x->setEnabled(true);
    ui->spinBox_p4_y->setEnabled(true);
    ui->label_x4->setEnabled(true);
    ui->label_y4->setEnabled(true);
    ui->groupBox_point4->setEnabled(true);
      }else{
    ui->spinBox_p4_x->setEnabled(false);
    ui->spinBox_p4_y->setEnabled(false);
    ui->label_x4->setEnabled(false);
    ui->label_y4->setEnabled(false);
    ui->groupBox_point4->setEnabled(false);

  }
}

void DialogAffine::on_pushButton_clicked(){

  if(tmp_im.empty()){
    return;
  }
  mtx.lock();
  cv::Mat mat = tmp_im.clone();
  mtx.unlock();

  DialogGetPnts *dlg_get_pnts = new DialogGetPnts(this);

  dlg_get_pnts->max_pts = 99;
  dlg_get_pnts->is_get_area = true;
  dlg_get_pnts->g_point.clear();
  dlg_get_pnts->src = mat;
  dlg_get_pnts->ok = false;

  window_id = QUuid::createUuid().toString().toStdString();
  dlg_get_pnts->window_name = window_id;

  dlg_get_pnts->setWindowTitle("Get Points");
  dlg_get_pnts->show();

  cv::namedWindow(window_id, cv::WINDOW_KEEPRATIO | cv::WINDOW_GUI_NORMAL);
  cira_std::setOpenCVWindowToWidget(QString::fromStdString(window_id), dlg_get_pnts->img_widget);

  cv::setMouseCallback(window_id, DialogGetPnts::on_MouseHandle, dlg_get_pnts);
  cv::imshow(window_id, mat);

  dlg_get_pnts->restore_gpoint(g_point_tmp);
  dlg_get_pnts->init();

  //auto resize 500ms in the future
  QTimer::singleShot(500, [this, dlg_get_pnts](){
    int w = dlg_get_pnts->width();
    int h = dlg_get_pnts->height();
    dlg_get_pnts->resize(w+5, h+5);
    dlg_get_pnts->resize(w, h);
  });

  dlg_get_pnts->exec();

  cv::setMouseCallback(window_id, NULL, NULL);
  cv::destroyWindow(window_id);

  int index = ui->comboBox_type->currentIndex();
  if(index==1){
    if(dlg_get_pnts->ok && dlg_get_pnts->g_point.size() < 3) {
      QMessageBox msgError;
      msgError.setText("ERROR!\nYou must click 3 points!");
      msgError.setIcon(QMessageBox::Critical);
      msgError.setWindowTitle("Error");
      msgError.exec();
      return;
    }

    //get points after finish
    if(dlg_get_pnts->ok) {
      g_point_tmp = dlg_get_pnts->g_point;
      ui->spinBox_p1_x->setValue(g_point_tmp[0].x);
      ui->spinBox_p1_y->setValue(g_point_tmp[0].y);
      ui->spinBox_p2_x->setValue(g_point_tmp[1].x);
      ui->spinBox_p2_y->setValue(g_point_tmp[1].y);
      ui->spinBox_p3_x->setValue(g_point_tmp[2].x);
      ui->spinBox_p3_y->setValue(g_point_tmp[2].y);

    }
  }else{
    if(dlg_get_pnts->ok && dlg_get_pnts->g_point.size() < 4) {
      QMessageBox msgError;
      msgError.setText("ERROR!\nYou must click 4 points!");
      msgError.setIcon(QMessageBox::Critical);
      msgError.setWindowTitle("Error");
      msgError.exec();
      return;
    }

    //get points after finish
    if(dlg_get_pnts->ok) {
      g_point_tmp = dlg_get_pnts->g_point;
      ui->spinBox_p1_x->setValue(g_point_tmp[0].x);
      ui->spinBox_p1_y->setValue(g_point_tmp[0].y);
      ui->spinBox_p2_x->setValue(g_point_tmp[1].x);
      ui->spinBox_p2_y->setValue(g_point_tmp[1].y);
      ui->spinBox_p3_x->setValue(g_point_tmp[2].x);
      ui->spinBox_p3_y->setValue(g_point_tmp[2].y);
      ui->spinBox_p4_x->setValue(g_point_tmp[3].x);
      ui->spinBox_p4_y->setValue(g_point_tmp[3].y);
    }
  }


}
