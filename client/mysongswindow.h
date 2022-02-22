#ifndef MYSONGSWINDOW_H
#define MYSONGSWINDOW_H

#include <QMainWindow>

namespace Ui {
class MySongsWindow;
}

class MySongsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MySongsWindow(QWidget *parent = nullptr);
    ~MySongsWindow();

private:
    Ui::MySongsWindow *ui;

public slots:
    void onShow();
};

#endif // MYSONGSWINDOW_H
