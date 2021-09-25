/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHOOSERPGTARGETACTION_H
#define _PLAYERBOT_CHOOSERPGTARGETACTION_H

#include "MovementActions.h"

class Event;
class ObjectGuid;
class Player;
class PlayerbotAI;
class WorldObject;
class WorldLocation;

class ChooseRpgTargetAction : public MovementAction
{
    public:
        ChooseRpgTargetAction(PlayerbotAI* botAI, std::string const& name = "choose rpg target") : MovementAction(botAI, name) { }

        bool Execute(Event event) override;
        bool isUseful() override;

        static bool isFollowValid(Player* bot, WorldObject* target);
        static bool isFollowValid(Player* bot, WorldLocation location);

    private:
        virtual uint32 HasSameTarget(ObjectGuid guid);
        virtual bool CanTrain(ObjectGuid guid);
        virtual BattlegroundTypeId CanQueueBg(ObjectGuid guid);
};

class ClearRpgTargetAction : public ChooseRpgTargetAction
{
    public:
        ClearRpgTargetAction(PlayerbotAI* botAI) : ChooseRpgTargetAction(botAI, "clear rpg target") { }

        bool Execute(Event event) override;
        bool isUseful() override;
};

#endif
