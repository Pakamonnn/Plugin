#ifndef DialogAffine_H
#define DialogAffine_H

#include <QDialog>
#include <QJsonObject>
#include <QEvent>
#include<QMutex>
#include <iostream>
#include <QTimer>
#include <QMessageBox>

//#include<opencv2/opencv.hpp>
//#include<QWindow>

#include <cira_std_lib/cira_std_lib.hpp>

#include "dialog_get_points/DialogGetPnts.h"

namespace Ui {
class DialogAffine;
}

class DialogAffine : public QDialog
{
  Q_OBJECT

public:
  explicit DialogAffine(QWidget *parent = 0);
  ~DialogAffine();

  void changeEvent(QEvent *ev) {
    if (ev->type() == QEvent::ActivationChange)
    {
        if(this->isActiveWindow())
        {

        }
        else
        {
          if(!isPin) {
            this->close();
          }
        }
    }
  }

  bool isPin = true;

  QString style_unpin = "background-color: transparent; border-image: url(:/test_plugin/icon/unpin.png); background: none; border: none; background-repeat: none;";
  QString style_pin = "background-color: transparent; border-image: url(:/test_plugin/icon/pin.png); background: none; border: none; background-repeat: none;";

  QMutex mtx;
  cv::Mat tmp_im;

  std::string window_id;
  std::vector<cv::Point> g_point_tmp;

  QMap<QString,cv::Point> map_pnts;

  QJsonObject saveState();
  void restoreState(QJsonObject param_js_data);

//private slots:
//  void getFlow(std::shared_ptr<FlowData> flow);;
private slots:
  void on_comboBox_type_currentIndexChanged(int index);

//  QWindow*tmp_window;

private slots:
  void on_pushButton_clicked();


private:
  Ui::DialogAffine *ui;
};

#endif // DialogAffine_H
