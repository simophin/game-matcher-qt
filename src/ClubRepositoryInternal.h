//
// Created by Fanchao Liu on 1/07/20.
//

#ifndef GAMEMATCHER_CLUBREPOSITORYINTERNAL_H
#define GAMEMATCHER_CLUBREPOSITORYINTERNAL_H

#include "models.h"
#include "ClubRepository.h"

#include <QObject>

struct GameAllocationMember : MemberInfo {
    Q_GADGET
public:
    DECLARE_PROPERTY(CourtId, courtId, )
    DECLARE_PROPERTY(QString, courtName, )
};


#endif //GAMEMATCHER_CLUBREPOSITORYINTERNAL_H
