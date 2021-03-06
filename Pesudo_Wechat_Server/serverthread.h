#ifndef MYSERVER_H
#define MYSERVER_H

#include <QThread>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "user.h"
#include "clientthread.h"

#define BACKLOG 100  // 指定了该服务器所能连接客户端的最大数目
#define PORT 3699

class ServerThread: public QThread
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
    QMap<int, ClientThread*> fdToThreadMap;
    QMap<QString, ClientThread*> usernameToThreadMap;

    void log(QString level, QString msg);
    void log_error(QString msg);
    void log_info(QString msg);
    QByteArray jsonToString(QJsonObject json);
    QByteArray jsonToReadableString(QJsonObject json);
    QJsonObject stringToJson(const char* bytes, int len);

protected:
    void run();
    void parseReceived(const char* msg, int length);

public:
    ServerThread();

public slots:
    void slot_validate_user(QString username, QString password);
    void slot_new_friend(QString myName, QString name);  // send from ClientThread
    void slot_receive_resend_text(QDateTime time, QString sender, QString receiver, QString text);  // received from some ClientThread
    void slot_receive_resend_file(QDateTime time, QString sender, QString receiver, QString filename, QString rawContent);  // received from some ClientThread
    void slot_send_friend_list(QString username, bool sendForNew, bool succeeded);  // emit from ClientThread, for multithread problem

signals:
    void signal_error_box(QString);
    void signal_info_box(QString);
};

#endif // MYSERVER_H
