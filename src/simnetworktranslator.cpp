#include "simnetworktranslator.h"

SimNetworkTranslator::SimNetworkTranslator(QObject *parent, const QString &hostName, quint16 listening_port, quint16 connecting_port,
                                           QAbstractSocket::NetworkLayerProtocol protocol) :
    NetworkTranslator(), m_server(new QTcpServer(this))
{
    m_udpSocket = new QUdpSocket(this);
    m_tcpSocket = new QTcpSocket(this);
    m_inputData = new QByteArray();
    m_outputData = new QByteArray();
    m_hostName = hostName;
    m_listeningPort = listening_port;
    m_connectingPort = connecting_port;
    m_inputConnection = false;
    m_outputConnection = false;
    m_protocol = protocol;
    this->setParent(parent);
    connect(m_udpSocket, &QUdpSocket::stateChanged, this, &SimNetworkTranslator::changeOutputStatus);
    connect(m_tcpSocket, &QTcpSocket::stateChanged, this, &SimNetworkTranslator::changeInputStatus);
}

SimNetworkTranslator::~SimNetworkTranslator()
{
    connect(m_tcpSocket, &QTcpSocket::stateChanged, this, &SimNetworkTranslator::changeInputStatus);
    connect(m_udpSocket, &QUdpSocket::stateChanged, this, &SimNetworkTranslator::changeOutputStatus);
    delete m_outputData;
    delete m_inputData;
    delete m_tcpSocket;
    delete m_server;
    delete m_udpSocket;
}

bool SimNetworkTranslator::setupConnection(const QString &hostName, quint16 listening_port, quint16 connecting_port,
                                           QAbstractSocket::NetworkLayerProtocol protocol, quint16 server_port, int mode)
{
    QMessageBox box;
    m_protocol = protocol;
    if(hostName.simplified() != "")
    {
        m_hostName = hostName;
    }
    if(listening_port == server_port && listening_port != NO_PORT)
    {
        m_listeningPort = listening_port;
    }
    if(connecting_port != NO_PORT)
    {
        m_connectingPort = connecting_port;
    }

    m_udpSocket->connectToHost(m_hostName, m_connectingPort, QIODevice::WriteOnly, m_protocol);
    if(!m_udpSocket->waitForConnected())
    {
        switch(mode)
        {
        case NORMAL:
            box.setText("Server cannot connect to GUI");
            box.setIcon(QMessageBox::Warning);
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();
            break;
        case CONSOLE:
            qInfo() << "Server cannot connect to GUI";
            break;
        default:
            break;
        }
        return false;
    }
    connect(this, &SimNetworkTranslator::newDataToSend, this,  &SimNetworkTranslator::write);
#if defined (CONNECTION_DEBUG)
    qDebug() << "Simulator UDP socket peer port is:" << m_udpSocket->peerPort();
    qDebug() << "Simulator UDP socket local port is:" << m_udpSocket->localPort();
#endif

    switch(mode)
    {
    case NORMAL:
        box.setText("Server succesfully connected to GUI");
        box.setIcon(QMessageBox::Information);
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        break;
    case CONSOLE:
        qInfo() << "Server succesfully connected to GUI";
        break;
    default:
        break;
    }
    return true;
}

void SimNetworkTranslator::disconnect()
{
    QObject::disconnect(this, &SimNetworkTranslator::newDataToSend, this,  &SimNetworkTranslator::write);
    m_udpSocket->disconnectFromHost();
}

QTcpServer *SimNetworkTranslator::getServer()
{
    return m_server;
}

quint16 SimNetworkTranslator::getIport()
{
    return m_listeningPort;
}

quint16 SimNetworkTranslator::getOport()
{
    return m_connectingPort;
}

bool SimNetworkTranslator::setupServer(QHostAddress server_address, int mode)
{
    QMessageBox msgBox;
    SimMessageDialog box;
    int count = 0;
    connect(m_server, &QTcpServer::newConnection, this, &SimNetworkTranslator::connectSocket);
    switch (mode)
    {
    case NORMAL:
        while(!m_server->listen(server_address, m_listeningPort))
        {
            box.setText("Port " + QString::number(m_listeningPort) + " is unavailable. Choose another one\n"
                        "(empty input will lead to a random free port choice):");
            box.setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            box.setIcon(DialogPic::Warning);
            box.setValidator(new QIntValidator(LOWEST_PORT_VALUE, HIGHEST_PORT_VALUE, this));
            box.exec();
            if(box.result() == QDialog::Rejected)
            {
                msgBox.setText("Server is not running");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.exec();
                return false;
            }
            else if(box.result() == QDialog::Accepted)
            {
                m_listeningPort = box.text().toUShort();
            }
        }
        msgBox.setText("Server is listening on " + QString::number(m_server->serverPort()) + " port");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        break;
    case CONSOLE:
        while(!m_server->listen(server_address, m_listeningPort) && count < 1000)
        {
            qInfo() << "Port " + QString::number(m_listeningPort) + " is unavailable";
            m_listeningPort = 0;
            count++;
        }
        if(m_server->isListening())
        {
            qInfo() << "Server is listening on " + QString::number(m_server->serverPort()) + " port";
        }
        else
        {
            qInfo() << "Server is not listening";
            return false;
        }
        break;
    case SILENT:
        while(!m_server->listen(server_address, m_listeningPort) && count < 1000)
        {
            m_listeningPort = 0;
            count++;
        }
        if(!m_server->isListening())
        {
            return false;
        }
    }
    m_listeningPort = m_server->serverPort();
#if defined (CONNECTION_DEBUG)
    qDebug() << "Simulator server listening port is:" << m_server->serverPort();
#endif
    return true;
}

void SimNetworkTranslator::stopServer()
{
    disconnect();
    if(m_tcpSocket->isOpen())
    {
        if(m_tcpSocket->bytesAvailable() != NO_DATA)
        {
            *m_inputData = m_tcpSocket->readAll();
        }
        QObject::disconnect(m_tcpSocket, &QTcpSocket::readyRead, this, &SimNetworkTranslator::read);
        m_tcpSocket->close();
    }
    QObject::disconnect(m_server, &QTcpServer::newConnection, this, &SimNetworkTranslator::connectSocket);
    m_server->close();
}

void SimNetworkTranslator::connectSocket()
{
    m_tcpSocket = m_server->nextPendingConnection();
    if(m_tcpSocket->waitForConnected())
    {
        connect(m_tcpSocket, &QTcpSocket::readyRead, this, &SimNetworkTranslator::read);
    }
#if defined (CONNECTION_DEBUG)
    qDebug() << "Simulator TCP socket state: ";
    returnState(m_tcpSocket->state());
#endif
}

void SimNetworkTranslator::read()
{
    if(m_tcpSocket->state() == QTcpSocket::ConnectedState)
    {
        while(m_tcpSocket->bytesAvailable() )
        {
            QVariant result;
            QDataStream in(m_inputData, QIODevice::ReadOnly);
            in.setVersion(QDataStream::Qt_5_13);
            in.setDevice(m_tcpSocket);
            in.startTransaction();
            if(!in.commitTransaction())
            {
                return;
            }
            in >> result;
#if defined (CONNECTION_DEBUG)
            qDebug() << "TCP transported data is:" << result;
            qDebug() << "int typeid" << typeid(int).name();
            if(result.typeName() == QString("int").toLocal8Bit())
            {
                int a = decodeData<int>(result);
                qDebug() << "TCP transported data encoded is:" << a;
            }
#endif
            if(result.typeName() == QString("QString").toLocal8Bit())
            {
                QString a = decodeData<QString>(result);
                qDebug() << "TCP transported data encoded is:" << a;
                if(a == "*ESE?")
                {
                    encodeData(QString::number(rand() % 255));
                }
//                else if(a.startsWith(":Limit:Hposition"))
//                {
//                    encodeData("1000");
//                }
                else
                {
                    encodeData(a);
                }
            }
//#endif
        }
#if defined (CONNECTION_DEBUG)
        //encodeData("TCP transmission succesful");
#endif
    }
}

void SimNetworkTranslator::write()
{
#if defined (CONNECTION_DEBUG)
    qDebug() << "UDP write bytes:" << m_udpSocket->write(*m_outputData);
#else
    m_udpSocket->write(*m_outputData);
#endif
    m_udpSocket->waitForBytesWritten(-1);
    m_outputData->clear();
}

void SimNetworkTranslator::encodeData(QVariant source)
{
    QDataStream out(m_outputData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_13);
    out << source;
#if defined (CONNECTION_DEBUG)
    qDebug() << "Initial reply data is" << source;
    qDebug() << "Initial reply data encoded is" << source;
#endif
    emit newDataToSend();
}

template<typename T>
T SimNetworkTranslator::decodeData(QVariant data)
{
    T input = nullptr;
    if(data.canConvert<T>())
    {
        input = data.value<T>();
    }
    else
    {
#if defined (CONNECTION_DEBUG)
        qDebug() << "Cannot convert to T";
#endif
    }
    return input;
}

void SimNetworkTranslator::checkConnection()
{
    if(m_inputConnection && m_outputConnection)
    {
        emit connectionEstablished();
    }
}

void SimNetworkTranslator::changeInputStatus(QAbstractSocket::SocketState state)
{
    if(state == QAbstractSocket::ConnectedState)
    {
        m_inputConnection = true;
        emit checkConnection();
    }
    else
    {
        m_inputConnection = false;
    }
}

void SimNetworkTranslator::changeOutputStatus(QAbstractSocket::SocketState state)
{
    if(state == QAbstractSocket::ConnectedState)
    {
        m_outputConnection = true;
        emit checkConnection();
    }
    else
    {
        m_outputConnection = false;
    }
}
