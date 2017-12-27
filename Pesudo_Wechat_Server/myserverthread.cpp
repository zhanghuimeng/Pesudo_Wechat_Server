#include "myserverthread.h"
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

MyServerThread::MyServerThread(): serverPort(6666), QThread()
{

}

QByteArray MyServerThread::jsonToString(QJsonObject json)
{
    QJsonDocument doc(json);
    QByteArray ba = doc.toBinaryData();
    return ba;
}

void MyServerThread::log(QString level, QString msg)
{
    if (level == "error") {
        emit signal_error_box(msg);
    }
    else if (level == "info") {
        emit signal_info_box(msg);
    }
    qDebug() << "MyServerThread: " << level << ": " << msg << endl;
}

void MyServerThread::log_error(QString msg)
{
    log("error", msg);
}

void MyServerThread::log_info(QString msg)
{
    log("info", msg);
}

void MyServerThread::run()
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

        n = recv(clientfd, buf, MAXLEN, 0);
        buf[n] = '\0';

        info = QString("Received message from client: length=%1").arg(n);
        log_info(info);
        info.clear();

        parseReceived(buf, n);

        close(clientfd);
    }

    close(socketfd);
}

void MyServerThread::slot_send_bytes(const char *bytes)
{
    if (send(clientfd, bytes, strlen(bytes), 0) < 0)
    {
        error = "Cannot send data to client";
        log("error", error);
        error.clear();
    }
    info = QString("Send data to client, length=%1").arg(strlen(bytes));
    log("info", info);
    info.clear();
}

void MyServerThread::parseReceived(const char* msg, int length)
{
    QJsonDocument doc = QJsonDocument::fromRawData(msg, length);
    QJsonObject jsonRec = doc.object();
    QString action = jsonRec.find("action").value().toString();

    info = QString("Parsing: client action=%1").arg(action);
    log_info(info);
    info.clear();

    /*
    action: "client_login"
    username: "zhm_x"
    password: "123456"
     */
    if (action == "client_login")
    {
        log_info("Action: client login");
        QString username = jsonRec.find("username").value().toString();
        QString password = jsonRec.find("password").value().toString();

        /*
        action: "server_login_response"
        correct: "true/false"
        */
        QJsonObject jsonSend;
        jsonSend.insert("action", QJsonValue("server_login_response"));

        if (userMap.validateUser(username, password))
        {
            jsonSend.insert("correct", QJsonValue(true));
            log_info("Action: client login succeeded");
            log_info(QString("username = %1").arg(username));
        }
        else
        {
            jsonSend.insert("correct", QJsonValue(false));
            log_info("Action: client login failed");
        }
        slot_send_bytes(jsonToString(jsonSend).data());
    }
}
