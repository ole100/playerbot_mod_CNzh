/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChatHelper.h"

#include "AiFactory.h"
#include "Common.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "SpellInfo.h"

#include <regex>

std::map<std::string, uint32> ChatHelper::consumableSubClasses;
std::map<std::string, uint32> ChatHelper::tradeSubClasses;
std::map<std::string, uint32> ChatHelper::itemQualities;
std::map<std::string, uint32> ChatHelper::projectileSubClasses;
std::map<std::string, uint32> ChatHelper::slots;
std::map<std::string, uint32> ChatHelper::skills;
std::map<std::string, ChatMsg> ChatHelper::chats;
std::map<uint8, std::string> ChatHelper::classes;
std::map<uint8, std::string> ChatHelper::races;
std::map<uint8, std::map<uint8, std::string> > ChatHelper::specs;

template <class T>
static bool substrContainsInMap(std::string const searchTerm, std::map<std::string, T> searchIn)
{
    for (typename std::map<std::string, T>::iterator i = searchIn.begin(); i != searchIn.end(); ++i)
    {
        std::string const term = i->first;
        if (term.size() > 1 && searchTerm.find(term) != std::string::npos)
            return true;
    }

    return false;
}

ChatHelper::ChatHelper(PlayerbotAI* botAI) : PlayerbotAIAware(botAI)
{
    itemQualities["破烂"] = ITEM_QUALITY_POOR;
    itemQualities["垃圾"] = ITEM_QUALITY_POOR;
    itemQualities["正常"] = ITEM_QUALITY_NORMAL;
    itemQualities["白板"] = ITEM_QUALITY_NORMAL;
    itemQualities["稀有"] = ITEM_QUALITY_UNCOMMON;
    itemQualities["绿色"] = ITEM_QUALITY_UNCOMMON;
    itemQualities["稀有"] = ITEM_QUALITY_RARE;
    itemQualities["蓝色"] = ITEM_QUALITY_RARE;
    itemQualities["精良"] = ITEM_QUALITY_EPIC;
    itemQualities["紫装"] = ITEM_QUALITY_EPIC;
    itemQualities["史诗"] = ITEM_QUALITY_LEGENDARY;
    itemQualities["橙装"] = ITEM_QUALITY_LEGENDARY;

    consumableSubClasses["药水"] = ITEM_SUBCLASS_POTION;
    consumableSubClasses["合剂"] = ITEM_SUBCLASS_ELIXIR;
    consumableSubClasses["瓶剂"] = ITEM_SUBCLASS_FLASK;
    consumableSubClasses["卷轴"] = ITEM_SUBCLASS_SCROLL;
    consumableSubClasses["食物"] = ITEM_SUBCLASS_FOOD;
    consumableSubClasses["绷带"] = ITEM_SUBCLASS_BANDAGE;
    consumableSubClasses["附魔"] = ITEM_SUBCLASS_CONSUMABLE_OTHER;

    projectileSubClasses["箭"] = ITEM_SUBCLASS_ARROW;
    projectileSubClasses["子弹"] = ITEM_SUBCLASS_BULLET;

    // tradeSubClasses["cloth"] = ITEM_SUBCLASS_CLOTH;
    // tradeSubClasses["leather"] = ITEM_SUBCLASS_LEATHER;
    // tradeSubClasses["metal"] = ITEM_SUBCLASS_METAL_STONE;
    // tradeSubClasses["stone"] = ITEM_SUBCLASS_METAL_STONE;
    // tradeSubClasses["ore"] = ITEM_SUBCLASS_METAL_STONE;
    // tradeSubClasses["meat"] = ITEM_SUBCLASS_MEAT;
    // tradeSubClasses["herb"] = ITEM_SUBCLASS_HERB;
    // tradeSubClasses["elemental"] = ITEM_SUBCLASS_ELEMENTAL;
    // tradeSubClasses["disenchants"] = ITEM_SUBCLASS_ENCHANTING;
    // tradeSubClasses["enchanting"] = ITEM_SUBCLASS_ENCHANTING;
    // tradeSubClasses["gems"] = ITEM_SUBCLASS_JEWELCRAFTING;
    // tradeSubClasses["jewels"] = ITEM_SUBCLASS_JEWELCRAFTING;
    // tradeSubClasses["jewelcrafting"] = ITEM_SUBCLASS_JEWELCRAFTING;

    slots["头部"] = EQUIPMENT_SLOT_HEAD;
    slots["颈部"] = EQUIPMENT_SLOT_NECK;
    slots["肩膀"] = EQUIPMENT_SLOT_SHOULDERS;
    slots["衬衣"] = EQUIPMENT_SLOT_BODY;
    slots["胸甲"] = EQUIPMENT_SLOT_CHEST;
    slots["腰部"] = EQUIPMENT_SLOT_WAIST;
    slots["腿部"] = EQUIPMENT_SLOT_LEGS;
    slots["脚"] = EQUIPMENT_SLOT_FEET;
    slots["手腕"] = EQUIPMENT_SLOT_WRISTS;
    slots["护手"] = EQUIPMENT_SLOT_HANDS;
    slots["戒指1"] = EQUIPMENT_SLOT_FINGER1;
    slots["戒指2"] = EQUIPMENT_SLOT_FINGER2;
    slots["饰品1"] = EQUIPMENT_SLOT_TRINKET1;
    slots["饰品2"] = EQUIPMENT_SLOT_TRINKET2;
    slots["背部"] = EQUIPMENT_SLOT_BACK;
    slots["主手"] = EQUIPMENT_SLOT_MAINHAND;
    slots["副手"] = EQUIPMENT_SLOT_OFFHAND;
    slots["远程武器栏位"] = EQUIPMENT_SLOT_RANGED;
    slots["圣物"] = EQUIPMENT_SLOT_TABARD;

    skills["急救"] = SKILL_FIRST_AID;
    skills["钓鱼"] = SKILL_FISHING;
    skills["烹饪"] = SKILL_COOKING;
    skills["炼金术"] = SKILL_ALCHEMY;
    skills["附魔"] = SKILL_ENCHANTING;
    skills["工程"] = SKILL_ENGINEERING;
    skills["制皮"] = SKILL_LEATHERWORKING;
    skills["锻造"] = SKILL_BLACKSMITHING;
    skills["裁缝"] = SKILL_TAILORING;
    skills["草药学"] = SKILL_HERBALISM;
    skills["采矿"] = SKILL_MINING;
    skills["剥皮"] = SKILL_SKINNING;
    skills["珠宝加工"] = SKILL_JEWELCRAFTING;

    chats["队伍"] = CHAT_MSG_PARTY;
    chats["队伍"] = CHAT_MSG_PARTY;
    chats["工会"] = CHAT_MSG_GUILD;
    chats["工会"] = CHAT_MSG_GUILD;
    chats["副本"] = CHAT_MSG_RAID;
    chats["副本"] = CHAT_MSG_RAID;
    chats["私聊"] = CHAT_MSG_WHISPER;
    chats["私聊"] = CHAT_MSG_WHISPER;

    classes[CLASS_DRUID] = "德鲁伊";
    specs[CLASS_DRUID][0] = "鸟德";
    specs[CLASS_DRUID][1] = "野德";
    specs[CLASS_DRUID][2] = "树德";

    classes[CLASS_HUNTER] = "猎人";
    specs[CLASS_HUNTER][0] = "兽王猎";
    specs[CLASS_HUNTER][1] = "射击猎";
    specs[CLASS_HUNTER][2] = "生存猎";

    classes[CLASS_MAGE] = "法师";
    specs[CLASS_MAGE][0] = "奥法";
    specs[CLASS_MAGE][1] = "火法";
    specs[CLASS_MAGE][2] = "冰法";

    classes[CLASS_PALADIN] = "圣骑";
    specs[CLASS_PALADIN][0] = "奶骑";
    specs[CLASS_PALADIN][1] = "防骑";
    specs[CLASS_PALADIN][2] = "惩戒骑";

    classes[CLASS_PRIEST] = "牧师";
    specs[CLASS_PRIEST][0] = "戒律牧";
    specs[CLASS_PRIEST][1] = "神牧";
    specs[CLASS_PRIEST][2] = "暗牧";

    classes[CLASS_ROGUE] = "盗贼";
    specs[CLASS_ROGUE][0] = "刺杀贼";
    specs[CLASS_ROGUE][1] = "战斗贼";
    specs[CLASS_ROGUE][2] = "敏锐贼";

    classes[CLASS_SHAMAN] = "萨满";
    specs[CLASS_SHAMAN][0] = "元素萨";
    specs[CLASS_SHAMAN][1] = "增强萨";
    specs[CLASS_SHAMAN][2] = "奶萨";

    classes[CLASS_WARLOCK] = "术士";
    specs[CLASS_WARLOCK][0] = "痛苦术";
    specs[CLASS_WARLOCK][1] = "恶魔术";
    specs[CLASS_WARLOCK][2] = "毁灭术";

    classes[CLASS_WARRIOR] = "战士";
    specs[CLASS_WARRIOR][0] = "武器战";
    specs[CLASS_WARRIOR][1] = "狂暴战";
    specs[CLASS_WARRIOR][2] = "防战";

    classes[CLASS_DEATH_KNIGHT] = "死骑";
    specs[CLASS_DEATH_KNIGHT][0] = "血dk";
    specs[CLASS_DEATH_KNIGHT][1] = "冰霜dk";
    specs[CLASS_DEATH_KNIGHT][2] = "邪DK";

    races[RACE_DWARF] = "侏儒";
    races[RACE_GNOME] = "侏儒";
    races[RACE_HUMAN] = "人类";
    races[RACE_NIGHTELF] = "暗夜精灵";
    races[RACE_ORC] = "兽人";
    races[RACE_TAUREN] = "牛头人";
    races[RACE_TROLL] = "巨魔";
    races[RACE_UNDEAD_PLAYER] = "亡灵";
    races[RACE_BLOODELF] = "血精灵";
    races[RACE_DRAENEI] = "德莱尼";
}

std::string const ChatHelper::formatMoney(uint32 copper)
{
    std::ostringstream out;
    if (!copper)
    {
        out << "0";
        return out.str();
    }

    uint32 gold = uint32(copper / 10000);
    copper -= (gold * 10000);
    uint32 silver = uint32(copper / 100);
    copper -= (silver * 100);

    bool space = false;
    if (gold > 0)
    {
        out << gold << "金";
        space = true;
    }

    if (silver > 0 && gold < 50)
    {
        if (space)
            out << " ";

        out << silver << "银";
        space = true;
    }

    if (copper > 0 && gold < 10)
    {
        if (space)
            out << " ";

        out << copper << "铜";
    }

    return out.str();
}

std::string ChatHelper::parseValue(const std::string& type, const std::string& text)
{
    std::string retString;

    std::string pattern = "Hvalue:" + type + ":";

    int pos = text.find(pattern, 0);
    if (pos == -1)
        return retString;

    pos += pattern.size();

    int endPos = text.find('|', pos);
    if (endPos == -1)
        return retString;

    retString = text.substr(pos, endPos - pos);
    return retString;
}

uint32 ChatHelper::parseMoney(std::string const text)
{
    // if user specified money in ##g##s##c format
    std::string acum = "";
    uint32 copper = 0;
    for (uint8 i = 0; i < text.length(); i++)
    {
        if (text[i] == 'g')
        {
            copper += (atol(acum.c_str()) * 100 * 100);
            acum = "";
        }
        else if (text[i] == 's')
        {
            copper += atol(acum.c_str());
            acum = "";
        }
        else if (text[i] == 'c')
        {
            copper += (atol(acum.c_str()) * 100);
            acum = "";
        }
        else if (text[i] == ' ')
            break;
        else if (text[i] >= 48 && text[i] <= 57)
            acum += text[i];
        else
        {
            copper = 0;
            break;
        }
    }
    return copper;
}

ItemIds ChatHelper::parseItems(std::string const text)
{
    ItemIds itemIds;

    uint8 pos = 0;
    while (true)
    {
        auto i = text.find("Hitem:", pos);
        if (i == std::string::npos)
            break;

        pos = i + 6;
        auto endPos = text.find(':', pos);
        if (endPos == std::string::npos)
            break;

        std::string const idC = text.substr(pos, endPos - pos);
        auto id = atol(idC.c_str());
        pos = endPos;
        if (id)
            itemIds.insert(id);
    }

    return itemIds;
}

std::string const ChatHelper::FormatQuest(Quest const* quest)
{
    if (!quest)
    {
        return "无效任务";
    }

    std::ostringstream out;
    QuestLocale const* locale = sObjectMgr->GetQuestLocale(quest->GetQuestId());
    std::string questTitle;

    if (locale && locale->Title.size() > sWorld->GetDefaultDbcLocale())
        questTitle = locale->Title[sWorld->GetDefaultDbcLocale()];

    if (questTitle.empty())
        questTitle = quest->GetTitle();
    
    out << "|cFFFFFF00|Hquest:" << quest->GetQuestId() << ':' << quest->GetQuestLevel() << "|h[" << questTitle << "]|h|r";
    return out.str();
}

std::string const ChatHelper::FormatGameobject(GameObject* go)
{
    std::ostringstream out;
    out << "|cFFFFFF00|Hfound:" << go->GetGUID().GetRawValue() << ":" << go->GetEntry() << ":"
        << "|h[" << go->GetNameForLocaleIdx(sWorld->GetDefaultDbcLocale()) << "]|h|r";
    return out.str();
}

std::string const ChatHelper::FormatWorldobject(WorldObject* wo)
{
    std::ostringstream out;
    out << "|cFFFFFF00|Hfound:" << wo->GetGUID().GetRawValue() << ":" << wo->GetEntry() << ":"
        << "|h[";
    out << (wo->ToGameObject() ? ((GameObject*)wo)->GetNameForLocaleIdx(sWorld->GetDefaultDbcLocale())
                               : wo->GetNameForLocaleIdx(sWorld->GetDefaultDbcLocale()))
        << "]|h|r";
    return out.str();
}

std::string const ChatHelper::FormatWorldEntry(int32 entry)
{
    CreatureTemplate const* cInfo = nullptr;
    GameObjectTemplate const* gInfo = nullptr;

    if (entry > 0)
        cInfo = sObjectMgr->GetCreatureTemplate(entry);
    else
        gInfo = sObjectMgr->GetGameObjectTemplate(entry * -1);

    std::ostringstream out;
    out << "|cFFFFFF00|Hentry:" << abs(entry) << ":"
        << "|h[";

    if (entry < 0 && gInfo)
        out << gInfo->name;
    else if (entry > 0 && cInfo)
        out << cInfo->Name;
    else
        out << "unknown";

    out << "]|h|r";
    return out.str();
}

std::string const ChatHelper::FormatSpell(SpellInfo const* spellInfo)
{
    std::ostringstream out;
    std::string spellName = spellInfo->SpellName[sWorld->GetDefaultDbcLocale()] ?
        spellInfo->SpellName[sWorld->GetDefaultDbcLocale()] : spellInfo->SpellName[LOCALE_enUS];
    out << "|cffffffff|Hspell:" << spellInfo->Id << "|h[" << spellName << "]|h|r";
    return out.str();
}

std::string const ChatHelper::FormatItem(ItemTemplate const* proto, uint32 count, uint32 total)
{
    char color[32];
    sprintf(color, "%x", ItemQualityColors[proto->Quality]);

    std::string itemName;
    const ItemLocale* locale = sObjectMgr->GetItemLocale(proto->ItemId);

    if (locale && locale->Name.size() > sWorld->GetDefaultDbcLocale())
        itemName = locale->Name[sWorld->GetDefaultDbcLocale()];
    
    if (itemName.empty())
        itemName = proto->Name1;

    std::ostringstream out;
    out << "|c" << color << "|Hitem:" << proto->ItemId << ":0:0:0:0:0:0:0"
        << "|h[" << itemName << "]|h|r";

    if (count > 1)
        out << "x" << count;

    if (total > 0)
        out << " (" << total << ")";

    return out.str();
}

std::string const ChatHelper::FormatQItem(uint32 itemId)
{
    char color[32];
    sprintf(color, "%x", ItemQualityColors[0]);

    std::ostringstream out;
    out << "|c" << color << "|Hitem:" << itemId << ":0:0:0:0:0:0:0"
        << "|h[item"
        << "]|h|r";

    return out.str();
}

ChatMsg ChatHelper::parseChat(std::string const text)
{
    if (chats.find(text) != chats.end())
        return chats[text];

    return CHAT_MSG_SYSTEM;
}

std::string const ChatHelper::FormatChat(ChatMsg chat)
{
    switch (chat)
    {
        case CHAT_MSG_GUILD:
            return "guild";
        case CHAT_MSG_PARTY:
            return "party";
        case CHAT_MSG_WHISPER:
            return "whisper";
        case CHAT_MSG_RAID:
            return "raid";
        default:
            break;
    }

    return "unknown";
}

uint32 ChatHelper::parseSpell(std::string const text)
{
    PlayerbotChatHandler handler(botAI->GetBot());
    return handler.extractSpellId(text);
}

GuidVector ChatHelper::parseGameobjects(std::string const text)
{
    GuidVector gos;
    //    Link format
    //    |cFFFFFF00|Hfound:" << guid << ':'  << entry << ':'  <<  "|h[" << gInfo->name << "]|h|r";
    //    |cFFFFFF00|Hfound:9582:1731|h[Copper Vein]|h|r

    uint8 pos = 0;
    while (true)
    {
        // extract GO guid
        auto i = text.find("Hfound:", pos);  // base H = 11
        if (i == std::string::npos)          // break if error
            break;

        pos = i + 7;                        // start of window in text 11 + 7 = 18
        auto endPos = text.find(':', pos);  // end of window in text 22
        if (endPos == std::string::npos)    // break if error
            break;

        std::istringstream stream(text.substr(pos, endPos - pos));
        uint64 guid;
        stream >> guid;

        // extract GO entry
        pos = endPos + 1;
        endPos = text.find(':', pos);     // end of window in text
        if (endPos == std::string::npos)  // break if error
            break;

        std::string const entryC = text.substr(pos, endPos - pos);  // get std::string const within window i.e entry
        //uint32 entry = atol(entryC.c_str());                        // convert ascii to float

        ObjectGuid lootCurrent = ObjectGuid(guid);

        if (guid)
            gos.push_back(lootCurrent);
    }

    return gos;
}

std::string const ChatHelper::FormatQuestObjective(std::string const name, uint32 available, uint32 required)
{
    std::ostringstream out;
    out << "|cFFFFFFFF" << name << (available >= required ? "|c0000FF00: " : "|c00FF0000: ") << available << "/"
        << required << "|r";

    return out.str();
}

uint32 ChatHelper::parseItemQuality(std::string const text)
{
    if (itemQualities.find(text) == itemQualities.end())
        return MAX_ITEM_QUALITY;

    return itemQualities[text];
}

bool ChatHelper::parseItemClass(std::string const text, uint32* itemClass, uint32* itemSubClass)
{
    if (text == "questitem")
    {
        *itemClass = ITEM_CLASS_QUEST;
        *itemSubClass = ITEM_SUBCLASS_QUEST;
        return true;
    }

    if (consumableSubClasses.find(text) != consumableSubClasses.end())
    {
        *itemClass = ITEM_CLASS_CONSUMABLE;
        *itemSubClass = consumableSubClasses[text];
        return true;
    }

    if (tradeSubClasses.find(text) != tradeSubClasses.end())
    {
        *itemClass = ITEM_CLASS_TRADE_GOODS;
        *itemSubClass = tradeSubClasses[text];
        return true;
    }

    if (projectileSubClasses.find(text) != projectileSubClasses.end())
    {
        *itemClass = ITEM_CLASS_PROJECTILE;
        *itemSubClass = projectileSubClasses[text];
        return true;
    }

    return false;
}

uint32 ChatHelper::parseSlot(std::string const text)
{
    if (slots.find(text) != slots.end())
        return slots[text];

    return EQUIPMENT_SLOT_END;
}

bool ChatHelper::parseable(std::string const text)
{
    return text.find("|H") != std::string::npos || text == "questitem" || text == "ammo" ||
           substrContainsInMap<uint32>(text, consumableSubClasses) ||
           substrContainsInMap<uint32>(text, tradeSubClasses) || substrContainsInMap<uint32>(text, itemQualities) ||
           substrContainsInMap<uint32>(text, slots) || substrContainsInMap<ChatMsg>(text, chats) ||
           substrContainsInMap<uint32>(text, skills) || parseMoney(text) > 0;
}

std::string const ChatHelper::FormatClass(Player* player, int8 spec)
{
    uint8 cls = player->getClass();

    std::ostringstream out;
    out << specs[cls][spec] << " (";

    std::map<uint8, uint32> tabs = AiFactory::GetPlayerSpecTabs(player);
    uint32 c0 = tabs[0];
    uint32 c1 = tabs[1];
    uint32 c2 = tabs[2];

    out << (c0 ? "|h|cff00ff00" : "") << c0 << "|h|cffffffff/";
    out << (c1 ? "|h|cff00ff00" : "") << c1 << "|h|cffffffff/";
    out << (c2 ? "|h|cff00ff00" : "") << c2 << "|h|cffffffff";

    out << ")|r " << classes[cls];
    return out.str();
}

std::string const ChatHelper::FormatClass(uint8 cls) { return classes[cls]; }

std::string const ChatHelper::FormatRace(uint8 race) { return races[race]; }

uint32 ChatHelper::parseSkill(std::string const text)
{
    if (skills.find(text) != skills.end())
        return skills[text];

    return SKILL_NONE;
}

std::string const ChatHelper::FormatSkill(uint32 skill)
{
    for (std::map<std::string, uint32>::iterator i = skills.begin(); i != skills.end(); ++i)
    {
        if (i->second == skill)
            return i->first;
    }

    return "";
}

std::string const ChatHelper::FormatBoolean(bool flag) { return flag ? "|cff00ff00ON|r" : "|cffffff00OFF|r"; }

void ChatHelper::eraseAllSubStr(std::string& mainStr, std::string const toErase)
{
    size_t pos = std::string::npos;

    // Search for the substring in std::string const in a loop untill nothing is found
    while ((pos = mainStr.find(toErase)) != std::string::npos)
    {
        // If found then erase it from std::string
        mainStr.erase(pos, toErase.length());
    }
}

std::set<uint32> extractGeneric(std::string_view text, std::string_view prefix)
{
    std::set<uint32_t> ids;
    std::string_view text_view = text;

    size_t pos = 0;
    while ((pos = text_view.find(prefix, pos)) != std::string::npos)
    {
        // skip "Hquest:/Hitem:"
        pos += prefix.size();

        // extract everything after "Hquest:/Hitem:"
        size_t end_pos = text_view.find_first_not_of("0123456789", pos);
        std::string_view number_str = text_view.substr(pos, end_pos - pos);

        uint32 number = 0;
        
        auto [ptr, ec] = std::from_chars(number_str.data(), number_str.data() + number_str.size(), number);

        if (ec == std::errc())
        {
            ids.insert(number);
        }
        pos = end_pos;
    }

    return ids;
}

std::set<uint32> ChatHelper::ExtractAllQuestIds(const std::string& text)
{
    return extractGeneric(text, "Hquest:");
}

std::set<uint32> ChatHelper::ExtractAllItemIds(const std::string& text)
{
    return extractGeneric(text, "Hitem:");
}
