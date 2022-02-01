#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "playthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    PlayThread *playThread;

protected:
    void play();

private:
    Ui::MainWindow *ui;

public slots:
    void onTimeChanged(float,float);
    void onSongChanged(QString);

};
#endif // MAINWINDOW_H
