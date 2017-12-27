#include "myserver.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <QDebug>

MyServer::MyServer(): serverPort(6666)
{

}

void MyServer::run()
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
        emit signal_error_box(error);
        qDebug() << "Error: " << error << endl;
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
        emit signal_error_box(error);
        qDebug() << "Error: " << error << endl;
        error.clear();
        return;
    }

    if (listen(socketfd, BACKLOG) == -1)
    {
        error = QString("Cannot listen to port %1").arg(serverPort);
        emit signal_error_box(error);
        qDebug() << "Error: " << error << endl;
        error.clear();
        return;
    }

    info = QString("Listening to port %1").arg(serverPort);
    emit signal_info_box(info);
    qDebug() << "Info: " << info << endl;
    info.clear();

    while(true)
    {
        if ((clientfd = accept(socketfd, (struct sockaddr*) NULL, NULL)) == -1)
        {
            error = "Cannot accept socket";
            emit signal_error_box(error);
            qDebug() << "Error: " << error << endl;
            continue;
        }
        n = recv(clientfd, buf, MAXLINE, 0);
        buf[n] = '\0';

        info = QString("Received message from client: length=%1").arg(n);
        emit signal_info_box(info);
        qDebug() << "Info: " << info << endl;
        info.clear();
        // close(clientfd);
    }

    // close(socketfd);
}
