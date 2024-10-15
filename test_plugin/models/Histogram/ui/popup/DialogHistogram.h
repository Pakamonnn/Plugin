#ifndef DialogHistogram_H
#define DialogHistogram_H

#include <QDialog>
#include <QJsonObject>
#include <QEvent>

#include <iostream>

//#include <cira_std_lib/cira_std_lib.hpp>

namespace Ui {
class DialogHistogram;
}

class DialogHistogram : public QDialog
{
  Q_OBJECT

public:
  explicit DialogHistogram(QWidget *parent = 0);
  ~DialogHistogram();

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

  QJsonObject saveState();
  void restoreState(QJsonObject param_js_data);

//private slots:
//  void getFlow(std::shared_ptr<FlowData> flow);

private slots:
  void on_groupBox_clahe_clicked(bool checked);
  
private:
  Ui::DialogHistogram *ui;
};

#endif // DialogHistogram_H
