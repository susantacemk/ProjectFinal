#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QDateTime>
#include <QTimer>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Assuming `this` is a QMainWindow
    QLabel *dateTimeLabel = new QLabel(this);  // Create a QLabel to display date and time
    ui->statusBar->addPermanentWidget(dateTimeLabel);  // Add it to the status bar

    // Create a timer to update the date and time every second
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=]() {
        QString dateTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        dateTimeLabel->setText(dateTimeString);
    });
    timer->start(1000);  // Update every 1 second (1000 ms)

    ui->tableView->setVisible(false);
    ui->widget_3->setMinimumWidth(200);
    ui->widget_3->setMaximumWidth(350);
}

MainWindow::~MainWindow()
{
    delete ui;
}
