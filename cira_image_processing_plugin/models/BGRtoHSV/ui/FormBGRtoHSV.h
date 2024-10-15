#ifndef FormBGRtoHSV_H
#define FormBGRtoHSV_H

#include <QWidget>
#include <iostream>
#include <QMovie>
#include <QScreen>

#include "ThreadBGRtoHSV.hpp"
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include "popup/DialogBGRtoHSV.h"

namespace Ui {
class FormBGRtoHSV;
}

class FormBGRtoHSV : public QWidget
{
  Q_OBJECT

public:
  explicit FormBGRtoHSV(QWidget *parent = 0);
  ~FormBGRtoHSV();

  bool nodeStatus_enable = true;
  bool nodeStatus_ready = true;
  bool nodeStatus_complete = true;

  qint64 timestamp_base = 0;
  qint64 timestamp_input = -1;

  bool isHaveError = false;

  QString style_nodeDisable = "background-color: transparent; border-image: url(:/cira_image_processing_plugin/icon/node_disable.png); background: none; border: none; background-repeat: none;";
  QString style_nodeEnable = "background-color: transparent; border-image: url(:/cira_image_processing_plugin/icon/node_enable.png); background: none; border: none; background-repeat: none;";

  QMovie *mv_node_run;

  ThreadBGRtoHSV *workerThread;
  DialogBGRtoHSV *dialog;

public slots:
  void update_ui();

private slots:
  void stop_node_process();
  void on_pushButton_nodeEnable_clicked();
  void on_pushButton_prop_clicked();

private:
  Ui::FormBGRtoHSV *ui;
};

#endif // FormBGRtoHSV_H
