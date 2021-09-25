/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RPGACTION_H
#define _PLAYERBOT_RPGACTION_H

#include "MovementActions.h"

class Event;
class ObjectGuid;
class PlayerbotAI;
class Unit;

class RpgAction : public MovementAction
{
    public:
        RpgAction(PlayerbotAI* botAI) : MovementAction(botAI, "rpg") { }

        bool Execute(Event event) override;
        bool isUseful() override;

    private:
        typedef void(RpgAction::*RpgElement)(ObjectGuid guid);

        bool AddIgnore(ObjectGuid guid);
        bool RemIgnore(ObjectGuid guid);
        bool HasIgnore(ObjectGuid guid);

        void stay(ObjectGuid guid);
        void cancel(ObjectGuid guid);
        void emote(ObjectGuid guid);
        void work(ObjectGuid guid);
        void taxi(ObjectGuid guid);
        void quest(ObjectGuid guid);
        void trade(ObjectGuid guid);
        void repair(ObjectGuid guid);
        void train(ObjectGuid guid);
        void heal(ObjectGuid guid);
        void use(ObjectGuid guid);
        void spell(ObjectGuid guid);
        void craft(ObjectGuid guid);
        void homebind(ObjectGuid guid);
        void queuebg(ObjectGuid guid);

        bool CanTrain(ObjectGuid guid);
        BattlegroundTypeId CanQueueBg(ObjectGuid guid);
};

#endif
