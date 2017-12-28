#ifndef USER_H
#define USER_H

#include <QMap>
#include <QSet>
#include <QList>
#include "message.h"

class User
{
public:
    User(QString username, QString password);
    QString getUsername() const;
    QString getPassword() const;
    void addFriend(User* user);
    QList<User*> getFriendList();

private:
    QString username;
    QString password;
    QSet<User*> friendSet;
    QList<Message> messageToReceiveList;
};

class UserMap
{
public:
    UserMap(bool def = true);
    bool validateUser(QString username, QString password);
    User* findUser(QString username);

private:
    QMap<QString, User*> userMap;
};

#endif // USER_H
