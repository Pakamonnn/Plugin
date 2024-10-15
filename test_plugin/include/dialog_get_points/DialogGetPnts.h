#ifndef ASDFQOPIUKLJWT_H
#define ASDFQOPIUKLJWT_H

#include <QDialog>
#include <QWidget>
#include <QDebug>
#include <QToolTip>

#include <QDebug>
#include <opencv2/opencv.hpp>

namespace Ui {
class DialogGetPnts;
}

class DialogGetPnts : public QDialog
{
  Q_OBJECT

public:
  explicit DialogGetPnts(QWidget *parent = 0);
  ~DialogGetPnts();

  QWidget *img_widget;

  std::vector<cv::Scalar> color_pts;

  void init();
  std::string window_name;
  bool ok = false;
  cv::Mat src;
  std::vector<cv::Point> g_point;

  bool g_bDrawingBox = false;
  static void on_MouseHandle(int event, int x, int y, int flags, void *param);
  void on_MouseHandle(int event, int x, int y, void *param, int flags);
  void mouse_addRect(int event, int x, int y);
  void draw();
  void restore_gpoint(std::vector<cv::Point> pnts);

  bool is_get_area = false;
  int max_pts = 4;

  int id_g_points = 0;

private slots:
  void on_pushButton_ok_clicked();

  void on_pushButton_cancel_clicked();

private:
  Ui::DialogGetPnts *ui;
};

#endif // DialogGetPnts_ADHOC_H
