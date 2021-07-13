/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MailAction.h"
#include "Mail.h"
#include "Event.h"
#include "ChatHelper.h"
#include "Helpers.h"
#include "Playerbot.h"

std::map<std::string, MailProcessor*> MailAction::processors;

class TellMailProcessor : public MailProcessor
{
    public:
       bool Before(PlayerbotAI* botAI) override
        {
            botAI->TellMaster("=== Mailbox ===");
            tells.clear();
            return true;
        }

        bool Process(uint32 index, Mail* mail, PlayerbotAI* botAI) override
        {
            Player* bot = botAI->GetBot();
            time_t cur_time = time(nullptr);
            uint32 days = (cur_time - mail->deliver_time) / 3600 / 24;

            std::ostringstream out;
            out << "#" << (index+1) << " ";
            if (!mail->money && !mail->HasItems())
                out << "|cffffffff" << mail->subject;

            if (mail->money)
            {
                out << "|cffffff00" << ChatHelper::formatMoney(mail->money);
                if (!mail->subject.empty())
                    out << " |cffa0a0a0(" << mail->subject << ")";
            }

            if (mail->HasItems())
            {
                for (MailItemInfoVec::iterator i = mail->items.begin(); i != mail->items.end(); ++i)
                {
                    Item* item = bot->GetMItem(i->item_guid);
                    uint32 count = item ? item->GetCount() : 1;

                    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(i->item_template))
                    {
                        out << ChatHelper::formatItem(proto, count);
                        if (!mail->subject.empty())
                            out << " |cffa0a0a0(" << mail->subject << ")";
                    }
                }
            }

            out  << ", |cff00ff00" << days << " day(s)";
            tells.push_front(out.str());
            return true;
        }

        bool After(PlayerbotAI* botAI) override
        {
            for (std::list<std::string>::iterator i = tells.begin(); i != tells.end(); ++i)
                botAI->TellMaster(*i);

            return true;
        }

        static TellMailProcessor instance;

    private:
        std::list<std::string> tells;
};

class TakeMailProcessor : public MailProcessor
{
    public:
        bool Process(uint32 index, Mail* mail, PlayerbotAI* botAI) override
        {
            Player* bot = botAI->GetBot();
            if (!CheckBagSpace(bot))
            {
                botAI->TellError("Not enough bag space");
                return false;
            }

            ObjectGuid mailbox = FindMailbox(botAI);
            if (mail->money)
            {
                std::ostringstream out;
                out << mail->subject << ", |cffffff00" << ChatHelper::formatMoney(mail->money) << "|cff00ff00 processed";
                botAI->TellMaster(out.str());

                WorldPacket packet;
                packet << mailbox;
                packet << mail->messageID;
                bot->GetSession()->HandleMailTakeMoney(packet);
                RemoveMail(bot, mail->messageID, mailbox);
            }
            else if (!mail->items.empty())
            {
                std::vector<uint32> guids;
                for (MailItemInfoVec::iterator i = mail->items.begin(); i != mail->items.end(); ++i)
                    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(i->item_template))
                        guids.push_back(i->item_guid);

                for (std::vector<uint32>::iterator i = guids.begin(); i != guids.end(); ++i)
                {
                    WorldPacket packet;
                    packet << mailbox;
                    packet << mail->messageID;
                    packet << *i;

                    Item* item = bot->GetMItem(*i);

                    std::ostringstream out;
                    out << mail->subject << ", " << ChatHelper::formatItem(item->GetTemplate()) << "|cff00ff00 processed";

                    bot->GetSession()->HandleMailTakeItem(packet);
                    botAI->TellMaster(out.str());
                }

                RemoveMail(bot, mail->messageID, mailbox);
            }

            return true;
        }

        static TakeMailProcessor instance;

    private:
        bool CheckBagSpace(Player* bot)
        {
            uint32 totalused = 0, total = 16;
            for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
                if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                    ++totalused;

            uint32 totalfree = 16 - totalused;
            for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
            {
                if (Bag const* pBag = (Bag*) bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
                {
                    ItemTemplate const* pBagProto = pBag->GetTemplate();
                    if (pBagProto->Class == ITEM_CLASS_CONTAINER && pBagProto->SubClass == ITEM_SUBCLASS_CONTAINER)
                        totalfree += pBag->GetFreeSlots();
                }

            }

            return totalfree >= 2;
        }
};

class DeleteMailProcessor : public MailProcessor
{
    public:
        bool Process(uint32 index, Mail* mail, PlayerbotAI* botAI) override
        {
            std::ostringstream out;
            out << "|cffffffff" << mail->subject << "|cffff0000 deleted";
            RemoveMail(botAI->GetBot(), mail->messageID, FindMailbox(botAI));
            botAI->TellMaster(out.str());
            return true;
        }

        static DeleteMailProcessor instance;
};

class ReadMailProcessor : public MailProcessor
{
    public:
        bool Process(uint32 index, Mail* mail, PlayerbotAI* botAI) override
        {
            std::ostringstream out, body;
            out << "|cffffffff" << mail->subject;
            botAI->TellMaster(out.str());

            if (mail->itemTextId)
            {
                body << "\n" << sObjectMgr->GetItemText(mail->itemTextId);
                botAI->TellMaster(body.str());
            }

            return true;
        }

        static ReadMailProcessor instance;
};

TellMailProcessor TellMailProcessor::instance;
TakeMailProcessor TakeMailProcessor::instance;
DeleteMailProcessor DeleteMailProcessor::instance;
ReadMailProcessor ReadMailProcessor::instance;

bool MailAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!MailProcessor::FindMailbox(botAI))
    {
        botAI->TellError("There is no mailbox nearby");
        return false;
    }

    if (processors.empty())
    {
        processors["?"] = &TellMailProcessor::instance;
        processors["take"] = &TakeMailProcessor::instance;
        processors["delete"] = &DeleteMailProcessor::instance;
        processors["read"] = &ReadMailProcessor::instance;
    }

    std::string const& text = event.getParam();
    if (text.empty())
    {
        botAI->TellMaster("whisper 'mail ?' to query mailbox, 'mail take/delete/read filter' to take/delete/read mails by filter");
        return false;
    }

    std::vector<std::string> ss = split(text, ' ');
    std::string const& action = ss[0];
    std::string const& filter = ss.size() > 1 ? ss[1] : "";

    MailProcessor* processor = processors[action];
    if (!processor)
    {
        std::ostringstream out; out << action << ": I don't know how to do that";
        botAI->TellMaster(out.str());
        return false;
    }

    if (!processor->Before(botAI))
        return false;

    std::vector<Mail*> mailList;
    time_t cur_time = time(nullptr);
    for (PlayerMails::iterator itr = bot->GetMailBegin(); itr != bot->GetMailEnd(); ++itr)
    {
        if ((*itr)->state == MAIL_STATE_DELETED || cur_time < (*itr)->deliver_time)
            continue;

        Mail* mail = *itr;
        mailList.push_back(mail);
    }

    std::map<uint32, Mail*> filtered = filterList(mailList, filter);
    for (std::map<uint32, Mail*>::iterator i = filtered.begin(); i != filtered.end(); ++i)
    {
        if (!processor->Process(i->first, i->second, botAI))
            break;
    }

    return processor->After(botAI);
}

void MailProcessor::RemoveMail(Player* bot, uint32 id, ObjectGuid mailbox)
{
    WorldPacket packet;
    packet << mailbox;
    packet << id;
    packet << (uint32)0; //mailTemplateId
    bot->GetSession()->HandleMailDelete(packet);
}

ObjectGuid MailProcessor::FindMailbox(PlayerbotAI* botAI)
{
    GuidVector gos = *botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects");
    ObjectGuid mailbox;
    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            if (go->GetGoType() == GAMEOBJECT_TYPE_MAILBOX)
            {
                mailbox = go->GetGUID();
                break;
            }
    }

    return mailbox;
}
