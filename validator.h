#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QString>
#include <QLineEdit>

class Validator
{
public:
    Validator();

    static bool validateIp(QLineEdit *field);
    static bool isValidIp(QString ip);

    static bool validatePort(QLineEdit *field);
};

#endif // VALIDATOR_H
