/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AttackAction.h"

#include "CreatureAI.h"
#include "Event.h"
#include "LastMovementValue.h"
#include "LootObjectStack.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "Unit.h"

bool AttackAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!target->IsInWorld())
    {
        return false;
    }
    return Attack(target);
}

bool AttackMyTargetAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ObjectGuid guid = master->GetTarget();
    if (!guid)
    {
        if (verbose)
            botAI->TellError("You have no target");

        return false;
    }

    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({guid});
    bool result = Attack(botAI->GetUnit(guid));
    if (result)
        context->GetValue<ObjectGuid>("pull target")->Set(guid);

    return result;
}

bool AttackAction::Attack(Unit* target, bool with_pet /*true*/)
{
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE ||
        bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
    {
        if (verbose)
            botAI->TellError("我不能在飞行中攻击");

        return false;
    }

    if (!target)
    {
        if (verbose)
            botAI->TellError("我没有目标");

        return false;
    }

    if (!target->IsInWorld())
    {
        return false;
    }

    if (!bot->IsValidAttackTarget(target))
    {
        if (verbose)
            botAI->TellError("我不能攻击无效目标");

        return false;
    }

    std::ostringstream msg;
    msg << target->GetName();

    if (bot->IsFriendlyTo(target))
    {
        msg << " 友善的目标";
        if (verbose)
            botAI->TellError(msg.str());

        return false;
    }

    if (!bot->IsWithinLOSInMap(target))
    {
        msg << " 不在视线内";
        if (verbose)
            botAI->TellError(msg.str());
        
        return false;
    }

    if (target->isDead())
    {
        msg << " 目标死了";
        if (verbose)
            botAI->TellError(msg.str());

        return false;
    }

    if (sPlayerbotAIConfig->IsInPvpProhibitedZone(bot->GetZoneId())
        && (target->IsPlayer() || target->IsPet()))
    {
        if (verbose)
            botAI->TellError("我不能在 PvP 禁止区域攻击他人");

        return false;
    }

    // if (bot->IsMounted() && bot->IsWithinLOSInMap(target))
    // {
    //     WorldPacket emptyPacket;
    //     bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    // }

    ObjectGuid guid = target->GetGUID();
    bot->SetSelection(target->GetGUID());

    Unit* oldTarget = context->GetValue<Unit*>("current target")->Get();
    bool melee = bot->IsWithinMeleeRange(target) || botAI->IsMelee(bot);

    if (oldTarget == target && botAI->GetState() == BOT_STATE_COMBAT && bot->GetVictim() == target && (bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING) == melee))
        return false;

    context->GetValue<Unit*>("old target")->Set(oldTarget);

    context->GetValue<Unit*>("current target")->Set(target);
    context->GetValue<LootObjectStack*>("available loot")->Get()->Add(guid);

    LastMovement& lastMovement = AI_VALUE(LastMovement&, "last movement");
    bool moveControlled = bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE;
    if (lastMovement.priority < MovementPriority::MOVEMENT_COMBAT && bot->isMoving() && !moveControlled)
    {
        AI_VALUE(LastMovement&, "last movement").clear();
        bot->GetMotionMaster()->Clear(false);
        bot->StopMoving();
    }


    if (IsMovingAllowed() && !bot->HasInArc(CAST_ANGLE_IN_FRONT, target))
    {
        sServerFacade->SetFacingTo(bot, target);
    }
    botAI->ChangeEngine(BOT_STATE_COMBAT);
    
    bot->Attack(target, melee);
    /* prevent pet dead immediately in group */
    // if (bot->GetMap()->IsDungeon() && bot->GetGroup() && !target->IsInCombat()) {
    //     with_pet = false;
    // }
    // if (Pet* pet = bot->GetPet())
    // {
    //     if (with_pet) {
    //         pet->SetReactState(REACT_DEFENSIVE);
    //         pet->SetTarget(target->GetGUID());
    //         pet->GetCharmInfo()->SetIsCommandAttack(true);
    //         pet->AI()->AttackStart(target);
    //     } else {
    //         pet->SetReactState(REACT_PASSIVE);
    //         pet->GetCharmInfo()->SetIsCommandFollow(true);
    //         pet->GetCharmInfo()->IsReturning();
    //     }
    // }
    return true;
}

bool AttackDuelOpponentAction::isUseful() { return AI_VALUE(Unit*, "duel target"); }

bool AttackDuelOpponentAction::Execute(Event event) { return Attack(AI_VALUE(Unit*, "duel target")); }
