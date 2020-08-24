#include "mainwindow.h"
#include "ui_mainwindow.h"



#include <iostream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

	QObject::connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(onPushButtonClick()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onPushButtonClick() {
	Ping::PingResponse pingResponse;

	bool const result = m_ping.ping(ui->lineEdit->text(), 2000, pingResponse);
}
