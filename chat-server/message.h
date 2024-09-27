#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QDateTime>

class Message {
public:
    Message(int id, int senderId, int receiverId, const QString& content, const QDateTime& timestamp);

    int getId() const;           // получить ID сообщения
    int getSenderId() const;     // получить ID отправителя
    int getReceiverId() const;   // получить ID получателя
    QString getContent() const;  // получить текст сообщения
    QDateTime getTimestamp() const; // получить время отправки

private:
    int id;            // ID сообщения
    int senderId;      // ID отправителя
    int receiverId;    // ID получателя (0 для общих сообщений)
    QString content;   // текст сообщения
    QDateTime timestamp; // время отправки
};

#endif // MESSAGE_H