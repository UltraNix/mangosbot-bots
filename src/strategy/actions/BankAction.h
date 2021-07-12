/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "InventoryAction.h"

class Item;

class BankAction : public InventoryAction
{
    public:
        BankAction(PlayerbotAI* botAI) : InventoryAction(botAI, "bank") { }
        bool Execute(Event event) override;

    private:
        bool Execute(std::string const& text, Unit* bank);
        void ListItems();
        bool Withdraw(uint32 itemid);
        bool Deposit(Item* pItem);
        Item* FindItemInBank(uint32 ItemId);
};
