/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICPRIESTSTRATEGYACTIONNODEFACTORY_H
#define _PLAYERBOT_GENERICPRIESTSTRATEGYACTIONNODEFACTORY_H

#include "Action.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class GenericPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
    public:
        GenericPriestStrategyActionNodeFactory()
        {
            creators["inner fire"] = &inner_fire;
            creators["holy nova"] = &holy_nova;
            creators["power word: fortitude"] = &power_word_fortitude;
            creators["power word: fortitude on party"] = &power_word_fortitude_on_party;
            creators["divine spirit"] = &divine_spirit;
            creators["divine spirit on party"] = &divine_spirit_on_party;
            creators["power word: shield"] = &power_word_shield;
            creators["power word: shield on party"] = &power_word_shield_on_party;
            creators["renew"] = &renew;
            creators["renew on party"] = &renew_on_party;
            creators["greater heal"] = &greater_heal;
            creators["greater heal on party"] = &greater_heal_on_party;
            creators["heal"] = &heal;
            creators["heal on party"] = &heal_on_party;
            creators["lesser heal"] = &lesser_heal;
            creators["lesser heal on party"] = &lesser_heal_on_party;
            creators["flash heal"] = &flash_heal;
            creators["flash heal on party"] = &flash_heal_on_party;
            creators["psychic scream"] = &psychic_scream;
            creators["fade"] = &fade;
        }

    private:
        static ActionNode* inner_fire(PlayerbotAI* botAI)
        {
            return new ActionNode ("inner fire",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* holy_nova(PlayerbotAI* botAI)
        {
            return new ActionNode ("holy nova",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* power_word_fortitude(PlayerbotAI* botAI)
        {
            return new ActionNode ("power word: fortitude",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* power_word_fortitude_on_party(PlayerbotAI* botAI)
        {
            return new ActionNode ("power word: fortitude on party",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* divine_spirit(PlayerbotAI* botAI)
        {
            return new ActionNode ("divine spirit",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* divine_spirit_on_party(PlayerbotAI* botAI)
        {
            return new ActionNode ("divine spirit on party",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* power_word_shield(PlayerbotAI* botAI)
        {
            return new ActionNode ("power word: shield",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ NextAction::array(0, new NextAction("renew", 50.0f), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* power_word_shield_on_party(PlayerbotAI* botAI)
        {
            return new ActionNode ("power word: shield on party",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ NextAction::array(0, new NextAction("renew on party", 50.0f), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* renew(PlayerbotAI* botAI)
        {
            return new ActionNode ("renew",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* renew_on_party(PlayerbotAI* botAI)
        {
            return new ActionNode ("renew on party",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* greater_heal(PlayerbotAI* botAI)
        {
            return new ActionNode ("greater heal",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ NextAction::array(0, new NextAction("heal"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* greater_heal_on_party(PlayerbotAI* botAI)
        {
            return new ActionNode ("greater heal on party",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ NextAction::array(0, new NextAction("heal on party"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* heal(PlayerbotAI* botAI)
        {
            return new ActionNode ("heal",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ NextAction::array(0, new NextAction("lesser heal"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* heal_on_party(PlayerbotAI* botAI)
        {
            return new ActionNode ("heal on party",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ NextAction::array(0, new NextAction("lesser heal on party"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* lesser_heal(PlayerbotAI* botAI)
        {
            return new ActionNode ("lesser heal",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* lesser_heal_on_party(PlayerbotAI* botAI)
        {
            return new ActionNode ("lesser heal on party",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ nullptr,
                /*C*/ nullptr);
        }

        static ActionNode* flash_heal(PlayerbotAI* botAI)
        {
            return new ActionNode ("flash heal",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ NextAction::array(0, new NextAction("greater heal"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* flash_heal_on_party(PlayerbotAI* botAI)
        {
            return new ActionNode ("flash heal on party",
                /*P*/ NextAction::array(0, new NextAction("remove shadowform"), nullptr),
                /*A*/ NextAction::array(0, new NextAction("greater heal on party"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* psychic_scream(PlayerbotAI* botAI)
        {
            return new ActionNode ("psychic scream",
                /*P*/ nullptr,
                /*A*/ NextAction::array(0, new NextAction("fade"), nullptr),
                /*C*/ nullptr);
        }

        static ActionNode* fade(PlayerbotAI* botAI)
        {
            return new ActionNode ("fade",
                /*P*/ nullptr,
                /*A*/ nullptr,
                /*C*/ nullptr);
        }
};

#endif
