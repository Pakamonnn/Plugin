#ifndef Find_EdgeModel_HPP
#define Find_EdgeModel_HPP

#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QShowEvent>

#include <cira_lib_bernoulli/general/GlobalData.hpp>

#include <nodes/NodeDataModel>

#include <iostream>

#include "ui/FormFind_Edge.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

class FlowData;

class Find_EdgeModel : public NodeDataModel
{

  Q_OBJECT

public:
  Find_EdgeModel();

  virtual
  ~Find_EdgeModel() {}

  int portInStatus[1] = {PORTSTATUS::DISCONNECTED};

public:

  QString
  caption() const override
  { return QStringLiteral("Find_Edge"); }

  bool
  captionVisible() const override
  { return true; }

  bool
  portCaptionVisible(PortType, PortIndex) const override
  { return true; }

  bool
  resizable() const override { return false; }


  QString
  name() const override
  { return QStringLiteral("Find_Edge"); }

public:

  QJsonObject
  save() const override;

  void
  restore(QJsonObject const &p) override;

public:

  unsigned int
  nPorts(PortType portType) const override;

  NodeDataType
  dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData>
  outData(PortIndex port) override;

  void
  setInData(std::shared_ptr<NodeData> data, PortIndex portIndex) override;

  QWidget *
  embeddedWidget() override { return form; }

private:

  bool isBusy = false;
  std::shared_ptr<FlowData> _flowDataOut;
  FormFind_Edge *form;

  QTimer *timerLoadFromButton;

private slots:
  void runProcess(std::shared_ptr<FlowData> _flowDataIn);
  void loopTimerLoadFromButton();

};

#endif // Find_EdgeModel_HPP
