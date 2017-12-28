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
        QJsonObject textObject = jsonRec.find("text").value().toObject();
        QString text = textObject.find("text").value().toString();
        QString sender = textObject.find("sendby").value().toString();
        QString receiver = textObject.find("sendto").value().toString();
        QDateTime time = QDateTime::fromTime_t(textObject.find("time").value().toInt());
        log("info", QString("parseReceived(): Action: receiving text from client, sender=%1, receiver=%2, len=%3")
            .arg(sender).arg(receiver).arg(text.size()));
        emit signal_receive_text(time, sender, receiver, text);
    }
    /*
    action: "send_file_to_server"
    file: {filename: "a.png", content: "用base64压缩之后的文件", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}
    */
    else if (action == "send_file_to_server")
    {
        QJsonObject fileObject = jsonRec.find("file").value().toObject();
        QString filename = fileObject.find("filename").value().toString();
        QString sender = fileObject.find("sendby").value().toString();
        QString receiver = fileObject.find("sendto").value().toString();
        QDateTime time = QDateTime::fromTime_t(fileObject.find("time").value().toInt());

        QString rawContent = fileObject.find("content").value().toString();
        QByteArray encrypted = QByteArray::fromBase64(rawContent.toLocal8Bit());  // TOOD: is this right?  ok.
        QByteArray original = qUncompress(encrypted);

        log("info", QString("parseReceived(): FILE: sender=%1, receiver=%2, filename=%3").arg(sender).arg(receiver).arg(filename));
        log("info", QString("parseReceived(): FILE: received size=%1, decoded size=%2").arg(rawContent.size()).arg(original.size()));
        emit signal_receive_file(time, sender, receiver, filename, rawContent);
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

/**
 * @brief ClientThread::slot_send_message_text
 * @param time
 * @param sender
 * @param receiver
 * @param text
 */
/*
action: "send_text_to_client"
text: {text: "send some text blabla...", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}
*/
void ClientThread::slot_send_message_text(QDateTime time, QString sender, QString receiver, QString text)
{
    log("info", QString("slot_send_message_text(): sender=%1, text=%2").arg(sender).arg(text));
    QJsonObject textObject;
    textObject.insert("text", QJsonValue(text));
    textObject.insert("sendby", QJsonValue(sender));
    textObject.insert("sendto", QJsonValue(receiver));
    textObject.insert("time", QJsonValue(qint64(time.toTime_t())));
    QJsonObject jsonObject;
    jsonObject.insert("text", QJsonValue(textObject));
    jsonObject.insert("action", QJsonValue("send_text_to_client"));

    this->slot_send_json(jsonObject);
}

/**
 * @brief ClientThread::slot_send_message_file
 * @param time
 * @param sender
 * @param receiver
 * @param filename
 * @param rawContent
 */
/*
action: "send_file_to_client"
file: {filename: "a.png", content: "用base64压缩之后的文件", time: "2017/1/1:00:00:00", sendby: "zhm_1", sendto: "zhm_2"}
*/
void ClientThread::slot_send_message_file(QDateTime time, QString sender, QString receiver,
                                          QString filename, QString rawContent)
{
    log("info", QString("slot_send_message_file(): sender=%1, filename=%2").arg(sender).arg(filename));
    QJsonObject jsonObject;
    QJsonObject fileObject;
    fileObject.insert("filename", QJsonValue(filename));
    fileObject.insert("content", QJsonValue(rawContent));
    fileObject.insert("time", QJsonValue(qint64(time.toTime_t())));
    fileObject.insert("sendby", QJsonValue(sender));
    fileObject.insert("sendto", QJsonValue(receiver));

    jsonObject.insert("action", QJsonValue("send_file_to_client"));
    jsonObject.insert("file", QJsonValue(fileObject));

    this->slot_send_json(jsonObject);
}
