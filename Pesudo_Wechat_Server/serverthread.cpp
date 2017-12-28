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
        this->fdToThreadMap.insert(clientfd, clientThread);
        // send log to ui
        connect(clientThread, SIGNAL(signal_error_box(QString)), this, SIGNAL(signal_error_box(QString)));
        connect(clientThread, SIGNAL(signal_info_box(QString)), this, SIGNAL(signal_info_box(QString)));
        // user login
        connect(clientThread, SIGNAL(signal_validate_user(QString,QString)), this,
                SLOT(slot_validate_user(QString,QString)));
        // user ask for new friend
        connect(clientThread, SIGNAL(signal_new_friend(QString,QString)), this, SLOT(slot_new_friend(QString,QString)));
        connect(clientThread, SIGNAL(signal_send_friend_list(QString,bool,bool)), this, SLOT(slot_send_friend_list(QString,bool,bool)));
        // user send text and file
        connect(clientThread, SIGNAL(signal_receive_text(QDateTime,QString,QString,QString)), this,
                SLOT(slot_receive_resend_text(QDateTime,QString,QString,QString)));
        connect(clientThread, SIGNAL(signal_receive_file(QDateTime,QString,QString,QString,QString)),
                this, SLOT(slot_receive_resend_file(QDateTime,QString,QString,QString,QString)));

        clientThread->start();
    }

    close(socketfd);
}

void ServerThread::slot_validate_user(QString username, QString password)
{
    ClientThread* senderThread = (ClientThread*) QObject::sender();
    if (userMap.validateUser(username, password))
    {
        log("info", QString("slot_validate_user(): validating user %1").arg(username));
        senderThread->slot_validate_user(userMap.findUser(username));
        this->usernameToThreadMap.insert(username, senderThread);
    }
    else
        senderThread->slot_validate_user(NULL);
}

void ServerThread::slot_new_friend(QString myName, QString name)
{
    ClientThread* senderThread = (ClientThread*) QObject::sender();
    log("info", QString("slot_new_friend(): finding name %1").arg(name));
    if (userMap.findUser(name) != NULL)
    {
        User* me = userMap.findUser(myName);
        User* friendUser = userMap.findUser(name);
        me->addFriend(friendUser);
        senderThread->slot_send_friend_list(true, true);

        // friend's list needs to refresh
        ClientThread* friendThread = this->usernameToThreadMap.find(name).value();
        if (friendThread != NULL)
        {
            friendThread->slot_send_friend_list(false, true);
        }
    }
    else
        senderThread->slot_send_friend_list(true, false);
}

// receive text from client and resend to another client
void ServerThread::slot_receive_resend_text(QDateTime time, QString sender, QString receiver, QString text)
{
    if (!this->usernameToThreadMap.contains(receiver))
    {
        log("info", QString("slot_receive_resend_text(): the receiver %1 has not logined").arg(receiver));
        // TODO: save and wait
        return;
    }
    ClientThread* clientThread = usernameToThreadMap.find(receiver).value();
    clientThread->slot_send_message_text(time, sender, receiver, text);
}

// receive file from client and resend to another client
void ServerThread::slot_receive_resend_file(QDateTime time, QString sender, QString receiver, QString filename, QString rawContent)
{
    if (!this->usernameToThreadMap.contains(receiver))
    {
        log("info", QString("slot_receive_resend_file(): the receiver %1 has not logined").arg(receiver));
        // TODO: save and wait
        return;
    }
    ClientThread* clientThread = usernameToThreadMap.find(receiver).value();
    clientThread->slot_send_message_file(time, sender, receiver, filename, rawContent);
}

void ServerThread::slot_send_friend_list(QString username, bool sendForNew, bool succeeded)
{
    ClientThread* senderThread = (ClientThread*) QObject::sender();
    log("info", QString("slot_send_friend_list()"));
    User* user;

    QString username1;
    QList<QString> list = usernameToThreadMap.keys();
    for (int i = 0; i < list.size(); i++)
    {
        if (usernameToThreadMap.find(list[i]).value() == senderThread)
        {
            username1 = list[i];
            break;
        }
    }
    log("info", QString("slot_send_friend_list(), username=%1").arg(username1));
    user = userMap.findUser(username1);
    log("info", QString("slot_send_friend_list(), user=%1").arg(user->getUsername()));

    QJsonObject jsonObject;
    jsonObject.insert("action", "send_friends_list_to_client");
    QJsonArray friendArray;
    QList<User*> friendList = user->getFriendList();
    for (int i = 0; i < friendList.size(); i++)
    {
        QJsonObject a;
        a.insert("username", friendList[i]->getUsername());
        friendArray.append(a);
    }
    jsonObject.insert("friends", friendArray);

    jsonObject.insert("send_for_add", QJsonValue(sendForNew));
    jsonObject.insert("add_success", QJsonValue(succeeded));

    log("info", QString("slot_send_friend_list(): size=%1").arg(friendArray.size()));

    senderThread->slot_send_json(jsonObject);
}
