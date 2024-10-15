#include "DialogGetPnts.h"
#include "ui_DialogGetPnts.h"
#include <QMessageBox>
#include <QThread>

DialogGetPnts::DialogGetPnts(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogGetPnts)
{
  ui->setupUi(this);
  img_widget = ui->widget;

  //this->setWindowFlag(Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint );

  ui->label_help->setToolTipDuration(5000);
  color_pts.push_back(cv::Scalar(3, 3, 252));
  color_pts.push_back(cv::Scalar(3, 252, 115));
  color_pts.push_back(cv::Scalar(252, 49, 3));
  color_pts.push_back(cv::Scalar(198, 3, 252));

  color_pts.push_back(cv::Scalar(22, 172, 247));
  color_pts.push_back(cv::Scalar(169, 247, 22));
  color_pts.push_back(cv::Scalar(247, 22, 75));
  color_pts.push_back(cv::Scalar(225, 22, 247));
}

DialogGetPnts::~DialogGetPnts()
{
  delete ui;
}

void DialogGetPnts::init() {
  ui->pushButton_ok->setFocus();
}

void DialogGetPnts::DialogGetPnts::mouse_addRect(int event, int x, int y) {

    //std::cout << g_bDrawingBox << std::endl;
    if(event == cv::EVENT_MOUSEMOVE) {
    }
    else if(event == cv::EVENT_LBUTTONDOWN) {

        g_bDrawingBox = true;
        g_point.push_back(cv::Point(x,y));
    }
}

void DialogGetPnts::DialogGetPnts::on_MouseHandle(int event, int x, int y, int flag, void *param) {
    DialogGetPnts *self = static_cast<DialogGetPnts*>(param);
    self->on_MouseHandle(event, x, y, param, flag);
}

void DialogGetPnts::DialogGetPnts::on_MouseHandle(int event, int x, int y, void *param, int flags) {
  if(event == cv::EVENT_RBUTTONDOWN) {
    if(g_point.size() > 0) {
      g_point.pop_back();
      draw();
    }
  }
  else if(event == cv::EVENT_LBUTTONDOWN) {

      g_bDrawingBox = true;
      if(g_point.size() < max_pts) {
        g_point.push_back(cv::Point(x,y));
      }

      draw();
  }
}


void DialogGetPnts::restore_gpoint(std::vector<cv::Point> pnts){
    for(int i = 0; i < pnts.size(); i++) {
        g_point.push_back(cv::Point(pnts[i].x,pnts[i].y));
    }
    draw();
}

void DialogGetPnts::DialogGetPnts::draw() {
  int pt_size = src.cols / 210.0;
  cv::Mat tmp = src.clone();

  for(int i = 0; i < g_point.size(); i++) {
    cv::line(tmp, g_point[i], g_point[(i+1)%g_point.size()], cv::Scalar(252, 248, 3), pt_size);
  }

  for(int i = 0; i < g_point.size(); i++) {
    cv::circle(tmp, g_point[i], pt_size*2, color_pts[i%color_pts.size()], pt_size);
  }

  cv::imshow(window_name, tmp);
}

void DialogGetPnts::on_pushButton_ok_clicked()
{
  if(!is_get_area && g_point.size() < max_pts) return;
  //else if(g_point.size() < 4) return;
  ok = true;
  qDebug() << g_point.size();
  this->close();
}

void DialogGetPnts::on_pushButton_cancel_clicked()
{
  ok = false;
  this->close();
}
