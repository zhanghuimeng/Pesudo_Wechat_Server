#ifndef USER_H
#define USER_H

#include <QMap>

class User
{
public:
    User(QString username, QString password);
    QString getUsername() const;
    QString getPassword() const;

private:
    QString username;
    QString password;
};

class UserMap
{
public:
    UserMap(bool def = true);
    bool validateUser(QString username, QString password);

private:
    QMap<QString, User> userMap;
};

#endif // USER_H
