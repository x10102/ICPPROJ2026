/**
 * @file udpconnector.hpp
 * @author Ondřej Turek, xtureko00
 * @brief UDP socket for interpreter IPC interface
 */
#ifndef _UDPREC_H
#define _UDPREC_H

#include "gui/picojson.h"
#include <QObject>
#include <QUdpSocket>
#include <qudpsocket.h>

class UdpConnector : public QObject {
    Q_OBJECT
public:
    explicit UdpConnector(QObject *parent = nullptr);
    UdpConnector(QObject *parent = nullptr, unsigned int port = 6767);

private:
    QUdpSocket *inSock = nullptr;
    QUdpSocket *outSock = nullptr;
    bool isRunning = false;
    bool isConnected = false;
    unsigned int port = 6767;

public slots:
    void start();
    void terminate();
    void sendStep();
    void connectToInt();

private slots:
    void onReadyRead();

signals:
    void dataReceived(picojson::object &data);

};

#endif