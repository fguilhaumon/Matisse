﻿#include "ExpertFormWidget.h"
#include "ui_ExpertFormWidget.h"


using namespace MatisseTools;
using namespace MatisseServer;


ExpertFormWidget::ExpertFormWidget(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::ExpertFormWidget)//,
    //_server(NULL)
{
    _ui->setupUi(this);
    init();
}

ExpertFormWidget::~ExpertFormWidget()
{
    delete _ui;
}

void ExpertFormWidget::init()
{
    _ui->_GRW_assembly->setEnabled(false);
    _ui->_GRW_assembly->setAcceptDrops(true);

    QRect containerRect = _ui->_GRW_assembly->rect();

    _scene = new AssemblyGraphicsScene(containerRect);
    _ui->_GRW_assembly->setScene(_scene);
    _ui->_GRW_assembly->centerOn(0, 0);
    _scene->initViewport();
}

void ExpertFormWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    qDebug() << "ExpertFormWidget Resize event : " << event->oldSize() << event->size();
}

//void ExpertFormWidget::setServer(Server *server) {
//    _server = server;
//    _scene->setServer(server);
//}


QGraphicsView *ExpertFormWidget::getGraphicsView()
{
    return _ui->_GRW_assembly;
}


bool ExpertFormWidget::loadAssembly(QString assemblyName)
{
    qDebug() << "Load assembly:" << assemblyName;
    return _scene->loadAssembly(assemblyName);

}

void ExpertFormWidget::resetAssemblyForm()
{
    _scene->reset();
    _ui->_GRW_assembly->invalidateScene();
    _ui->_GRW_assembly->update();
}
