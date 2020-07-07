//
// Created by Fanchao Liu on 6/07/20.
//

#ifndef GAMEMATCHER_NAMEFORMATUTILS_H
#define GAMEMATCHER_NAMEFORMATUTILS_H

#include <QHash>

#include "models.h"
#include "CollectionUtils.h"


template<typename Col, typename ReferenceCol>
static void formatMemberDisplayNames(Col &members, const ReferenceCol &ref) {
    typedef QString FirstName;
    typedef QString LastNameInitial;

    struct NameInfo {
        int times = 0;
    };

    struct FirstNameInfo : NameInfo {
        QHash<LastNameInitial, NameInfo> lastNameInitials;
    };
    QHash<FirstName, FirstNameInfo> nameMap;

    for (const Member &m : ref) {
        auto lowerFirstName = m.firstName.toLower();
        auto &firstNameInfo = nameMap[lowerFirstName];
        firstNameInfo.times++;
        firstNameInfo.lastNameInitials[m.lastName.left(1).toLower()].times++;
    }

    for (Member &m : members) {
        auto lowerFirstName = m.firstName.toLower();
        if (auto firstNameInfo = getMapValue(nameMap, lowerFirstName); firstNameInfo && firstNameInfo->times > 1) {
            auto lastNameInitial = m.lastName.left(1).toLower();
            if (auto nameInfo = getMapValue(firstNameInfo->lastNameInitials, lastNameInitial); nameInfo && nameInfo->times > 1) {
                m.displayName = m.fullName();
            } else {
                m.displayName = QObject::tr("%1 %2", "first/last name initial").arg(m.firstName, lastNameInitial.toUpper());
            }

        } else {
            m.displayName = m.firstName;
        }
    }
}

template<typename Col>
static void formatMemberDisplayNames(Col &members) {
    return formatMemberDisplayNames(members, members);
}

#endif //GAMEMATCHER_NAMEFORMATUTILS_H
