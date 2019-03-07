#include "validator.h"

#include <QRegExpValidator>

Validator::Validator()
{

}

bool Validator::validateIp(QLineEdit *field) {
    if (isValidIp(field->text())) {
        return true;
    }
    else {
        return false;
    }

}

bool Validator::isValidIp(QString ip) {
    QStringList parts = ip.split('.');

    if (parts.length() != 4 || parts.at(0).toInt() > 239 || parts.at(0).toInt() < 224){
        return false;
    }

    bool ok;
    for (int i = 0; i < parts.length(); i++) {
        uint num = parts.at(i).toUInt(&ok);
        if (!ok || num > 255)
            return false;
    }

    return true;
}

bool Validator::validatePort(QLineEdit *field) {
    bool ok;
    uint port = field->text().toUShort(&ok);

    if(ok) {

        return true;
    }


    else {
        return false;
    }
}
