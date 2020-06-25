//
// Created by Fanchao Liu on 25/06/20.
//

#include "dbutils.h"

#include <QRegularExpression>

QString camelCaseToUnderscore(const QString &s) {
    static QRegularExpression regExp1 {QLatin1String("(.)([A-Z][a-z]+)")};
    static QRegularExpression regExp2 {QLatin1String("([a-z0-9])([A-Z])")};

    QString result = s;
    result.replace(regExp1, QLatin1String("\\1_\\2"));
    result.replace(regExp2, QLatin1String("\\1_\\2"));

    return result.toLower();
}
