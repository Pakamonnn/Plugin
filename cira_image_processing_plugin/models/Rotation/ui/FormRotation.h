#ifndef FormRotation_H
#define FormRotation_H

#include <QWidget>
#include <iostream>
#include <QMovie>
#include <QScreen>

#include "ThreadRotation.hpp"
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include "popup/DialogRotation.h"

namespace Ui {
class FormRotation;
}

class FormRotation : public QWidget
{
  Q_OBJECT

public:
  explicit FormRotation(QWidget *parent = 0);
  ~FormRotation();

  bool nodeStatus_enable = true;
  bool nodeStatus_ready = true;
  bool nodeStatus_complete = true;

  qint64 timestamp_base = 0;
  qint64 timestamp_input = -1;

  bool isHaveError = false;

  QString style_nodeDisable = "background-color: transparent; border-image: url(:/cira_image_processing_plugin/icon/node_disable.png); background: none; border: none; background-repeat: none;";
  QString style_nodeEnable = "background-color: transparent; border-image: url(:/cira_image_processing_plugin/icon/node_enable.png); background: none; border: none; background-repeat: none;";

  QMovie *mv_node_run;

  ThreadRotation *workerThread;
  DialogRotation *dialog;

public slots:
  void update_ui();

private slots:
  void stop_node_process();
  void on_pushButton_nodeEnable_clicked();
  void on_pushButton_prop_clicked();

private:
  Ui::FormRotation *ui;
};

#endif // FormRotation_H
