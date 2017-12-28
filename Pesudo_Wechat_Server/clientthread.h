#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QObject>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "user.h"

#define MAXLEN 1050000

class ClientThread : public QThread
{
    Q_OBJECT
public:
    ClientThread(int clientfd);

signals:
    void signal_validate_user(QString, QString);  // send to ServerThread to validate
    void signal_error_box(QString);  // send to UI
    void signal_info_box(QString);  // send to UI
    void signal_new_friend(QString myName, QString username);  // send to ServerThread
    void signal_send_friend_list(QString username, bool sendForNew, bool succeeded);  // send to ServerThread
    void signal_receive_text(QDateTime time, QString sender, QString receiver, QString text);  // emit to mainthread
    void signal_receive_file(QDateTime time, QString sender, QString receiver, QString filename, QString rawContent);  // emit to mainthread

public slots:
    void slot_send_bytes(const char *bytes);
    void slot_send_json(QJsonObject jsonObject);
    void slot_validate_user(User* user);  // send by ServerThread to receive the user
    void slot_send_friend_list(bool sendForNew, bool succeeded);  // send friends to client
    void slot_send_message_text(QDateTime time, QString sender, QString receiver, QString text);  // direct called from serverthread
    void slot_send_message_file(QDateTime time, QString sender, QString receiver, QString filename, QString rawContent);  // direct called from server thread

protected:
    void run();
    void parseReceived(const char* msg, int length);

private:
    void log(QString level, QString msg);
    QByteArray jsonToString(QJsonObject json);
    QJsonObject stringToJson(const char *bytes, int length);

    int clientfd;
    char* buffer;
    User* user;
    QString username;
};

#endif // CLIENTTHREAD_H
