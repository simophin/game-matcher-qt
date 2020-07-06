//
// Created by Fanchao Liu on 6/07/20.
//

#ifndef GAMEMATCHER_NAMEFORMATUTILS_H
#define GAMEMATCHER_NAMEFORMATUTILS_H

#include <QHash>

#include "models.h"

template<typename Col>
static void formatMemberDisplayNames(Col &members) {
    QHash<QString, Member *> memberByFirstName;
    for (Member &m : members) {
        auto lowerFirstName = m.firstName.toLower();
        if (auto found = memberByFirstName.constFind(lowerFirstName); found != memberByFirstName.constEnd()) {
            m.displayName = m.fullName();
            (*found)->displayName = (*found)->fullName();
            memberByFirstName[lowerFirstName] = &m;
        } else {
            m.displayName = m.firstName;
        }
    }
}

#endif //GAMEMATCHER_NAMEFORMATUTILS_H
