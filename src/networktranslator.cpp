#include "networktranslator.h"

#if defined (CONNECTION_DEBUG)
void NetworkTranslator::returnState(QAbstractSocket::SocketState state)
{
    switch(state)
    {
        case QAbstractSocket::ClosingState:     qDebug() << "TcpSocket on a closing state";     break;
        case QAbstractSocket::ConnectedState:   qDebug() << "TcpSocket on a connected state";   break;
        case QAbstractSocket::ConnectingState:  qDebug() << "TcpSocket on a connecting state";  break;
        case QAbstractSocket::UnconnectedState: qDebug() << "TcpSocket on a unconnected state"; break;
        case QAbstractSocket::BoundState:       qDebug() << "TcpSocket on a bound state";       break;
        case QAbstractSocket::ListeningState:   qDebug() << "TcpSocket on a listening state";   break;
        case QAbstractSocket::HostLookupState:  qDebug() << "TcpSocket on a host lookup state"; break;
    }
}
#endif

NetworkTranslator::~NetworkTranslator()
{

}
