#include "QuestConfirmAcceptAction.h"

#include "WorldPacket.h"

bool QuestConfirmAcceptAction::Execute(Event event)
{
    WorldPacket packet(event.getPacket());
    uint32 questId;
    packet >> questId;

    WorldPacket sendPacket(CMSG_QUEST_CONFIRM_ACCEPT);
    sendPacket << questId;
    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest || !bot->CanAddQuest(quest, true))
    {
        return false;
    }
    std::ostringstream out;
    out << "任务: " << chat->FormatQuest(quest) << " 确认接受";
    botAI->TellMaster(out);
    bot->GetSession()->HandleQuestConfirmAccept(sendPacket);
    return true;
}