#include "connectwindow.h"
#include "ui_connectwindow.h"

ConnectWindow::ConnectWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::ConnectWindow)
{
    ui->setupUi(this);
    ui->lStatus->setStyleSheet("QLabel {color : black; }");

    connect(ui->bConnect, &QPushButton::clicked, this, &ConnectWindow::connectPressed);
}

void ConnectWindow::connectPressed()
{
    emit ConnectToServer(ui->leServerAdress->text(), ui->leServerPort->text(), ui->leUserName->text());
}

void ConnectWindow::onChangeStatus(QString status)
{
    ui->lStatus->setStyleSheet("QLabel {color : black; }");
    ui->lStatus->setText(status);
}

void ConnectWindow::onShowError(int number, QString msg)
{
    ui->lStatus->setStyleSheet("QLabel {color : red; }");
    ui->lStatus->setText(msg + " (" +QString::number(number) + ")");
}

void ConnectWindow::onHide()
{
    hide();
}

void ConnectWindow::onShow()
{
    show();
}

ConnectWindow::~ConnectWindow()
{
    delete ui;
}
