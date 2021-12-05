/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MAINTANCEVALUE_H
#define _PLAYERBOT_MAINTANCEVALUE_H

#include "Value.h"

class PlayerbotAI;

class CanMoveAroundValue : public BoolCalculatedValue
{
    public:
        CanMoveAroundValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can move around", 2) {}

        bool Calculate() override;
};

class CanMoveAroundValue : public BoolCalculatedValue
{
    public:
        CanMoveAroundValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can move around", 2) {}

        bool Calculate() override;
};

class ShouldHomeBindValue : public BoolCalculatedValue
{
    public:
        ShouldHomeBindValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "should home bind", 2) {}

        bool Calculate() override;
};

class ShouldRepairValue : public BoolCalculatedValue
{
	public:
        ShouldRepairValue(PlayerbotAI* ai) : BoolCalculatedValue(ai,"should repair",2) {}

        bool Calculate() override;
};

class CanRepairValue : public BoolCalculatedValue
{
    public:
        CanRepairValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can repair",2) {}

        bool Calculate() override;
};

class ShouldSellValue : public BoolCalculatedValue
{
    public:
        ShouldSellValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "should sell",2) {}

        bool Calculate() override;
};

class CanSellValue : public BoolCalculatedValue
{
    public:
        CanSellValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can sell",2) {}

        bool Calculate() override;
};

class CanFightEqualValue: public BoolCalculatedValue
{
    public:
        CanFightEqualValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can fight equal",2) {}

        bool Calculate() override;
};

class CanFightEliteValue : public BoolCalculatedValue
{
    public:
        CanFightEliteValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can fight elite") {}

        bool Calculate() override;
};

class CanFightBossValue : public BoolCalculatedValue
{
    public:
        CanFightBossValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can fight boss") {}

        bool Calculate() override;
};

#endif
