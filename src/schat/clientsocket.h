#ifndef CLIENTSOCKET_H_
#define CLIENTSOCKET_H_

#include <QTcpSocket>
#include <QListWidgetItem>

class ClientSocket : public QTcpSocket
{
  Q_OBJECT

public:
  ClientSocket(QObject *parent = 0);
  void setNick(const QString &n);
  void send(quint16 opcode);
  void send(quint16 opcode, const QString &s);

signals:
  void newParticipant(const QString &p, bool echo = true);
  void participantLeft(const QString &nick);
  void newMessage(const QString &nick, const QString &message);
  
private slots:
  void readyRead();
  void sendGreeting();
  void error();
  void disconnected();
  
private:
  bool readBlock();
  void newParticipant(bool echo = true);
  
  QDataStream currentBlock;
  QString nick;
  QString userAgent;
  QString userMask;
  QString message;
  
  quint16 currentCommand;
  quint16 currentState;
  quint16 nextBlockSize;
};

#endif /*CLIENTSOCKET_H_*/
