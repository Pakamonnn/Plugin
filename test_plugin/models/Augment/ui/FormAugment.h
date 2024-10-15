#ifndef FormAugment_H
#define FormAugment_H

#include <QWidget>
#include <iostream>
#include <QMovie>
#include <QScreen>

#include "ThreadAugment.hpp"
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include "popup/DialogAugment.h"

#include "bbbrunaugment.h"

namespace Ui {
class FormAugment;
}

class FormAugment : public QWidget
{
  Q_OBJECT

public:
  explicit FormAugment(QWidget *parent = 0);
  ~FormAugment();

  bool nodeStatus_enable = true;
  bool nodeStatus_ready = true;
  bool nodeStatus_complete = true;

  qint64 timestamp_base = 0;
  qint64 timestamp_input = -1;

  bool isHaveError = false;

  QString style_nodeDisable = "background-color: transparent; border-image: url(:/test_plugin/icon/node_disable.png); background: none; border: none; background-repeat: none;";
  QString style_nodeEnable = "background-color: transparent; border-image: url(:/test_plugin/icon/node_enable.png); background: none; border: none; background-repeat: none;";

  QMovie *mv_node_run;

  ThreadAugment *workerThread;
  DialogAugment *dialog;

public slots:
  void update_ui();

private slots:
  void stop_node_process();
  void on_pushButton_nodeEnable_clicked();
  void on_pushButton_prop_clicked();
  void on_pushButton_RUN_clicked();

private:
  Ui::FormAugment *ui;
};

#endif // FormAugment_H
