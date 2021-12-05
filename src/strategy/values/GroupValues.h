/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Value.h"

class PlayerbotAI;

class GroupMembersValue : public ObjectGuidListCalculatedValue
{
    public:
        GroupMembersValue(PlayerbotAI* ai) : ObjectGuidListCalculatedValue(ai, "group members", 2) {}

        GuidVector Calculate() override;
};

class IsFollowingPartyValue : public BoolCalculatedValue
{
    public:
        IsFollowingPartyValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "following party") {}

        bool Calculate() override;
};

class IsNearLeaderValue : public BoolCalculatedValue
{
    public:
        IsNearLeaderValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "near leader") {}

        bool Calculate() override;
};

class BoolANDValue : public BoolCalculatedValue, public Qualified
{
    public:
        BoolANDValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "bool and") {}

        bool Calculate() override;
};

class GroupBoolCountValue : public Uint32CalculatedValue, public Qualified
{
    public:
        GroupBoolCountValue(PlayerbotAI* ai) : Uint32CalculatedValue(ai, "group count") {}

       uint32 Calculate() override;
};

class GroupBoolANDValue : public BoolCalculatedValue, public Qualified
{
    public:
        GroupBoolANDValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "group bool and") {}

        bool Calculate() override;
};

class GroupBoolORValue : public BoolCalculatedValue, public Qualified
{
    public:
        GroupBoolORValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "group bool or") {}

        bool Calculate() override;
};

class GroupReadyValue : public BoolCalculatedValue, public Qualified
{
    public:
        GroupReadyValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "group ready", 2) {}

        bool Calculate()override;
};
