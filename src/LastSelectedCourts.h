//
// Created by Fanchao Liu on 2/08/20.
//

#ifndef GAMEMATCHER_LASTSELECTEDCOURTS_H
#define GAMEMATCHER_LASTSELECTEDCOURTS_H

#include "models.h"

#include <QSet>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <optional>

struct LastSelectedCourt {
    SessionId sessionId = 0;
    QSet<CourtId> selectedCourts;

    QString toString() const {
        QJsonObject doc;

        doc[QStringLiteral("sessionId")] = sessionId;

        QJsonArray array;
        for (const auto &court : selectedCourts) {
            array.push_back(court);
        }

        doc[QStringLiteral("selectedCourts")] = array;
        return QString::fromUtf8(QJsonDocument(doc).toJson());
    }

    static std::optional<LastSelectedCourt> fromString(const QString &str) {
        std::optional<LastSelectedCourt> c;
        if (str.isNull()) return c;

        auto doc = QJsonDocument::fromJson(str.toUtf8());
        if (!doc.isObject()) return c;

        auto obj = doc.object();

        c.emplace().sessionId = obj.value(QStringLiteral("sessionId")).toVariant().value<SessionId>();
        QJsonArray arr = obj.value(QStringLiteral("selectedCourts")).toArray();
        for (const auto &item : arr) {
            c->selectedCourts.insert(item.toVariant().value<CourtId>());
        }
        return c;
    }
};

#endif //GAMEMATCHER_LASTSELECTEDCOURTS_H
