#include "mysongswindow.h"
#include "ui_mysongswindow.h"

MySongsWindow::MySongsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MySongsWindow)
{
    ui->setupUi(this);
}

MySongsWindow::~MySongsWindow()
{
    delete ui;
}

void MySongsWindow::onShow()
{
    show();
}
