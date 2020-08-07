//
// Created by Fanchao Liu on 7/08/20.
//

#include "models.h"
#include "ClubRepository.h"


void registerModels() {
    qRegisterMetaType<GameId>("GameId");
    qRegisterMetaType<CourtId>("CourtId");
    qRegisterMetaType<SessionId>("SessionId");
    qRegisterMetaType<MemberId>("MemberId");
    qRegisterMetaType<SettingKey>("SettingKey");
    qRegisterMetaType<Member>();
    qRegisterMetaType<CourtPlayers>();
}