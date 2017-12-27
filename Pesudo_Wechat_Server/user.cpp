#include "user.h"

UserMap::UserMap(bool def/*= true*/)
{
    if (def)
    {
        // 10 zhms
        for (int i = 0; i < 10; i++)
        {
            QString username = QString("zhm_%1").arg(i);
            QString password = username;
            User user(username, password);
            this->userMap.insert(username, user);
        }
    }
}

bool UserMap::validateUser(QString username, QString password)
{
    if (!userMap.contains(username))
        return false;
    if (userMap.find(username).value().getPassword() != password)
        return false;
    return true;
}

User* UserMap::findUser(QString username)
{
    if (!userMap.contains(username))
        return NULL;
    return &userMap.find(username).value();
}

User::User(QString username, QString password): username(username), password(password)
{

}

QString User::getUsername() const
{
    return username;
}

QString User::getPassword() const
{
    return password;
}
