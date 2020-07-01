//
// Created by Fanchao Liu on 25/06/20.
//

#include "DbUtils.h"

#include <QRegularExpression>


QueryResult<QSqlQuery> DbUtils::buildQuery(QSqlDatabase &db, const QString &sql, const QVector<QVariant> &binds) {
    QueryResult<QSqlQuery> rc;
    QSqlQuery q(db);
    if (!q.prepare(sql)) {
        qWarning() << "Error preparing: " << q.lastQuery() << ": " << q.lastError();
        rc.result = q.lastError();
        return rc;
    }

    for (int i = 0, size = binds.size(); i < size; i++) {
        q.bindValue(i, binds[i]);
    }

    if (!q.exec()) {
        qWarning() << "Error preparing: " << q.lastQuery() << ": " << q.lastError();
        rc.result = q.lastError();
        return rc;
    }

    rc.result = q;
    return rc;
}