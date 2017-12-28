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
            this->userMap.insert(username, new User(username, password));
        }
        // add some friends
        for (int i = 0; i < 10; i++)
        {
            QString username1 = QString("zhm_%1").arg(i);
            User* user1 = this->findUser(username1);
            QString username2 = QString("zhm_%1").arg((i+1) % 10);
            User* user2 = this->findUser(username2);
            user1->addFriend(user2);
        }
    }
}

bool UserMap::validateUser(QString username, QString password)
{
    if (!userMap.contains(username))
        return false;
    if (userMap.find(username).value()->getPassword() != password)
        return false;
    return true;
}

User* UserMap::findUser(QString username)
{
    if (!userMap.contains(username))
        return NULL;
    return userMap.find(username).value();
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

// bidirectional
void User::addFriend(User *user)
{
    if (this == user)
        return;
    if (this->friendSet.contains(user))
        return;
    this->friendSet.insert(user);
    user->addFriend(this);
}

QList<User*> User::getFriendList()
{
    return friendSet.toList();
}
