/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Trigger.h"
class LfgProposalActiveTrigger : public Trigger
{
    public:
        LfgProposalActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lfg proposal active", 20) { }

        bool IsActive() override;
};
