#ifndef NETTHREAD_H
#define NETTHREAD_H

#include <QThread>

class NetThread : public QThread
{
    Q_OBJECT
public:
    explicit NetThread(QObject *parent = nullptr);
};

#endif // NETTHREAD_H
