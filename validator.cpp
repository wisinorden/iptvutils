#include "validator.h"

#include <QRegExpValidator>

Validator::Validator()
{

}

bool Validator::validateIp(QLineEdit *field, bool emptyAllowed) {
    if ((emptyAllowed && field->text().length() == 0) || isValidIp(field->text())) {
        field->setStyleSheet("");
        return true;
    }
    else {
        field->setStyleSheet("QLineEdit{background: #ffd3d3;}");
        return false;
    }
}

bool Validator::isValidIp(QString ip) {
    QStringList parts = ip.split('.');

    if (parts.length() != 4)
        return false;

    bool ok;
    for (int i = 0; i < parts.length(); i++) {
        uint num = parts.at(i).toUInt(&ok);
        if (!ok || num > 255)
            return false;
    }

    return true;
}

bool Validator::validatePort(QLineEdit *field, bool emptyAllowed) {
    bool ok;
    uint num = field->text().toUInt(&ok);

    if ((ok && num <= 65535) || (emptyAllowed && field->text().length() == 0)) {
        field->setStyleSheet("");
        return true;
    }
    else {
        field->setStyleSheet("QLineEdit{background: #ffd3d3;}");
        return false;
    }
}
