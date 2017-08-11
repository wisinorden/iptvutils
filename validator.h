#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QString>
#include <QLineEdit>

class Validator
{
public:
    Validator();

    static bool validateIp(QLineEdit *field, bool emptyAllowed = false);
    static bool isValidIp(QString ip);

    static bool validatePort(QLineEdit *field, bool emptyAllowed = false);
};

#endif // VALIDATOR_H
