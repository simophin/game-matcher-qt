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
        auto upperFirstName = m.firstName.toUpper();
        auto &firstNameInfo = nameMap[upperFirstName];
        firstNameInfo.times++;
        firstNameInfo.lastNameInitials[m.lastName.left(1).toUpper()].times++;
    }

    for (Member &m : members) {
        auto upperFirstName = m.firstName.toUpper();
        if (auto firstNameInfo = getMapValue(nameMap, upperFirstName); firstNameInfo && firstNameInfo->times > 1) {
            auto lastNameInitial = m.lastName.left(1).toUpper();
            if (auto nameInfo = getMapValue(firstNameInfo->lastNameInitials, lastNameInitial); nameInfo && nameInfo->times > 1) {
                m.displayName = m.fullName().toUpper();
            } else {
                m.displayName = QObject::tr("%1 %2", "first/last name initial").arg(m.firstName.toUpper(), lastNameInitial.toUpper());
            }

        } else {
            m.displayName = m.firstName.toUpper();
        }
    }
}

template<typename Col>
static void formatMemberDisplayNames(Col &members) {
    return formatMemberDisplayNames(members, members);
}

#endif //GAMEMATCHER_NAMEFORMATUTILS_H
