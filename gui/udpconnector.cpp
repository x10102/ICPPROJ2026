/**
 * @file udpconnector.cpp
 * @author Ondřej Turek, xtureko00
 * @brief Implementace UDP rozhraní pro komunikaci interpretu a GUI
 */
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
    this->inSock = new QUdpSocket(this);
    // I'm not working with IPv6 in C or C++ again, thanks for the trauma, IPK
    this->inSock->bind(QHostAddress::AnyIPv4, this->port);
    this->outSock = new QUdpSocket(this);
    this->isRunning = true;
    connect(inSock, &QUdpSocket::readyRead, this, &UdpConnector::onReadyRead);
}

void UdpConnector::terminate() {
    this->isRunning = false;
    if(this->inSock)
        this->inSock->close();
}

void UdpConnector::sendStep() {
    std::string command = "{\"command\": \"step\"}";
    QByteArray datagram(command.c_str());
    this->outSock->writeDatagram(datagram.data(), QHostAddress::LocalHost, port+1);
}

void UdpConnector::connectToInt() {
    QHostAddress local("127.0.0.1");
    this->outSock->connectToHost(local, this->port+1);
}

void UdpConnector::onReadyRead() {
    while(inSock->hasPendingDatagrams()) {
        QByteArray data;
        picojson::value json;
        data.resize(inSock->pendingDatagramSize());

        // Save the remote address so we know where to respond to later
        inSock->readDatagram(data.data(), data.size());

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