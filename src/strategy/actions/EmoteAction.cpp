/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "EmoteAction.h"
#include "Event.h"
#include "Playerbot.h"
#include "ServerFacade.h"

std::map<std::string, uint32> EmoteActionBase::emotes;
std::map<std::string, uint32> EmoteActionBase::textEmotes;
char* strstri(char const* haystack, char const* needle);

EmoteActionBase::EmoteActionBase(PlayerbotAI* botAI, std::string const& name) : Action(botAI, name)
{
    if (emotes.empty())
        InitEmotes();
}

EmoteAction::EmoteAction(PlayerbotAI* botAI) : EmoteActionBase(botAI, "emote"), Qualified()
{
}

void EmoteActionBase::InitEmotes()
{
    emotes["dance"] = EMOTE_ONESHOT_DANCE;
    emotes["drown"] = EMOTE_ONESHOT_DROWN;
    emotes["land"] = EMOTE_ONESHOT_LAND;
    emotes["liftoff"] = EMOTE_ONESHOT_LIFTOFF;
    emotes["loot"] = EMOTE_ONESHOT_LOOT;
    emotes["no"] = EMOTE_ONESHOT_NO;
    emotes["roar"] = EMOTE_STATE_ROAR;
    emotes["salute"] = EMOTE_ONESHOT_SALUTE;
    emotes["stomp"] = EMOTE_ONESHOT_STOMP;
    emotes["train"] = EMOTE_ONESHOT_TRAIN;
    emotes["yes"] = EMOTE_ONESHOT_YES;
    emotes["applaud"] = EMOTE_ONESHOT_APPLAUD;
    emotes["beg"] = EMOTE_ONESHOT_BEG;
    emotes["bow"] = EMOTE_ONESHOT_BOW;
    emotes["cheer"] = EMOTE_ONESHOT_CHEER;
    emotes["chicken"] = EMOTE_ONESHOT_CHICKEN;
    emotes["cry"] = EMOTE_ONESHOT_CRY;
    emotes["dance"] = EMOTE_STATE_DANCE;
    emotes["eat"] = EMOTE_ONESHOT_EAT;
    emotes["exclamation"] = EMOTE_ONESHOT_EXCLAMATION;
    emotes["flex"] = EMOTE_ONESHOT_FLEX;
    emotes["kick"] = EMOTE_ONESHOT_KICK;
    emotes["kiss"] = EMOTE_ONESHOT_KISS;
    emotes["kneel"] = EMOTE_ONESHOT_KNEEL;
    emotes["laugh"] = EMOTE_ONESHOT_LAUGH;
    emotes["point"] = EMOTE_ONESHOT_POINT;
    emotes["question"] = EMOTE_ONESHOT_QUESTION;
    emotes["ready1h"] = EMOTE_ONESHOT_READY1H;
    emotes["roar"] = EMOTE_ONESHOT_ROAR;
    emotes["rude"] = EMOTE_ONESHOT_RUDE;
    emotes["shout"] = EMOTE_ONESHOT_SHOUT;
    emotes["shy"] = EMOTE_ONESHOT_SHY;
    emotes["sleep"] = EMOTE_STATE_SLEEP;
    emotes["talk"] = EMOTE_ONESHOT_TALK;
    emotes["wave"] = EMOTE_ONESHOT_WAVE;
    emotes["wound"] = EMOTE_ONESHOT_WOUND;

    textEmotes["bored"] = TEXT_EMOTE_BORED;
    textEmotes["bye"] = TEXT_EMOTE_BYE;
    textEmotes["cheer"] = TEXT_EMOTE_CHEER;
    textEmotes["congratulate"] = TEXT_EMOTE_CONGRATULATE;
    textEmotes["hello"] = TEXT_EMOTE_HELLO;
    textEmotes["no"] = TEXT_EMOTE_NO;
    textEmotes["nod"] = TEXT_EMOTE_NOD; // yes
    textEmotes["sigh"] = TEXT_EMOTE_SIGH;
    textEmotes["thank"] = TEXT_EMOTE_THANK;
    textEmotes["welcome"] = TEXT_EMOTE_WELCOME; // you are welcome
    textEmotes["whistle"] = TEXT_EMOTE_WHISTLE;
    textEmotes["yawn"] = TEXT_EMOTE_YAWN;
    textEmotes["oom"] = 323;
    textEmotes["follow"] = 324;
    textEmotes["wait"] = 325;
    textEmotes["healme"] = 326;
    textEmotes["openfire"] = 327;
    textEmotes["helpme"] = 303;
    textEmotes["flee"] = 306;
    textEmotes["danger"] = 304;
    textEmotes["charge"] = 305;
    textEmotes["help"] = 307;
    textEmotes["train"] = 264;
}

bool EmoteActionBase::Emote(Unit* target, uint32 type)
{
    if (bot->isMoving())
        return false;

    if (target && !bot->HasInArc(EMOTE_ANGLE_IN_FRONT, target, sPlayerbotAIConfig->sightDistance))
        bot->SetFacingToObject(target);

    ObjectGuid oldSelection = bot->GetTarget();
    if (target)
    {
        bot->SetTarget(target->GetGUID());

        Player* player = dynamic_cast<Player*>(target);
        if (player && player->GetPlayerbotAI() && !player->HasInArc(EMOTE_ANGLE_IN_FRONT, bot, sPlayerbotAIConfig->sightDistance))
            player->SetFacingToObject(bot);
    }

    bot->HandleEmoteCommand(type);

    if (oldSelection)
        bot->SetTarget(oldSelection);

    return true;
}

Unit* EmoteActionBase::GetTarget()
{
    Unit* target = nullptr;

    GuidVector nfp = *context->GetValue<GuidVector>("nearest friendly players");
    std::vector<Unit*> targets;
    for (GuidVector::iterator i = nfp.begin(); i != nfp.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && sServerFacade->GetDistance2d(bot, unit) < sPlayerbotAIConfig->tooCloseDistance)
            targets.push_back(unit);
    }

    if (!targets.empty())
        target = targets[urand(0, targets.size() - 1)];

    return target;
}

bool EmoteActionBase::ReceiveEmote(Player* source, uint32 emote)
{
    uint32 emoteId = 0;
    std::string emoteText;
    std::string emoteYell;

    switch (emote)
    {
        case TEXT_EMOTE_BONK:
            emoteId = EMOTE_ONESHOT_CRY;
            break;
        case TEXT_EMOTE_SALUTE:
            emoteId = EMOTE_ONESHOT_SALUTE;
            break;
        case 325:
            if (botAI->GetMaster() == source)
            {
                botAI->ChangeStrategy("-follow,+stay", BOT_STATE_NON_COMBAT);
                botAI->TellMasterNoFacing("Fine.. I'll stay right here..");
            }
            break;
        case TEXT_EMOTE_BECKON:
        case 324:
            if (botAI->GetMaster() == source)
            {
                botAI->ChangeStrategy("+follow", BOT_STATE_NON_COMBAT);
                botAI->TellMasterNoFacing("Wherever you go, I'll follow..");
            }
            break;
        case TEXT_EMOTE_WAVE:
        case TEXT_EMOTE_GREET:
        case TEXT_EMOTE_HAIL:
        case TEXT_EMOTE_HELLO:
        case TEXT_EMOTE_WELCOME:
        case TEXT_EMOTE_INTRODUCE:
            emoteText = "Hey there!";
            emoteId = EMOTE_ONESHOT_WAVE;
            break;
        case TEXT_EMOTE_DANCE:
            emoteText = "Shake what your mama gave you!";
            emoteId = EMOTE_ONESHOT_DANCE;
            break;
        case TEXT_EMOTE_FLIRT:
        case TEXT_EMOTE_KISS:
        case TEXT_EMOTE_HUG:
        case TEXT_EMOTE_BLUSH:
        case TEXT_EMOTE_SMILE:
        case TEXT_EMOTE_LOVE:
            //case TEXT_EMOTE_HOLDHAND:
            emoteText = "Awwwww...";
            emoteId = EMOTE_ONESHOT_SHY;
            break;
        case TEXT_EMOTE_FLEX:
            emoteText = "Hercules! Hercules!";
            emoteId = EMOTE_ONESHOT_APPLAUD;
            break;
        case TEXT_EMOTE_ANGRY:
            //case TEXT_EMOTE_FACEPALM:
        case TEXT_EMOTE_GLARE:
        case TEXT_EMOTE_BLAME:
            //case TEXT_EMOTE_FAIL:
            //case TEXT_EMOTE_REGRET:
            //case TEXT_EMOTE_SCOLD:
            //case TEXT_EMOTE_CROSSARMS:
            emoteText = "Did I do thaaaaat?";
            emoteId = EMOTE_ONESHOT_QUESTION;
            break;
        case TEXT_EMOTE_FART:
        case TEXT_EMOTE_BURP:
        case TEXT_EMOTE_GASP:
        case TEXT_EMOTE_NOSEPICK:
        case TEXT_EMOTE_SNIFF:
        case TEXT_EMOTE_STINK:
            emoteText = "Wasn't me! Just sayin'..";
            emoteId = EMOTE_ONESHOT_POINT;
            break;
        case TEXT_EMOTE_JOKE:
            emoteId = EMOTE_ONESHOT_LAUGH;
            emoteText = "Oh.. was I not supposed to laugh so soon?";
            break;
        case TEXT_EMOTE_CHICKEN:
            emoteText = "We'll see who's chicken soon enough!";
            emoteId = EMOTE_ONESHOT_RUDE;
            break;
        case TEXT_EMOTE_APOLOGIZE:
            emoteId = EMOTE_ONESHOT_POINT;
            emoteText = "You damn right you're sorry!";
            break;
        case TEXT_EMOTE_APPLAUD:
        case TEXT_EMOTE_CLAP:
        case TEXT_EMOTE_CONGRATULATE:
        case TEXT_EMOTE_HAPPY:
            //case TEXT_EMOTE_GOLFCLAP:
            emoteId = EMOTE_ONESHOT_BOW;
            emoteText = "Thank you.. Thank you.. I'm here all week.";
            break;
        case TEXT_EMOTE_BEG:
        case TEXT_EMOTE_GROVEL:
        case TEXT_EMOTE_PLEAD:
            emoteId = EMOTE_ONESHOT_NO;
            emoteText = "Beg all you want.. I have nothing for you.";
            break;
        case TEXT_EMOTE_BITE:
        case TEXT_EMOTE_POKE:
        case TEXT_EMOTE_SCRATCH:
            //case TEXT_EMOTE_PINCH:
            //case TEXT_EMOTE_PUNCH:
            emoteId = EMOTE_ONESHOT_ROAR;
            emoteYell = "OUCH! Dammit, that hurt!";
            break;
        case TEXT_EMOTE_BORED:
            emoteId = EMOTE_ONESHOT_NO;
            emoteText = "My job description doesn't include entertaining you..";
            break;
        case TEXT_EMOTE_BOW:
        case TEXT_EMOTE_CURTSEY:
            emoteId = EMOTE_ONESHOT_BOW;
            break;
        case TEXT_EMOTE_BRB:
        case TEXT_EMOTE_SIT:
            emoteId = EMOTE_ONESHOT_EAT;
            emoteText = "Looks like time for an AFK break..";
            break;
        case TEXT_EMOTE_AGREE:
        case TEXT_EMOTE_NOD:
            emoteId = EMOTE_ONESHOT_EXCLAMATION;
            emoteText = "At least SOMEONE agrees with me!";
            break;
        case TEXT_EMOTE_AMAZE:
        case TEXT_EMOTE_COWER:
        case TEXT_EMOTE_CRINGE:
        case TEXT_EMOTE_EYE:
        case TEXT_EMOTE_KNEEL:
        case TEXT_EMOTE_PEER:
        case TEXT_EMOTE_SURRENDER:
        case TEXT_EMOTE_PRAISE:
        case TEXT_EMOTE_SCARED:
        case TEXT_EMOTE_COMMEND:
            //case TEXT_EMOTE_AWE:
            //case TEXT_EMOTE_JEALOUS:
            //case TEXT_EMOTE_PROUD:
            emoteId = EMOTE_ONESHOT_FLEX;
            emoteText = "Yes, Yes. I know I'm amazing..";
            break;
        case TEXT_EMOTE_BLEED:
        case TEXT_EMOTE_MOURN:
        case TEXT_EMOTE_FLOP:
            //case TEXT_EMOTE_FAINT:
            //case TEXT_EMOTE_PULSE:
            emoteId = EMOTE_ONESHOT_KNEEL;
            emoteText = "MEDIC! Stat!";
            break;
        case TEXT_EMOTE_BLINK:
            emoteId = EMOTE_ONESHOT_KICK;
            emoteText = "What? You got something in your eye?";
            break;
        case TEXT_EMOTE_BOUNCE:
        case TEXT_EMOTE_BARK:
            emoteId = EMOTE_ONESHOT_POINT;
            emoteText = "Who's a good doggy? You're a good doggy!";
            break;
        case TEXT_EMOTE_BYE:
            emoteId = EMOTE_ONESHOT_WAVE;
            emoteText = "Umm.... wait! Where are you going?!";
            break;
        case TEXT_EMOTE_CACKLE:
        case TEXT_EMOTE_LAUGH:
        case TEXT_EMOTE_CHUCKLE:
        case TEXT_EMOTE_GIGGLE:
        case TEXT_EMOTE_GUFFAW:
        case TEXT_EMOTE_ROFL:
        case TEXT_EMOTE_SNICKER:
            //case TEXT_EMOTE_SNORT:
            emoteId = EMOTE_ONESHOT_LAUGH;
            emoteText = "Wait... what are we laughing at again?";
            break;
        case TEXT_EMOTE_CONFUSED:
        case TEXT_EMOTE_CURIOUS:
        case TEXT_EMOTE_FIDGET:
        case TEXT_EMOTE_FROWN:
        case TEXT_EMOTE_SHRUG:
        case TEXT_EMOTE_SIGH:
        case TEXT_EMOTE_STARE:
        case TEXT_EMOTE_TAP:
        case TEXT_EMOTE_SURPRISED:
        case TEXT_EMOTE_WHINE:
        case TEXT_EMOTE_BOGGLE:
        case TEXT_EMOTE_LOST:
        case TEXT_EMOTE_PONDER:
        case TEXT_EMOTE_SNUB:
        case TEXT_EMOTE_SERIOUS:
        case TEXT_EMOTE_EYEBROW:
            emoteId = EMOTE_ONESHOT_QUESTION;
            emoteText = "Don't look at  me.. I just work here";
            break;
        case TEXT_EMOTE_COUGH:
        case TEXT_EMOTE_DROOL:
        case TEXT_EMOTE_SPIT:
        case TEXT_EMOTE_LICK:
        case TEXT_EMOTE_BREATH:
            //case TEXT_EMOTE_SNEEZE:
            //case TEXT_EMOTE_SWEAT:
            emoteId = EMOTE_ONESHOT_POINT;
            emoteText = "Ewww! Keep your nasty germs over there!";
            break;
        case TEXT_EMOTE_CRY:
            emoteId = EMOTE_ONESHOT_CRY;
            emoteText = "Don't you start crying or it'll make me start crying!";
            break;
        case TEXT_EMOTE_CRACK:
            emoteId = EMOTE_ONESHOT_ROAR;
            emoteText = "It's clobbering time!";
            break;
        case TEXT_EMOTE_EAT:
        case TEXT_EMOTE_DRINK:
            emoteId = EMOTE_ONESHOT_EAT;
            emoteText = "I hope you brought enough for the whole class...";
            break;
        case TEXT_EMOTE_GLOAT:
        case TEXT_EMOTE_MOCK:
        case TEXT_EMOTE_TEASE:
        case TEXT_EMOTE_EMBARRASS:
            emoteId = EMOTE_ONESHOT_CRY;
            emoteText = "Doesn't mean you need to be an ass about it..";
            break;
        case TEXT_EMOTE_HUNGRY:
            emoteId = EMOTE_ONESHOT_EAT;
            emoteText = "What? You want some of this?";
            break;
        case TEXT_EMOTE_LAYDOWN:
        case TEXT_EMOTE_TIRED:
        case TEXT_EMOTE_YAWN:
            emoteId = EMOTE_ONESHOT_KNEEL;
            emoteText = "Is it break time already?";
            break;
        case TEXT_EMOTE_MOAN:
        case TEXT_EMOTE_MOON:
        case TEXT_EMOTE_SEXY:
        case TEXT_EMOTE_SHAKE:
        case TEXT_EMOTE_WHISTLE:
        case TEXT_EMOTE_CUDDLE:
        case TEXT_EMOTE_PURR:
        case TEXT_EMOTE_SHIMMY:
        case TEXT_EMOTE_SMIRK:
        case TEXT_EMOTE_WINK:
            //case TEXT_EMOTE_CHARM:
            emoteId = EMOTE_ONESHOT_NO;
            emoteText = "Keep it in your pants, boss..";
            break;
        case TEXT_EMOTE_NO:
        case TEXT_EMOTE_VETO:
        case TEXT_EMOTE_DISAGREE:
        case TEXT_EMOTE_DOUBT:
            emoteId = EMOTE_ONESHOT_QUESTION;
            emoteText = "Aww.... why not?!";
            break;
        case TEXT_EMOTE_PANIC:
            emoteId = EMOTE_ONESHOT_EXCLAMATION;
            emoteText = "Now is NOT the time to panic!";
            break;
        case TEXT_EMOTE_POINT:
            emoteId = EMOTE_ONESHOT_POINT;
            emoteText = "What?! I can do that TOO!";
            break;
        case TEXT_EMOTE_RUDE:
        case TEXT_EMOTE_RASP:
            emoteId = EMOTE_ONESHOT_RUDE;
            emoteText = "Right back at you, bub!", LANG_UNIVERSAL;
            break;
        case TEXT_EMOTE_ROAR:
        case TEXT_EMOTE_THREATEN:
        case TEXT_EMOTE_CALM:
        case TEXT_EMOTE_DUCK:
        case TEXT_EMOTE_TAUNT:
        case TEXT_EMOTE_PITY:
        case TEXT_EMOTE_GROWL:
            //case TEXT_EMOTE_TRAIN:
            //case TEXT_EMOTE_INCOMING:
            //case TEXT_EMOTE_CHARGE:
            //case TEXT_EMOTE_FLEE:
            //case TEXT_EMOTE_ATTACKMYTARGET:
        case TEXT_EMOTE_OPENFIRE:
        case TEXT_EMOTE_ENCOURAGE:
        case TEXT_EMOTE_ENEMY:
            //case TEXT_EMOTE_CHALLENGE:
            //case TEXT_EMOTE_REVENGE:
            //case TEXT_EMOTE_SHAKEFIST:
            emoteId = EMOTE_ONESHOT_ROAR;
            emoteYell = "RAWR!";
            break;
        case TEXT_EMOTE_TALK:
        case TEXT_EMOTE_TALKEX:
        case TEXT_EMOTE_TALKQ:
        case TEXT_EMOTE_LISTEN:
            emoteId = EMOTE_ONESHOT_TALK;
            emoteText = "Blah Blah Blah Yakety Smackety..";
            break;
        case TEXT_EMOTE_THANK:
            emoteId = EMOTE_ONESHOT_BOW;
            emoteText = "You are quite welcome!";
            break;
        case TEXT_EMOTE_VICTORY:
        case TEXT_EMOTE_CHEER:
        case TEXT_EMOTE_TOAST:
            //case TEXT_EMOTE_HIGHFIVE:
            //case TEXT_EMOTE_DING:
            emoteId = EMOTE_ONESHOT_CHEER;
            emoteText = "Yay!";
            break;
        case TEXT_EMOTE_COLD:
        case TEXT_EMOTE_SHIVER:
        case TEXT_EMOTE_THIRSTY:
            //case TEXT_EMOTE_OOM:
            //case TEXT_EMOTE_HEALME:
            //case TEXT_EMOTE_POUT:
            emoteId = EMOTE_ONESHOT_QUESTION;
            emoteText = "And what exactly am I supposed to do about that?";
            break;
        case TEXT_EMOTE_COMFORT:
        case TEXT_EMOTE_SOOTHE:
        case TEXT_EMOTE_PAT:
            emoteId = EMOTE_ONESHOT_CRY;
            emoteText = "Thanks...";
            break;
        case TEXT_EMOTE_INSULT:
            emoteId = EMOTE_ONESHOT_CRY;
            emoteText = "You hurt my feelings..";
            break;
        case TEXT_EMOTE_JK:
            emoteId = EMOTE_ONESHOT_POINT;
            emoteText = "You.....";
            break;
        case TEXT_EMOTE_RAISE:
            emoteId = EMOTE_ONESHOT_POINT;
            emoteText = "Yes.. you.. at the back of the class..";
            break;
        case TEXT_EMOTE_READY:
            emoteId = EMOTE_ONESHOT_SALUTE;
            emoteText = "Ready here, too!";
            break;
        case TEXT_EMOTE_SHOO:
            emoteId = EMOTE_ONESHOT_KICK;
            emoteText = "Shoo yourself!";
            break;
        case TEXT_EMOTE_SLAP:
            //case TEXT_EMOTE_SMACK:
            emoteId = EMOTE_ONESHOT_CRY;
            emoteText = "What did I do to deserve that?";
            break;
        case TEXT_EMOTE_STAND:
            emoteId = EMOTE_ONESHOT_NONE;
            emoteText = "What? Break time's over? Fine..";
            break;
        case TEXT_EMOTE_TICKLE:
            emoteId = EMOTE_ONESHOT_LAUGH;
            emoteText = "Hey! Stop that!";
            break;
        case TEXT_EMOTE_VIOLIN:
            emoteId = EMOTE_ONESHOT_TALK;
            emoteText = "Har Har.. very funny..";
            break;
            //case TEXT_EMOTE_HELPME:
            //    bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
            //    bot->Yell("Quick! Someone HELP!", LANG_UNIVERSAL);
            //    break;
        case TEXT_EMOTE_GOODLUCK:
            //case TEXT_EMOTE_LUCK:
            emoteId = EMOTE_ONESHOT_TALK;
            emoteText = "Thanks... I'll need it..";
            break;
        case TEXT_EMOTE_BRANDISH:
            //case TEXT_EMOTE_MERCY:
            emoteId = EMOTE_ONESHOT_BEG;
            emoteText = "Please don't kill me!";
            break;
        /*case TEXT_EMOTE_BADFEELING:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);
            bot->Say("I'm just waiting for the ominous music now...", LANG_UNIVERSAL);
            break;
        case TEXT_EMOTE_MAP:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_NO);
            bot->Say("Noooooooo.. you just couldn't ask for directions, huh?", LANG_UNIVERSAL);
            break;
        case TEXT_EMOTE_IDEA:
        case TEXT_EMOTE_THINK:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_NO);
            bot->Say("Oh boy.. another genius idea...", LANG_UNIVERSAL);
            break;
        case TEXT_EMOTE_OFFER:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_NO);
            bot->Say("No thanks.. I had some back at the last village", LANG_UNIVERSAL);
            break;
        case TEXT_EMOTE_PET:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
            bot->Say("Do I look like a dog to you?!", LANG_UNIVERSAL);
            break;
        case TEXT_EMOTE_ROLLEYES:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
            bot->Say("Keep doing that and I'll roll those eyes right out of your head..", LANG_UNIVERSAL);
            break;
        case TEXT_EMOTE_SING:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_APPLAUD);
            bot->Say("Lovely... just lovely..", LANG_UNIVERSAL);
            break;
        case TEXT_EMOTE_COVEREARS:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
            bot->Yell("You think that's going to help you?!", LANG_UNIVERSAL);
            break;*/
        default:
            //return false;
            //bot->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);
            //bot->Say("Mmmmmkaaaaaay...", LANG_UNIVERSAL);
            break;
    }

    if (source && !bot->isMoving() && !bot->HasInArc(EMOTE_ANGLE_IN_FRONT, source, sPlayerbotAIConfig->sightDistance))
        sServerFacade->SetFacingTo(bot, source);

    if (emoteText.size())
        bot->Say(emoteText, (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));

    if (emoteYell.size())
        bot->Yell(emoteYell, (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));

    if (emoteId)
        bot->HandleEmoteCommand(emoteId);

    return true;
}

bool EmoteAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    uint32 emote = 0;

    Player* pSource = nullptr;
    bool isReact = false;
    if (!p.empty() && p.GetOpcode() == SMSG_TEXT_EMOTE)
    {
        isReact = true;
        ObjectGuid source;
        uint32 text_emote;
        uint32 emote_num;
        uint32 namlen;
        std::string nam;
        p.rpos(0);
        p >> source >> text_emote >> emote_num >> namlen;
        if (namlen > 1)
            p >> nam;

        if (strstri(bot->GetName().c_str(), nam.c_str()))
        {
            pSource = ObjectAccessor::FindPlayer(source);

            if (pSource && sServerFacade->GetDistance2d(bot, pSource) < sPlayerbotAIConfig->farDistance)
            {
                LOG_INFO("playerbots", "Bot %s %s:%d <%s> received SMSG_TEXT_EMOTE %d",
                    bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str(), text_emote);
                emote = text_emote;
            }

        }
    }

    if (!p.empty() && p.GetOpcode() == SMSG_EMOTE)
    {
        isReact = true;
        ObjectGuid source;
        uint32 emoteId;
        p.rpos(0);
        p >> emoteId >> source;

        pSource = ObjectAccessor::FindPlayer(source);
        if (pSource && sServerFacade->GetDistance2d(bot, pSource) < sPlayerbotAIConfig->farDistance && emoteId != EMOTE_ONESHOT_NONE)
        {
            if (pSource->GetTarget() == bot->GetGUID())
            {
                LOG_INFO("playerbots", "Bot %s %s:%d <%s> received SMSG_EMOTE %d",
                    bot->GetGUID().ToString().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName().c_str(), emoteId);

                std::vector<uint32> types;
                for (int32 i = sEmotesTextStore.GetNumRows(); i >= 0; --i)
                {
                    EmotesTextEntry const* em = sEmotesTextStore.LookupEntry(uint32(i));
                    if (!em)
                        continue;

                    if (em->textid == EMOTE_ONESHOT_TALK)
                        continue;

                    if (em->textid == EMOTE_ONESHOT_QUESTION)
                        continue;

                    if (em->textid == EMOTE_ONESHOT_EXCLAMATION)
                        continue;

                    if (em->textid == emoteId)
                    {
                        types.push_back(em->Id);
                    }
                }

                if (types.size())
                    emote = types[urand(0, types.size() - 1)];
            }
        }
    }

    if (isReact && !emote)
        return false;

    std::string param = event.getParam();
    if ((!isReact && param.empty()) || emote)
    {
        time_t lastEmote = AI_VALUE2(time_t, "last emote", qualifier);
        botAI->GetAiObjectContext()->GetValue<time_t>("last emote", qualifier)->Set(time(nullptr) + urand(1000, sPlayerbotAIConfig->repeatDelay) / 1000);
        param = qualifier;
    }

    if (emote)
        return ReceiveEmote(pSource, emote);

    if (param.find("sound") == 0)
    {
        return botAI->PlaySound(atoi(param.substr(5).c_str()));
    }

    if (!param.empty() && textEmotes.find(param) != textEmotes.end())
    {
        return botAI->PlaySound(textEmotes[param]);
    }

    if (param.empty() || emotes.find(param) == emotes.end())
    {
        uint32 index = rand() % emotes.size();
        for (std::map<std::string, uint32>::iterator i = emotes.begin(); i != emotes.end() && index; ++i, --index)
            emote = i->second;
    }
    else
    {
        emote = emotes[param];
    }

    if (param.find("text") == 0)
    {
        emote = atoi(param.substr(4).c_str());
    }

    return Emote(GetTarget(), emote);
}

bool EmoteAction::isUseful()
{
    if (!botAI->HasPlayerNearby())
        return false;

    if (bot->isMoving())
        return false;

    time_t lastEmote = AI_VALUE2(time_t, "last emote", qualifier);
    return (time(nullptr) - lastEmote) >= sPlayerbotAIConfig->repeatDelay / 1000;
}

bool TalkAction::Execute(Event event)
{
    Unit* target = botAI->GetUnit(AI_VALUE(ObjectGuid, "talk target"));
    if (!target)
        target = GetTarget();

    if (!urand(0, 100))
    {
        target = nullptr;
        context->GetValue<ObjectGuid>("talk target")->Set(ObjectGuid::Empty);
        return true;
    }

    if (target)
    {
        Player* player = dynamic_cast<Player*>(target);
        if (player && player->GetPlayerbotAI())
            player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<ObjectGuid>("talk target")->Set(bot->GetGUID());

        context->GetValue<ObjectGuid>("talk target")->Set(target->GetGUID());
        return Emote(target, GetRandomEmote(target));
    }

    return false;
}

uint32 TalkAction::GetRandomEmote(Unit* unit)
{
    std::vector<uint32> types;
    if (!urand(0, 20))
    {
        // expressions
        types.push_back(EMOTE_ONESHOT_BOW);
        types.push_back(EMOTE_ONESHOT_RUDE);
        types.push_back(EMOTE_ONESHOT_CRY);
        types.push_back(EMOTE_ONESHOT_LAUGH);
        types.push_back(EMOTE_ONESHOT_POINT);
        types.push_back(EMOTE_ONESHOT_CHEER);
        types.push_back(EMOTE_ONESHOT_SHY);
    }
    else
    {
        // talk
        types.push_back(EMOTE_ONESHOT_TALK);
        types.push_back(EMOTE_ONESHOT_EXCLAMATION);
        types.push_back(EMOTE_ONESHOT_QUESTION);

        if (unit && (unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER) || unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER)))
        {
            types.push_back(EMOTE_ONESHOT_SALUTE);
        }
    }

    return types[urand(0, types.size() - 1)];
}
