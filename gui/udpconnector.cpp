#include "udpconnector.hpp"
#include "gui/picojson.h"
#include <QUdpSocket>
#include <QDebug>
#include <qglobal.h>
#include <qhostaddress.h>
#include <qobject.h>
#include <qudpsocket.h>
#include <string>

UdpConnector::UdpConnector(QObject *parent, unsigned int port) : QObject(parent) {
    this->port = port;
}

void UdpConnector::start() {
    this->sock = new QUdpSocket(this);
    // I'm not working with IPv6 in C or C++ again, thanks for the trauma, IPK
    this->sock->bind(QHostAddress::AnyIPv4, this->port);
    this->isRunning = true;
    connect(sock, &QUdpSocket::readyRead, this, &UdpConnector::onReadyRead);
}

void UdpConnector::terminate() {
    this->isRunning = false;
    if(this->sock)
        this->sock->close();
}

void UdpConnector::onReadyRead() {
    while(sock->hasPendingDatagrams()) {
        QByteArray data;
        picojson::value json;
        data.resize(sock->pendingDatagramSize());
        sock->readDatagram(data.data(), data.size());

        // This may not be necessary, idk if we can make picojson read it straight from the buffer
        std::string jsonStr = QString::fromUtf8(data).toStdString();
        std::string decodeError = picojson::parse(json, jsonStr);

        if(!decodeError.empty() || !json.is<picojson::object>()) {
            qWarning() << "Received invalid datagram, discarding it.";
            continue;
        }

        emit dataReceived(json.get<picojson::object>());
    }
}