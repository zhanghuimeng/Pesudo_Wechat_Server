#include "message.h"

Message::Message(): sender(NULL), receiver(NULL)
{

}

Message::Message(User *sender, User *receiver, QDateTime time): sender(sender), receiver(receiver), time(time)
{

}

TextMessage::TextMessage()
{

}

TextMessage::TextMessage(User *sender, User *receiver, QString text, QDateTime time): Message(sender, receiver, time),
    text(text)
{

}

FileMessage::FileMessage()
{

}

FileMessage::FileMessage(User *sender, User *receiver, QFileInfo file, QDateTime time): Message(sender, receiver, time),
    file(file)
{

}
