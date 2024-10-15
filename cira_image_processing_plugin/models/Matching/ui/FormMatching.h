#ifndef FormMatching_H
#define FormMatching_H

#include <QWidget>
#include <iostream>
#include <QMovie>
#include <QScreen>

#include "ThreadMatching.hpp"
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include "popup/DialogMatching.h"

namespace Ui {
class FormMatching;
}

class FormMatching : public QWidget
{
  Q_OBJECT

public:
  explicit FormMatching(QWidget *parent = 0);
  ~FormMatching();

  bool nodeStatus_enable = true;
  bool nodeStatus_ready = true;
  bool nodeStatus_complete = true;

  qint64 timestamp_base = 0;
  qint64 timestamp_input = -1;

  bool isHaveError = false;

  QString style_nodeDisable = "background-color: transparent; border-image: url(:/cira_image_processing_plugin/icon/node_disable.png); background: none; border: none; background-repeat: none;";
  QString style_nodeEnable = "background-color: transparent; border-image: url(:/cira_image_processing_plugin/icon/node_enable.png); background: none; border: none; background-repeat: none;";

  QMovie *mv_node_run;

  ThreadMatching *workerThread;
  DialogMatching *dialog;

public slots:
  void update_ui();

private slots:
  void stop_node_process();
  void on_pushButton_nodeEnable_clicked();
  void on_pushButton_prop_clicked();

private:
  Ui::FormMatching *ui;
};

#endif // FormMatching_H
