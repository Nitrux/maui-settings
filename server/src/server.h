#ifndef SERVER_H
#define SERVER_H

#include <QCoreApplication>
#include <QObject>

class Server : public QCoreApplication
{
    Q_OBJECT
public:
    explicit Server(int &argc, char **argv);
};

#endif // SERVER_H
