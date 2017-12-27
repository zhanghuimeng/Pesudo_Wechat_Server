#include "clientthread.h"
#include <QDebug>

ClientThread::ClientThread(int clientfd): QThread(), clientfd(clientfd), buffer(new char[MAXLEN]), user(NULL)
{

}

void ClientThread::log(QString level, QString msg)
{
    qDebug() << QString("ClientThread(socketfd=%1): ").arg(clientfd) << level << ": " << msg << endl;
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
            log("error", "Receiving error");
            continue;
        }
        buffer[n] = '\0';
        if (n == 0 || strlen(buffer) == 0)  // has received nothing
            continue;

        log("info", QString("Received message from client: length=%1, content=%2").arg(n).arg(buffer));

        parseReceived(buffer, n);
        memset(buffer, 0, MAXLEN * sizeof(char));
    }
}

void ClientThread::slot_send_bytes(const char *bytes)
{
    if (send(clientfd, bytes, strlen(bytes), 0) < 0)
    {
        log("error", "Cannot send data to client");
        return;
    }
    log("info", QString("Send data to client, length=%1, content=%2").arg(strlen(bytes)).arg(bytes));
}

void ClientThread::slot_send_json(QJsonObject jsonObject)
{
    slot_send_bytes(jsonToString(jsonObject));
}

void ClientThread::parseReceived(const char* msg, int length)
{
    QJsonObject jsonRec = stringToJson(msg, length);
    QString action = jsonRec.find("action").value().toString();

    log("info", QString("Parsing: client action=%1, json=%2").arg(action).arg(jsonToString(jsonRec).data()));

    /*
    action: "client_login"
    username: "zhm_x"
    password: "123456"
     */
    if (action == "client_login")
    {
        log("info", "Action: client logining");
        QString username = jsonRec.find("username").value().toString();
        QString password = jsonRec.find("password").value().toString();

        emit signal_validate_user(username, password);
        // call ServerThread slot_validate_user
        // ServerThread direct call slot_validate_user
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
        log("info", "Action: client login succeeded");
        log("info", QString("username = %1").arg(user->getUsername()));
    }
    else
    {
        jsonSend.insert("correct", QJsonValue(false));
        log("info", "Action: client login failed");
        this->user = user;
    }
    slot_send_json(jsonSend);
}

