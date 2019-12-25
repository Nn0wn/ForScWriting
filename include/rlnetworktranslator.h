#ifndef RlNetworkTranslator_H
#define RlNetworkTranslator_H

#include "networktranslator.h"
#include "QTcpSocket"
#include "QNetworkDatagram"

#define TEST_CONNECTING_TIMER 100
#define CONNECTING_TIMER 30000
#define CONNECTING_DELAY 500
#define CONNECTING_INTERVAL 50
#define WRITING_TIMER 30000
#define CORRECT_EXIT_CODE 0
#define INCORRECT_EXIT_CODE -1

class RlNetworkTranslator : public NetworkTranslator
{
    Q_OBJECT

    QBasicTimer *m_errorTimer;
    QBasicTimer *m_tryTimer;
    QMessageBox *m_box;

    QString     answer;

    bool setupListener(quint16 listener_port, quint16 server_port, int mode = MessageMode::NORMAL);
    bool setupConnector(const QString& hostName, quint16 connecting_port,
                        QAbstractSocket::NetworkLayerProtocol protocol, int mode = MessageMode::NORMAL);
    void timerEvent(QTimerEvent* e) override;

public:
    explicit RlNetworkTranslator(QObject *parent = nullptr, const QString &hostName = QHostAddress(QHostAddress::LocalHost).toString(),
                                  quint16 listener_port = STANDART_LISTENING_PORT, quint16 connecting_port = STANDART_CONNECTION_PORT,
                                  QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol);
    ~RlNetworkTranslator() override;
    bool setupConnection(const QString &hostName, quint16 listening_port, quint16 connecting_port,
                         QAbstractSocket::NetworkLayerProtocol protocol, quint16 server_port = NO_PORT, int mode = MessageMode::NORMAL) override;
    void disconnect() override;
    quint16 getIport() override;
    quint16 getOport() override;

    QString getData();
    void encodeData(QVariant source);
    template<typename T> T decodeData(QVariant data);

signals:
    void newDataToSend();
    void connectionEstablished();
    void dataDelivered();
    void connectedToServer();
    void serverConnected();

private slots:
    virtual void checkConnection() override;
    virtual void changeInputStatus(QAbstractSocket::SocketState) override;
    virtual void changeOutputStatus(QAbstractSocket::SocketState) override;

public slots:
    virtual void read() override;
    virtual void write() override;

    void udpStateChanged();
};

#endif // RlNetworkTranslator_H
