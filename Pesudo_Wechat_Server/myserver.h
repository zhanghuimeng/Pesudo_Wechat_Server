#ifndef MYSERVER_H
#define MYSERVER_H

#include <QThread>
#include <QDebug>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#define BACKLOG 10  // 指定了该服务器所能连接客户端的最大数目
#define MAXLINE 4096

class MyServer: public QThread
{
    Q_OBJECT
private:
    int socketfd;  // 套接字描述符
    int clientfd;
    struct sockaddr_in serverAddr;
    int serverPort;
    char buf[MAXLINE];
    int n;
    QString error;
    QString info;

protected:
    void run();

public:
    MyServer();

signals:
    void signal_error_box(QString);
    void signal_info_box(QString);
};

#endif // MYSERVER_H
