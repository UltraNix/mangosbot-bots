/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "NamedObjectContext.h"
#include "Value.h"

class Item;
class PlayerbotAI;
class SpellInfo;

class ItemForSpellValue : public CalculatedValue<Item*>, public Qualified
{
	public:
        ItemForSpellValue(PlayerbotAI* botAI) : CalculatedValue<Item*>(botAI) { }

        Item* Calculate() override;

    private:
        Item* GetItemFitsToSpellRequirements(uint8 slot, SpellInfo const *spellInfo);
};
