#include "server.h"
#include <QHBoxLayout>   // подключаем нужные модули из Qt
#include <QMessageBox>
#include <QHeaderView>
#include <QDataStream>
#include <QDebug>
#include <QSplitter>

Server::Server(QWidget* parent) : QWidget(parent) {
    // инициализируем базу данных
    if (!db.connect()) {
        // если не удалось подключиться к базе, показываем критическое сообщение и выходим
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к БД PostgreSQL");
        exit(1);
    }

    // настраиваем пользовательский интерфейс
    setupUI();
    // загружаем список пользователей
    loadUsers();
    // загружаем историю сообщений
    loadMessageHistory();

    // создаём новый TCP-сервер
    tcpServer = new QTcpServer(this);
    // пытаемся начать слушать входящие подключения на любом адресе и порту 1234
    if (!tcpServer->listen(QHostAddress::Any, 1234)) {
        // если не получилось, показываем ошибку и выходим
        QMessageBox::critical(this, "Ошибка", "Не удалось запустить сервер");
        exit(1);
    }

    // подключаем сигнал нового подключения к нашему слоту
    connect(tcpServer, &QTcpServer::newConnection, this, &Server::newConnection);
}

Server::~Server() {
    // при уничтожении сервера отключаемся от базы данных
    db.disconnect();
}

void Server::setupUI() {
    // устанавливаем заголовок окна
    setWindowTitle("Сервер чата");

    // создаём виджет с вкладками
    tabWidget = new QTabWidget;

    // --- Первая вкладка - Чаты ---
    chatsTab = new QWidget; // создаём виджет для вкладки "Чаты"
    QSplitter* splitter = new QSplitter(Qt::Horizontal); // создаём сплиттер для разделения области на две части

    // создаём области для публичных и приватных сообщений
    publicChatArea = new QTextEdit;
    publicChatArea->setReadOnly(true); // делаем её только для чтения
    privateChatArea = new QTextEdit;
    privateChatArea->setReadOnly(true); // и эту тоже

    // добавляем области в сплиттер
    splitter->addWidget(publicChatArea);
    splitter->addWidget(privateChatArea);

    // создаём лэйаут для вкладки "Чаты"
    QHBoxLayout* chatsLayout = new QHBoxLayout;
    chatsLayout->addWidget(splitter); // добавляем сплиттер в лэйаут
    chatsTab->setLayout(chatsLayout); // устанавливаем лэйаут для вкладки

    // добавляем вкладку в виджет с вкладками
    tabWidget->addTab(chatsTab, "Чаты");

    // --- Вторая вкладка - Пользователи ---
    usersTab = new QWidget; // создаём виджет для вкладки "Пользователи"
    userTable = new QTableWidget; // создаём таблицу пользователей
    userTable->setColumnCount(3); // устанавливаем количество столбцов
    userTable->setHorizontalHeaderLabels({ "ID", "Имя", "Статус" }); // задаём заголовки столбцов
    userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // растягиваем столбцы по ширине

    // создаём кнопки "Забанить" и "Разбанить"
    banButton = new QPushButton("Забанить");
    unbanButton = new QPushButton("Разбанить");

    // подключаем сигналы нажатия кнопок к соответствующим слотам
    connect(banButton, &QPushButton::clicked, this, &Server::banUser);
    connect(unbanButton, &QPushButton::clicked, this, &Server::unbanUser);

    // создаём лэйауты для вкладки "Пользователи"
    QVBoxLayout* usersLayout = new QVBoxLayout;
    usersLayout->addWidget(userTable); // добавляем таблицу в лэйаут

    // лэйаут для кнопок
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(banButton);
    buttonLayout->addWidget(unbanButton);
    usersLayout->addLayout(buttonLayout); // добавляем лэйаут с кнопками в основной лэйаут

    usersTab->setLayout(usersLayout); // устанавливаем лэйаут для вкладки

    // добавляем вкладку "Пользователи" в виджет с вкладками
    tabWidget->addTab(usersTab, "Пользователи");

    // --- Третья вкладка - Лог ---
    logTab = new QWidget; // создаём виджет для вкладки "Лог"
    logArea = new QTextEdit;
    logArea->setReadOnly(true); // делаем область только для чтения

    // создаём лэйаут для вкладки "Лог"
    QVBoxLayout* logLayout = new QVBoxLayout;
    logLayout->addWidget(logArea);
    logTab->setLayout(logLayout);

    // добавляем вкладку "Лог" в виджет с вкладками
    tabWidget->addTab(logTab, "Лог");

    // основной лэйаут окна
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget); // добавляем виджет с вкладками в основной лэйаут
    setLayout(mainLayout); // устанавливаем основной лэйаут для окна

    // --- Загрузка стилей ---
    QFile styleFile("./styles.qss"); // открываем файл стилей
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = styleFile.readAll(); // читаем содержимое файла
        setStyleSheet(styleSheet); // применяем стили к приложению
        styleFile.close(); // закрываем файл
    }
}

void Server::loadUsers() {
    // получаем список всех пользователей из базы данных
    QList<User> users = db.getAllUsers();

    // устанавливаем количество строк в таблице пользователей
    userTable->setRowCount(users.size());
    // заполняем таблицу данными
    for (int i = 0; i < users.size(); ++i) {
        // создаём элементы таблицы
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(users[i].getId())); // ID пользователя
        QTableWidgetItem* nameItem = new QTableWidgetItem(users[i].getName()); // имя пользователя
        QString status = users[i].isBanned() ? "Забанен" : "Активен"; // статус пользователя
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);

        // добавляем элементы в таблицу
        userTable->setItem(i, 0, idItem);
        userTable->setItem(i, 1, nameItem);
        userTable->setItem(i, 2, statusItem);
    }
}

void Server::loadMessageHistory() {
    // получаем всю историю сообщений из базы данных
    QList<Message> messages = db.getAllMessages();

    // перебираем сообщения и добавляем их в соответствующие области
    for (const Message& msg : messages) {
        QString formattedMessage; // форматированное сообщение для отображения
        if (msg.getReceiverId() == 0) {
            // если это публичное сообщение
            formattedMessage = QString("[ALL] '%1': %2").arg(db.getUserById(msg.getSenderId()).getLogin(), msg.getContent());
            publicChatArea->append(formattedMessage); // добавляем его в область публичного чата
        }
        else {
            // если это приватное сообщение
            formattedMessage = QString("[PM] '%1' -> '%2': %3")
                .arg(db.getUserById(msg.getSenderId()).getLogin(),
                    db.getUserById(msg.getReceiverId()).getLogin(),
                    msg.getContent());
            privateChatArea->append(formattedMessage); // добавляем в область приватного чата
        }

        // также добавляем сообщение в лог
        logArea->append(formattedMessage);
    }
}

void Server::newConnection() {
    // получаем сокет нового подключения
    QTcpSocket* clientSocket = tcpServer->nextPendingConnection();
    int clientId = clientSocket->socketDescriptor(); // получаем дескриптор сокета
    clients[clientId] = clientSocket; // добавляем сокет в список клиентов

    // подключаем сигналы от клиента
    connect(clientSocket, &QTcpSocket::readyRead, this, &Server::readData); // когда есть данные для чтения
    connect(clientSocket, &QTcpSocket::disconnected, this, &Server::clientDisconnected); // когда клиент отключился

    // логируем новое подключение
    QString logMsg = "Новое подключение: " + QString::number(clientId);
    logArea->append(logMsg);
}

void Server::readData() {
    // получаем сокет, от которого пришли данные
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    QDataStream in(clientSocket); // создаём поток для чтения данных
    in.setVersion(QDataStream::Qt_5_15); // устанавливаем версию протокола

    // пока есть данные для чтения
    while (clientSocket->bytesAvailable()) {
        QString command;
        in >> command; // читаем команду от клиента
        qDebug() << "Received command:" << command; // выводим команду в отладку

        if (command == "REGISTER") {
            // обработка регистрации пользователя
            QString login, password, name;
            in >> login >> password >> name; // получаем данные регистрации

            // пытаемся зарегистрировать пользователя в базе данных
            bool success = db.registerUser(login, password, name);
            QDataStream out(clientSocket); // создаём поток для отправки данных клиенту
            out.setVersion(QDataStream::Qt_5_15);
            out << QString(success ? "REGISTER_SUCCESS" : "REGISTER_FAIL"); // отправляем результат регистрации
            clientSocket->waitForBytesWritten(); // ждём, пока данные отправятся

            // логируем результат регистрации
            logArea->append("Регистрация пользователя: " + login + " - " + (success ? "успешна" : "провалилась"));
            loadUsers(); // обновляем список пользователей в интерфейсе
        }
        else if (command == "LOGIN") {
            // обработка входа пользователя
            QString login, password;
            in >> login >> password; // получаем данные для входа

            // пытаемся аутентифицировать пользователя
            User user = db.authenticateUser(login, password);
            QDataStream out(clientSocket); // создаём поток для отправки данных клиенту
            out.setVersion(QDataStream::Qt_5_15);

            if (user.getId() != 0 && !user.isBanned()) {
                // если пользователь найден и не забанен
                out << QString("LOGIN_SUCCESS") << user.getId() << user.getName(); // отправляем успешный вход
                clientSocket->waitForBytesWritten();
                logArea->append("Пользователь вошел: " + login);

                // сохраняем соответствие между userId и сокетом
                userSockets[user.getId()] = clientSocket;

                // отправляем непрочитанные сообщения пользователю
                QList<Message> unreadMessages = db.getUnreadMessagesForUser(user.getId());
                for (const Message& message : unreadMessages) {
                    QDataStream messageOut(clientSocket);
                    messageOut.setVersion(QDataStream::Qt_5_15);
                    messageOut << QString("NEW_MESSAGE") << message.getSenderId() << message.getReceiverId() << message.getContent();
                    clientSocket->waitForBytesWritten();
                }

                // отмечаем сообщения как прочитанные
                db.markMessagesAsRead(user.getId());

                // отправляем сигнал окончания сообщений
                QDataStream endOut(clientSocket);
                endOut.setVersion(QDataStream::Qt_5_15);
                endOut << QString("END_OF_MESSAGES");
                clientSocket->waitForBytesWritten();
            }
            else if (user.isBanned()) {
                // если пользователь забанен
                out << QString("LOGIN_BANNED");
                clientSocket->waitForBytesWritten();
                logArea->append("Пользователь заблокирован: " + login);
            }
            else {
                // если неверные данные для входа
                out << QString("LOGIN_FAIL");
                clientSocket->waitForBytesWritten();
                logArea->append("Вход не удался: " + login);
            }
        }
        else if (command == "MESSAGE") {
            // обработка отправки сообщения
            int senderId, receiverId;
            QString content;
            in >> senderId >> receiverId >> content; // получаем данные сообщения

            // проверяем, не забанен ли отправитель
            User sender = db.getUserById(senderId);
            if (sender.isBanned()) {
                QDataStream out(clientSocket);
                out.setVersion(QDataStream::Qt_5_15);
                out << QString("USER_BANNED"); // сообщаем клиенту, что он забанен
                clientSocket->waitForBytesWritten();
                return; // прекращаем обработку
            }

            // сохраняем сообщение в базе данных
            db.saveMessage(senderId, receiverId, content);

            QString formattedMessage; // форматированное сообщение для отображения
            if (receiverId == 0) {
                // если сообщение публичное
                formattedMessage = QString("[ALL] '%1': %2").arg(sender.getLogin(), content);
                publicChatArea->append(formattedMessage); // добавляем в область публичного чата
            }
            else {
                // если сообщение приватное
                User receiver = db.getUserById(receiverId);
                formattedMessage = QString("[PM] '%1' -> '%2': %3").arg(sender.getLogin(), receiver.getLogin(), content);
                privateChatArea->append(formattedMessage); // добавляем в область приватного чата
            }

            // добавляем сообщение в лог
            logArea->append(formattedMessage);

            // отправляем сообщение получателям
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_5_15);
            out << QString("NEW_MESSAGE") << senderId << receiverId << content;

            if (receiverId == 0) {
                // если сообщение для всех, отправляем всем подключенным клиентам
                foreach(QTcpSocket * client, clients) {
                    client->write(block);
                    client->waitForBytesWritten();
                }
            }
            else {
                // если сообщение для конкретного пользователя
                QTcpSocket* receiverSocket = userSockets.value(receiverId, nullptr);
                if (receiverSocket) {
                    receiverSocket->write(block);
                    receiverSocket->waitForBytesWritten();
                }
                else {
                    // если получатель не подключен
                    logArea->append("Пользователь с ID " + QString::number(receiverId) + " не подключен.");
                }
            }

            // логируем отправку сообщения
            logArea->append("Сообщение от пользователя " + QString::number(senderId) + ": " + content);
        }
        else {
            // если команда неизвестна, выводим предупреждение и пропускаем оставшиеся данные
            qDebug() << "Неизвестная команда от клиента:" << command;
            in.skipRawData(clientSocket->bytesAvailable());
        }
    }
}

void Server::clientDisconnected() {
    // обработка отключения клиента
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    int clientId = clientSocket->socketDescriptor();

    // удаляем сокет из списка клиентов
    clients.remove(clientId);

    // удаляем соответствие userId и сокета из userSockets
    int userId = -1;
    for (auto it = userSockets.begin(); it != userSockets.end(); ++it) {
        if (it.value() == clientSocket) {
            userId = it.key();
            userSockets.erase(it);
            break;
        }
    }

    // удаляем сокет
    clientSocket->deleteLater();

    // логируем отключение клиента
    QString logMsg = "Клиент отключился: " + QString::number(clientId);
    logArea->append(logMsg);
}

void Server::banUser() {
    // получаем выбранные элементы в таблице пользователей
    QList<QTableWidgetItem*> selectedItems = userTable->selectedItems();
    if (selectedItems.isEmpty()) return; // если ничего не выбрано, выходим

    int row = selectedItems.first()->row(); // получаем номер строки
    int userId = userTable->item(row, 0)->text().toInt(); // получаем ID пользователя из таблицы

    // вызываем метод бана пользователя в базе данных
    db.banUser(userId);
    loadUsers(); // обновляем список пользователей в интерфейсе

    // логируем действие
    QString logMsg = "Пользователь забанен: " + QString::number(userId);
    logArea->append(logMsg);
}

void Server::unbanUser() {
    // получаем выбранные элементы в таблице пользователей
    QList<QTableWidgetItem*> selectedItems = userTable->selectedItems();
    if (selectedItems.isEmpty()) return; // если ничего не выбрано, выходим

    int row = selectedItems.first()->row(); // получаем номер строки
    int userId = userTable->item(row, 0)->text().toInt(); // получаем ID пользователя из таблицы

    // вызываем метод разбана пользователя в базе данных
    db.unbanUser(userId);
    loadUsers(); // обновляем список пользователей в интерфейсе

    // логируем действие
    QString logMsg = "Пользователь разбанен: " + QString::number(userId);
    logArea->append(logMsg);
}