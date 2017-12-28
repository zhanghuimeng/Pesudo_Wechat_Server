#include "serverthread.h"
#include "clientthread.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ServerThread::ServerThread(): QThread(), serverPort(PORT)
{

}

void ServerThread::log(QString level, QString msg)
{
    if (level == "error") {
        emit signal_error_box(msg);
    }
    else if (level == "info") {
        emit signal_info_box(msg);
    }
    qDebug() << "MyServerThread: " << level << ": " << msg << endl;
}

void ServerThread::log_error(QString msg)
{
    log("error", msg);
}

void ServerThread::log_info(QString msg)
{
    log("info", msg);
}

void ServerThread::run()
{
    // Use linux socket to establish the server
    /* 该函数包含三个参数:
     * domain参数用于指定所创建套接字的协议类型。通常选用AF_INET，表示使用IPv4的TCP/IP协议；如果只在本机内进行进程间通信，则可以使用AF_UNIX。
     * 参数type用来指定套接字的类型，SOCK_STREAM用于创建一个TCP流的套接字，SOCK_DGRAM用于创建UDP数据报套接字。
     * 参数protocol通常取0
     */
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        error = "Cannot create a socket file handle";
        log("error", error);
        error.clear();
        return;
    }

    memset(&serverAddr, 0, sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 通过bind函数可将服务器套接字和一个指定的端口号进行绑定。
    if(bind(socketfd, (struct sockaddr*) &serverAddr, sizeof(struct sockaddr_in)) == -1)
    {
        error = QString("Cannot bind a socket to port %1").arg(serverPort);
        log("error", error);
        error.clear();
        return;
    }
    info = QString("Binded socket to port %1").arg(serverPort);
    log("info", info);
    info.clear();

    if (listen(socketfd, BACKLOG) == -1)
    {
        error = QString("Cannot listen to port %1").arg(serverPort);
        log("error", error);
        error.clear();
        return;
    }
    info = QString("Listening to port %1").arg(serverPort);
    log("info", info);
    info.clear();

    while(true)
    {
        if ((clientfd = accept(socketfd, (struct sockaddr*) NULL, NULL)) == -1)
        {
            error = "Cannot accept socket";
            log("error", error);
            error.clear();
            continue;
        }
        info = QString("Accepted client socket: handle=%1").arg(clientfd);
        log("info", info);
        info.clear();

        ClientThread* clientThread = new ClientThread(clientfd);
        // send log to ui
        connect(clientThread, SIGNAL(signal_error_box(QString)), this, SIGNAL(signal_error_box(QString)));
        connect(clientThread, SIGNAL(signal_info_box(QString)), this, SIGNAL(signal_info_box(QString)));
        // user login
        connect(clientThread, SIGNAL(signal_validate_user(QString,QString)), this,
                SLOT(slot_validate_user(QString,QString)));
        clientThread->start();
    }

    close(socketfd);
}

void ServerThread::slot_validate_user(QString username, QString password)
{
    ClientThread* sender = (ClientThread*) QObject::sender();
    if (userMap.validateUser(username, password))
        sender->slot_validate_user(userMap.findUser(username));
    else
        sender->slot_validate_user(NULL);
}
