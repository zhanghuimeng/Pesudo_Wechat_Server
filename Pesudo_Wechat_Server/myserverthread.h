#ifndef MYSERVER_H
#define MYSERVER_H

#include <QThread>
#include <QDebug>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "user.h"

#define BACKLOG 10  // 指定了该服务器所能连接客户端的最大数目
#define MAXLEN 1050000

class MyServerThread: public QThread
{
    Q_OBJECT
private:
    int socketfd;  // 套接字描述符
    int clientfd;
    struct sockaddr_in serverAddr;
    int serverPort;
    char buf[MAXLEN];
    int n;
    QString error;
    QString info;
    UserMap userMap;

    void log(QString level, QString msg);
    void log_error(QString msg);
    void log_info(QString msg);
    QByteArray jsonToString(QJsonObject json);

protected:
    void run();
    void parseReceived(const char* msg, int length);

public:
    MyServerThread();

public slots:
    void slot_send_bytes(const char *bytes);

signals:
    void signal_error_box(QString);
    void signal_info_box(QString);
};

#endif // MYSERVER_H
