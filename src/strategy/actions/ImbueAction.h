/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Action.h"
class ImbueWithPoisonAction : public Action
{
public:
    ImbueWithPoisonAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

class ImbueWithStoneAction : public Action
{
    public:
        ImbueWithStoneAction(PlayerbotAI* botAI);

        bool Execute(Event event) override;
};

class ImbueWithOilAction : public Action
{
    public:
        ImbueWithOilAction(PlayerbotAI* botAI);

        bool Execute(Event event) override;
};

class TryEmergencyAction : public Action
{
    public:
        TryEmergencyAction(PlayerbotAI* botAI);

        bool Execute(Event event) override;
};
