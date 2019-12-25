#ifndef SIMNETWORKTRANSLATOR_H
#define SIMNETWORKTRANSLATOR_H

#include "networktranslator.h"
#include "QTcpServer"
#include "QInputDialog"
#include "SimMessageDialog.h"

#define STANDART_SERVER_LISTENING_PORT 6001
#define TEST_SERVER_LISTENING_PORT 601
#define STANDART_SIMULATOR_CONNECTION_PORT 6000
#define LOWEST_PORT_VALUE 1024
#define HIGHEST_PORT_VALUE 65535

class SimNetworkTranslator : public NetworkTranslator
{
    Q_OBJECT

    QTcpServer  *m_server;

public:
    explicit SimNetworkTranslator(QObject *parent = nullptr, const QString &hostName = QHostAddress(QHostAddress::LocalHost).toString(),
                                  quint16 listening_port = STANDART_SERVER_LISTENING_PORT,
                                  quint16 connecting_port = STANDART_SIMULATOR_CONNECTION_PORT,
                                  QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol);
    ~SimNetworkTranslator() override;
    bool setupConnection(const QString &hostName = NO_ADDRESS, quint16 listening_port = NO_PORT, quint16 connecting_port = NO_PORT,
                         QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol,
                         quint16 server_port = NO_PORT, int mode = MessageMode::NORMAL) override;
    void disconnect() override;
    quint16 getIport() override;
    quint16 getOport() override;

    bool setupServer(QHostAddress server_address, int mode = MessageMode::NORMAL);
    void stopServer();
    QTcpServer* getServer();

    void encodeData(QVariant source);
    template<typename T> T decodeData(QVariant data);

signals:
    void newDataToSend();
    void connectionEstablished();
    void connectedToClt();
    void disconnectedFromClt();
    void cltConnected();
    void cltDisconnected();

protected slots:
    virtual void checkConnection() override;
    virtual void changeInputStatus(QAbstractSocket::SocketState state) override;
    virtual void changeOutputStatus(QAbstractSocket::SocketState state) override;

public slots:
    virtual void read() override;
    virtual void write() override;

    void connectSocket();
};

#endif // SIMNETWORKTRANSLATOR_H
