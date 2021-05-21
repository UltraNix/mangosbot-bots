#pragma once
#include "../Value.h"

namespace botAI
{
    enum ItemUsage
    {
        ITEM_USAGE_NONE = 0,
        ITEM_USAGE_EQUIP = 1,
        ITEM_USAGE_REPLACE = 2,
        ITEM_USAGE_SKILL = 3,
        ITEM_USAGE_USE = 4,
        ITEM_USAGE_GUILD_TASK = 5,
        ITEM_USAGE_DISENCHANT = 6
    };

    class ItemUsageValue : public CalculatedValue<ItemUsage>, public Qualified
	{
	public:
        ItemUsageValue(PlayerbotAI* botAI) : CalculatedValue<ItemUsage>(botAI) { }

    public:
        virtual ItemUsage Calculate();

    private:
        ItemUsage QueryItemUsageForEquip(ItemTemplate const*  proto);
        bool IsItemUsefulForSkill(ItemTemplate const*  proto);
	};
}
