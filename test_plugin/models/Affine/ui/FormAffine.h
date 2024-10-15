#ifndef FormAffine_H
#define FormAffine_H

#include <QWidget>
#include <iostream>
#include <QMovie>
#include <QScreen>

#include "ThreadAffine.hpp"
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include "popup/DialogAffine.h"

namespace Ui {
class FormAffine;
}

class FormAffine : public QWidget
{
  Q_OBJECT

public:
  explicit FormAffine(QWidget *parent = 0);
  ~FormAffine();

  bool nodeStatus_enable = true;
  bool nodeStatus_ready = true;
  bool nodeStatus_complete = true;

  qint64 timestamp_base = 0;
  qint64 timestamp_input = -1;

  bool isHaveError = false;

  QString style_nodeDisable = "background-color: transparent; border-image: url(:/test_plugin/icon/node_disable.png); background: none; border: none; background-repeat: none;";
  QString style_nodeEnable = "background-color: transparent; border-image: url(:/test_plugin/icon/node_enable.png); background: none; border: none; background-repeat: none;";

  QMovie *mv_node_run;

  ThreadAffine *workerThread;
  DialogAffine *dialog;

public slots:
  void update_ui();

private slots:
  void stop_node_process();
  void on_pushButton_nodeEnable_clicked();
  void on_pushButton_prop_clicked();

private:
  Ui::FormAffine *ui;
};

#endif // FormAffine_H
