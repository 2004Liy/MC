#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->openGLWidget->setFocus();
    connect(ui->openGLWidget,&MyOpenGL::closemc,[&](){
        close();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
