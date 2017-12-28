#ifndef MESSAGE_H
#define MESSAGE_H

#include <QJsonObject>
#include <QDateTime>
#include <QFileInfo>

class User;

class Message
{
public:
    Message();
    Message(User* sender, User* receiver, QDateTime time);
    User* getSender();
    User* getReceiver();
    QDateTime getTime() const;

protected:
    User* sender;
    User* receiver;
    QDateTime time;
};

class TextMessage: public Message
{
public:
    TextMessage();
    TextMessage(User* sender, User* receiver, QString text, QDateTime time);

private:
    QString text;

};

class FileMessage: public Message
{
public:
    FileMessage();
    FileMessage(User* sender, User* receiver, QFileInfo file, QDateTime time);

private:
    QFileInfo file;
};

#endif // MESSAGE_H
