#ifndef FormMorphology_H
#define FormMorphology_H

#include <QWidget>
#include <iostream>
#include <QMovie>
#include <QScreen>

#include "ThreadMorphology.hpp"
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include "popup/DialogMorphology.h"

namespace Ui {
class FormMorphology;
}

class FormMorphology : public QWidget
{
  Q_OBJECT

public:
  explicit FormMorphology(QWidget *parent = 0);
  ~FormMorphology();

  bool nodeStatus_enable = true;
  bool nodeStatus_ready = true;
  bool nodeStatus_complete = true;

  qint64 timestamp_base = 0;
  qint64 timestamp_input = -1;

  bool isHaveError = false;

  QString style_nodeDisable = "background-color: transparent; border-image: url(:/cira_image_processing_plugin/icon/node_disable.png); background: none; border: none; background-repeat: none;";
  QString style_nodeEnable = "background-color: transparent; border-image: url(:/cira_image_processing_plugin/icon/node_enable.png); background: none; border: none; background-repeat: none;";

  QMovie *mv_node_run;

  ThreadMorphology *workerThread;
  DialogMorphology *dialog;

public slots:
  void update_ui();

private slots:
  void stop_node_process();
  void on_pushButton_nodeEnable_clicked();
  void on_pushButton_prop_clicked();

private:
  Ui::FormMorphology *ui;
};

#endif // FormMorphology_H
