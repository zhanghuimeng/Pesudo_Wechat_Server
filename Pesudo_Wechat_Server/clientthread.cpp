#include "clientthread.h"
#include <QDebug>
#include <QJsonArray>

ClientThread::ClientThread(int clientfd): QThread(), clientfd(clientfd), buffer(new char[MAXLEN]), user(NULL)
{

}

void ClientThread::log(QString level, QString msg)
{
    qDebug() << level << QString(": ClientThread(socketfd=%1)::%2").arg(clientfd).arg(msg) << endl;
    if (level == "error")
        emit signal_error_box(msg);
    else if (level == "info")
        emit signal_info_box(msg);
}

QByteArray ClientThread::jsonToString(QJsonObject json)
{
    QJsonDocument doc(json);
    QByteArray ba = doc.toJson(QJsonDocument::Compact);
    return ba;
}

QJsonObject ClientThread::stringToJson(const char *bytes, int length)
{
    QByteArray byteArray(bytes, length);
    QJsonDocument doc = QJsonDocument::fromJson(byteArray.data());
    QJsonObject json = doc.object();
    return json;
}

void ClientThread::run()
{
    // jumping into the major loop
    while (true)
    {
        int n = recv(clientfd, buffer, MAXLEN, 0);
        if (n < 0)
        {
            log("error", "run(): Receiving error");
            continue;
        }
        buffer[n] = '\0';
        if (n == 0 || strlen(buffer) == 0)  // has received nothing
            continue;

        log("info", QString("run(): Received message from client: length=%1, content=%2").arg(n).arg(buffer));

        parseReceived(buffer, n);
        memset(buffer, 0, MAXLEN * sizeof(char));
    }
}

void ClientThread::slot_send_bytes(const char *bytes)
{
    if (send(clientfd, bytes, strlen(bytes), 0) < 0)
    {
        log("error", "slot_send_bytes(): Cannot send data to client");
        return;
    }
    log("info", QString("slot_send_bytes(): Send data to client, length=%1, content=%2").arg(strlen(bytes)).arg(bytes));
}

void ClientThread::slot_send_json(QJsonObject jsonObject)
{
    slot_send_bytes(jsonToString(jsonObject));
}

void ClientThread::parseReceived(const char* msg, int length)
{
    QJsonObject jsonRec = stringToJson(msg, length);
    QString action = jsonRec.find("action").value().toString();

    log("info", QString("parseReceived(): Parsing: client action=%1, json=%2").arg(action).arg(jsonToString(jsonRec).data()));

    /*
    action: "client_login"
    username: "zhm_x"
    password: "123456"
     */
    if (action == "client_login")
    {
        log("info", "parseReceived(): Action: client logining");
        QString username = jsonRec.find("username").value().toString();
        QString password = jsonRec.find("password").value().toString();

        emit signal_validate_user(username, password);
        // call ServerThread slot_validate_user
        // ServerThread direct call slot_validate_user
    }
    /*
    action: "get_friends_list"
    username: "zhm_1"
    */
    else if (action == "get_friends_list")
    {
        log("info", "parseReceived(): Action: client requiring friends list");
        slot_send_friend_list();
    }
    /*
    action: "send_text_to_server"
    text: {text: "send some text blabla...", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}
    */
    else if (action == "send_text_to_server")
    {
        log("info", "parseReceived(): Action: receiving text from client");
    }
}

void ClientThread::slot_validate_user(User *user)
{
    /*
    action: "server_login_response"
    correct: "true/false"
    */
    QJsonObject jsonSend;
    jsonSend.insert("action", QJsonValue("server_login_response"));
    if (user != NULL)
    {
        jsonSend.insert("correct", QJsonValue(true));
        log("info", "slot_validate_user(): Action: client login succeeded");
        log("info", QString("slot_validate_user(): username = %1").arg(user->getUsername()));
        this->user = user;
        slot_send_friend_list();
    }
    else
    {
        jsonSend.insert("correct", QJsonValue(false));
        log("info", "slot_validate_user(): Action: client login failed");
    }
    slot_send_json(jsonSend);
}

/**
 * @brief send the friend list to a user
 */
void ClientThread::slot_send_friend_list()
{
    log("info", QString("slot_send_friend_list(): sending user \"%1\" friend list").arg(user->getUsername()));
    /*
    action: "send_friends_list_to_client"
    friends: [{username: "zhm_2"}, {username: "zhm_3"}]
    */
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

    log("info", QString("slot_send_friend_list(): size=%1").arg(friendArray.size()));
    slot_send_json(jsonObject);
}
