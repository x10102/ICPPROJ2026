#ifndef _UDPREC_H
#define _UDPREC_H

#include "gui/picojson.h"
#include <QObject>
#include <QUdpSocket>

class UdpReceiver : QObject {
    Q_OBJECT
public:
    explicit UdpReceiver(QObject *parent = nullptr);
    UdpReceiver(QObject *parent = nullptr, unsigned int port = 6767);

private:
    QUdpSocket *sock = nullptr;
    bool isRunning = false;
    unsigned int port = 6767;

public slots:
    void start(void);
    void terminate(void);

private slots:
    void onReadyRead();

signals:
    void dataReceived(const picojson::object &data);

};

#endif