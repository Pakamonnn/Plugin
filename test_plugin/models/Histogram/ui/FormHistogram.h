#ifndef FormHistogram_H
#define FormHistogram_H

#include <QWidget>
#include <iostream>
#include <QMovie>
#include <QScreen>

#include "ThreadHistogram.hpp"
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include "popup/DialogHistogram.h"

namespace Ui {
class FormHistogram;
}

class FormHistogram : public QWidget
{
  Q_OBJECT

public:
  explicit FormHistogram(QWidget *parent = 0);
  ~FormHistogram();

  bool nodeStatus_enable = true;
  bool nodeStatus_ready = true;
  bool nodeStatus_complete = true;

  qint64 timestamp_base = 0;
  qint64 timestamp_input = -1;

  bool isHaveError = false;

  QString style_nodeDisable = "background-color: transparent; border-image: url(:/test_plugin/icon/node_disable.png); background: none; border: none; background-repeat: none;";
  QString style_nodeEnable = "background-color: transparent; border-image: url(:/test_plugin/icon/node_enable.png); background: none; border: none; background-repeat: none;";

  QMovie *mv_node_run;

  ThreadHistogram *workerThread;
  DialogHistogram *dialog;

public slots:
  void update_ui();

private slots:
  void stop_node_process();
  void on_pushButton_nodeEnable_clicked();
  void on_pushButton_prop_clicked();

private:
  Ui::FormHistogram *ui;
};

#endif // FormHistogram_H
