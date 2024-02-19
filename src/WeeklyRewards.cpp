#include "WeeklyRewards.h"

#include "Config.h"
#include "Chat.h"
#include "GameTime.h"
#include "Player.h"

void WeeklyRewardsPlayerScript::OnPlayerCompleteQuest(Player* player, Quest const* quest)
{
    if (!sConfigMgr->GetOption<bool>("WeeklyRewards.Enable", false))
    {
        return;
    }

    if (!player || !quest)
    {
        return;
    }

    auto guid = player->GetGUID();
    if (!guid)
    {
        return;
    }

    auto activity = sWeeklyRewards->GetPlayerActivity(guid.GetRawValue());
    if (!activity)
    {
        return;
    }

    uint32 points = 0;
    uint32 questId = quest->GetQuestId();

    bool isLFGQuest = false;

    switch (questId)
    {
    case QUEST_ID_DAILY_HEROIC_FIRST:
    case QUEST_ID_DAILY_HEROIC_NTH:
        points = sConfigMgr->GetOption<uint32>("WeeklyRewards.Rewards.ActivityPoints.Dungeon.Heroic", 6);
        isLFGQuest = true;
        break;

    case QUEST_ID_DAILY_NORMAL_FIRST:
    case QUEST_ID_DAILY_NORMAL_NTH:
        points = sConfigMgr->GetOption<uint32>("WeeklyRewards.Rewards.ActivityPoints.Dungeon.Normal", 3);
        isLFGQuest = true;
        break;

    default:
        uint32 difficulty = quest->GetSuggestedPlayers();
        points = difficulty == 0 ? 1 : std::ceil(difficulty / 2);
        break;
    }

    sWeeklyRewards->AddPlayerActivity(player, points,
        isLFGQuest ?
        Acore::StringFormatFmt("|cffffffffYou have earned |cff00ff00{} |cffffffffactivity point(s) for completing a LFG dungeon!|r", points) :
        Acore::StringFormatFmt("|cffffffffYou have earned |cff00ff00{} |cffffffffactivity point(s) for completing a quest!|r", points));
}

void WeeklyRewardsPlayerScript::OnRewardKillRewarder(Player* player, KillRewarder* rewarder, bool /*isDungeon*/, float& /*rate*/)
{
    if (!sConfigMgr->GetOption<bool>("WeeklyRewards.Enable", false))
    {
        return;
    }

    if (!player)
    {
        return;
    }

    auto guid = player->GetGUID();
    if (!guid)
    {
        return;
    }

    if (!rewarder || !rewarder->GetVictim())
    {
        return;
    }

    auto victim = rewarder->GetVictim();
    if (!victim ||
        !victim->ToCreature())
    {
        return;
    }

    auto creature = victim->ToCreature();

    if (sWeeklyRewards->IsCreatureBlacklisted(creature))
    {
        return;
    }

    auto map = victim->GetMap();
    if (!map)
    {
        return;
    }

    auto creatureTemplate = creature->GetCreatureTemplate();

    if (map->IsRaid() &&
        creatureTemplate->rank == CREATURE_ELITE_WORLDBOSS)
    {
        uint32 points = 0;

        if (map->IsHeroic())
        {
            points = sConfigMgr->GetOption<uint32>("WeeklyRewards.Rewards.ActivityPoints.Raid.Heroic.Boss", 6);
        }
        else
        {
            points = sConfigMgr->GetOption<uint32>("WeeklyRewards.Rewards.ActivityPoints.Raid.Normal.Boss", 3);
        }

        sWeeklyRewards->AddPlayerActivity(player, points,
            Acore::StringFormatFmt("|cffffffffYou have earned |cff00ff00{} |cffffffffactivity point(s) for killing a raid boss!|r", points));

        return;
    }

    if (!map->IsRaid() &&
        !map->IsDungeon() &&
        !map->IsBattleground() &&
        (creatureTemplate->rank == CREATURE_ELITE_RAREELITE ||
            creatureTemplate->rank == CREATURE_ELITE_RARE))
    {
        // No credit if your level is further than 8 from the rare.
        uint32 deltaLevel = std::abs(player->GetLevel() - creature->GetLevel());

        if (deltaLevel > 8)
        {
            return;
        }

        uint32 points = 0;

        if (creatureTemplate->rank == CREATURE_ELITE_RARE)
        {
            points = sConfigMgr->GetOption<uint32>("WeeklyRewards.Rewards.ActivityPoints.Rare", 2);
        }

        if (creatureTemplate->rank == CREATURE_ELITE_RAREELITE)
        {
            points = sConfigMgr->GetOption<uint32>("WeeklyRewards.Rewards.ActivityPoints.RareElite", 4);
        }

        sWeeklyRewards->AddPlayerActivity(player, points,
            Acore::StringFormatFmt("|cffffffffYou have earned |cff00ff00{} |cffffffffactivity point(s) for killing a rare!|r", points));

        return;
    }
}

void WeeklyRewardsPlayerScript::OnUpdateGatheringSkill(Player* player, uint32 /*skillId*/, uint32 /*current*/, uint32 gray, uint32 green, uint32 yellow, uint32& gain)
{
    if (!sConfigMgr->GetOption<bool>("WeeklyRewards.Enable", false))
    {
        return;
    }

    if (!player)
    {
        return;
    }

    uint32 points = sConfigMgr->GetOption<uint32>("WeeklyRewards.Rewards.ActivityPoints.Gather", 1);
    uint32 chance = sConfigMgr->GetOption<uint32>("WeeklyRewards.Rewards.ActivityPoints.Gather.Chance", 10);

    uint32 roll = urand(0, 100);
    if (roll > chance)
    {
        return;
    }

    sWeeklyRewards->AddPlayerActivity(player, points,
        Acore::StringFormatFmt("|cffffffffYou have earned |cff00ff00{} |cffffffffactivity point(s) for gathering!|r", points));
}

void WeeklyRewardsPlayerScript::OnLogin(Player* player)
{
    if (!sConfigMgr->GetOption<bool>("WeeklyRewards.Enable", false))
    {
        return;
    }

    if (!player)
    {
        return;
    }

    auto guid = player->GetGUID();
    if (!guid)
    {
        return;
    }

    sWeeklyRewards->CreatePlayerActivity(guid.GetRawValue());

    if (sConfigMgr->GetOption<bool>("WeeklyRewards.ActivityPoints.Notify.Login", true))
    {
        sWeeklyRewards->PrintActivity(player);
    }
}

void WeeklyRewardsPlayerScript::OnLogout(Player* player)
{
    if (!sConfigMgr->GetOption<bool>("WeeklyRewards.Enable", false))
    {
        return;
    }

    if (!player)
    {
        return;
    }

    auto guid = player->GetGUID();
    if (!guid)
    {
        return;
    }

    sWeeklyRewards->SavePlayerActivity(guid.GetRawValue());
}

void WeeklyRewardsWorldScript::OnAfterConfigLoad(bool /*reload*/)
{
    if (!sConfigMgr->GetOption<bool>("WeeklyRewards.Enable", false))
    {
        return;
    }

    sWeeklyRewards->LoadWeeklyRewards();
    sWeeklyRewards->LoadWeeklyActivity();
    sWeeklyRewards->LoadBlacklist();
    sWeeklyRewards->LoadSpecialEncounters();
}

void WeeklyRewardsEventScript::OnStart(uint16 eventId)
{
    if (eventId != 123)
    {
        return;
    }

    if (!sConfigMgr->GetOption<bool>("WeeklyRewards.Enable", false))
    {
        return;
    }

    sWorld->SendServerMessage(SERVER_MSG_STRING, "Flushing weekly rewards..");

    if (sWeeklyRewards->CanSendWeeklyRewards())
    {
        sWeeklyRewards->FlushWeeklyRewards();
    }

    sWorld->SendServerMessage(SERVER_MSG_STRING, "Done flushing weekly rewards.");
}

void WeeklyRewardsHandler::LoadWeeklyRewards()
{
    LOG_INFO("module", "Loading Weekly Rewards from `character_weekly_rewards` table..");

    auto qResult = CharacterDatabase.Query("SELECT * FROM `character_weekly_rewards`");

    if (!qResult)
    {
        LOG_WARN("module", "No weekly rewards found.");
        return;
    }

    WeeklyRewards.clear();

    do
    {
        auto fields = qResult->Fetch();

        WeeklyReward reward;

        auto itemEntry = fields[0].Get<uint32>();
        auto count = fields[1].Get<uint32>();
        auto maxCount = fields[2].Get<uint32>();
        auto multiplier = fields[3].Get<uint32>();

        reward.ItemEntry = itemEntry;
        reward.Count = count;
        reward.MaxCount = maxCount;
        reward.Multiplier = multiplier;

        WeeklyRewards.push_back(reward);
    }
    while (qResult->NextRow());

    LOG_INFO("module", ">> Loaded '{}' weekly rewards.", WeeklyRewards.size());
}

void WeeklyRewardsHandler::LoadWeeklyActivity()
{
    LOG_INFO("module", "Loading Weekly Activity from `character_weekly_activity` table..");

    auto qResult = CharacterDatabase.Query("SELECT * FROM `character_weekly_activity`");

    if (!qResult)
    {
        LOG_WARN("module", "No weekly activity found.");
        return;
    }

    WeeklyActivities.clear();

    do
    {
        auto fields = qResult->Fetch();

        WeeklyActivity activity;

        auto guid = fields[0].Get<uint64>();
        auto points = fields[1].Get<uint32>();

        activity.Guid = guid;
        activity.Points = points;

        WeeklyActivities.emplace(activity.Guid, activity);
    }
    while (qResult->NextRow());

    LOG_INFO("module", ">> Loaded '{}' weekly activity.", WeeklyActivities.size());
}

void WeeklyRewardsHandler::LoadBlacklist()
{
    BlacklistCreatures.clear();

    //  Naxxramas
    {
        BlacklistCreatures.insert(15929); // Thaddius - Stalagg
        BlacklistCreatures.insert(15930); // Thaddius - Feugen

        BlacklistCreatures.insert(30549); // Four Horsemen - Baron Rivendare
        BlacklistCreatures.insert(16063); // Four Horsemen - Sir Zeliek
        BlacklistCreatures.insert(16065); // Four Horsemen - Lady Blaumeux
        BlacklistCreatures.insert(16064); // Four Horsemen - Thane Korthazz
    }
}

void WeeklyRewardsHandler::LoadSpecialEncounters()
{
    SpecialEncounters.clear();

    AddSpecialEncounter(INSTANCE_NAXXRAMAS, INSTANCE_NAXXRAMAS_BOSS_STATE_HORSEMEN);
    AddSpecialEncounter(INSTANCE_MOLTEN_CORE, INSTANCE_MOLTEN_CORE_BOSS_STATE_MAJORDOMO);
}

void WeeklyRewardsHandler::AddSpecialEncounter(uint32 instanceId, uint32 encounterId)
{
    auto it = SpecialEncounters.find(instanceId);
    if (it == SpecialEncounters.end())
    {
        std::unordered_set<uint32> encounterIds;
        auto result = SpecialEncounters.emplace(instanceId, std::unordered_set<uint32>());

        if (!result.second)
        {
            LOG_WARN("module", "Failed to create special encounter set with instance id '{}'.", instanceId);
            return;
        }

        it = result.first;
    }

    auto encounters = it->second;
    if (encounters.find(encounterId) != encounters.end())
    {
        return;
    }

    it->second.emplace(encounterId);
}

bool WeeklyRewardsHandler::IsSpecialEncounter(uint32 instanceId, uint32 encounterId)
{
    auto it = SpecialEncounters.find(instanceId);
    if (it == SpecialEncounters.end())
    {
        return false;
    }

    auto encounters = it->second;
    if (encounters.find(encounterId) == encounters.end())
    {
        return false;
    }

    return true;
}

void WeeklyRewardsHandler::CreatePlayerActivity(uint64 guid)
{
    auto it = WeeklyActivities.find(guid);
    if (it != WeeklyActivities.end())
    {
        return;
    }

    WeeklyActivity activity;

    activity.Guid = guid;
    activity.Points = 0;

    WeeklyActivities.emplace(guid, activity);
}

void WeeklyRewardsHandler::SavePlayerActivity(uint64 guid)
{
    auto it = WeeklyActivities.find(guid);
    if (it == WeeklyActivities.end())
    {
        return;
    }

    auto activity = it->second;

    CharacterDatabase.Execute("INSERT INTO character_weekly_activity (guid, points) VALUES ({}, {}) ON DUPLICATE KEY UPDATE points = '{}'", activity.Guid, activity.Points, activity.Points);
}

WeeklyActivity* WeeklyRewardsHandler::GetPlayerActivity(uint64 guid)
{
    auto it = WeeklyActivities.find(guid);
    if (it == WeeklyActivities.end())
    {
        return nullptr;
    }

    return &it->second;
}

WeeklyRewardsUpdateResult WeeklyRewardsHandler::UpdatePlayerActivity(uint64 guid, uint32 points)
{
    auto it = WeeklyActivities.find(guid);
    if (it == WeeklyActivities.end())
    {
        return WeeklyRewardsUpdateResult::WEEKLY_REWARD_UPDATE_RESULT_NO_ACTIVITY;
    }

    auto activity = &it->second;
    auto activityPoints = activity->Points;

    uint32 maxPoints = sConfigMgr->GetOption<uint32>("WeeklyRewards.ActivityPoints.Maximum", 100);

    if (activityPoints >= maxPoints)
    {
        return WeeklyRewardsUpdateResult::WEEKLY_REWARD_UPDATE_RESULT_MAX;
    }

    bool recentlyMax = false;
    if (points >= maxPoints)
    {
        points = std::clamp(points, 0U, maxPoints);
        recentlyMax = true;
    }

    points = std::clamp(points, 0U, maxPoints);
    activity->Points = points;

    if (recentlyMax)
    {
        return WeeklyRewardsUpdateResult::WEEKLY_REWARD_UPDATE_RESULT_RECENTLY_MAX;
    }

    return WeeklyRewardsUpdateResult::WEEKLY_REWARD_UPDATE_RESULT_OK;
}

void WeeklyRewardsHandler::AddPlayerActivity(Player* player, uint32 newPoints, std::string msg)
{
    if (!player)
    {
        return;
    }

    auto guid = player->GetGUID();

    auto activity = sWeeklyRewards->GetPlayerActivity(guid.GetRawValue());
    if (!activity)
    {
        return;
    }

    auto result = sWeeklyRewards->UpdatePlayerActivity(guid.GetRawValue(), activity->Points + newPoints);

    if (result == WeeklyRewardsUpdateResult::WEEKLY_REWARD_UPDATE_RESULT_NO_ACTIVITY ||
        result == WeeklyRewardsUpdateResult::WEEKLY_REWARD_UPDATE_RESULT_MAX)
    {
        return;
    }

    if (sConfigMgr->GetOption<bool>("WeeklyRewards.ActivityPoints.Notify", true))
    {
        if (!msg.empty())
        {
            player->SendSystemMessage(msg);
        }

        if (result == WeeklyRewardsUpdateResult::WEEKLY_REWARD_UPDATE_RESULT_RECENTLY_MAX)
        {
            player->SendSystemMessage("You have reached your maximum activity points for this week.");
        }
    }
}

void WeeklyRewardsHandler::SendWeeklyRewards(uint64 guid, uint32 points)
{
    std::vector<std::pair<uint32, uint32>> items;

    for (auto reward : WeeklyRewards)
    {
        auto itemProto = sObjectMgr->GetItemTemplate(reward.ItemEntry);
        if (!itemProto)
        {
            LOG_ERROR("module", "Tried to reward item '{}' to player guid '{}' but the item entry does not exist.", reward.ItemEntry, guid);
            continue;
        }

        uint32 achievementPoints = 0;
        if (sConfigMgr->GetOption<bool>("WeeklyRewards.Rewards.AchievementPoints.Scaling", true))
        {
            achievementPoints = GetAchievementPoints(guid);
        }

        float achievementScalar = sConfigMgr->GetOption<float>("WeeklyRewards.Rewards.AchievementPoints.Scalar", 1000);
        float activityScalar = sConfigMgr->GetOption<float>("WeeklyRewards.Rewards.ActivityPoints.Scalar", 1);

        // (Base Count x (Achievement Points / 1000) + Activity Points) x Multiplier
        float itemCount = (reward.Count * ((achievementPoints / achievementScalar) + (points / activityScalar))) * reward.Multiplier;

        itemCount = ceil(itemCount);

        if (itemCount > reward.MaxCount)
        {
            itemCount = reward.MaxCount;
        }

        items.push_back(std::make_pair(reward.ItemEntry, itemCount));
    }

    SendMailItems(guid, items, "Weekly Reward", "You have been rewarded for weekly activity.");
}

void WeeklyRewardsHandler::SendMailItems(uint64 guid, std::vector<std::pair<uint32, uint32>>& mailItems, std::string subject, std::string body)
{
    using SendMailTempateVector = std::vector<std::pair<uint32, uint32>>;

    std::vector<SendMailTempateVector> allItems;

    auto AddMailItem = [&allItems](uint32 itemEntry, uint32 itemCount)
    {
        SendMailTempateVector toSendItems;

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemEntry);
        if (!itemTemplate)
        {
            LOG_ERROR("module", "WeeklyRewards::SendMailItems: Item id {} is invalid", itemEntry);
            return;
        }

        if (itemCount < 1 || (itemTemplate->MaxCount > 0 && itemCount > static_cast<uint32>(itemTemplate->MaxCount)))
        {
            LOG_ERROR("module", "WeeklyRewards::SendMailItems: Incorrect item count ({}) for item id {}", itemCount, itemEntry);
            return;
        }

        while (itemCount > itemTemplate->GetMaxStackSize())
        {
            if (toSendItems.size() <= MAX_MAIL_ITEMS)
            {
                toSendItems.emplace_back(itemEntry, itemTemplate->GetMaxStackSize());
                itemCount -= itemTemplate->GetMaxStackSize();
            }
            else
            {
                allItems.emplace_back(toSendItems);
                toSendItems.clear();
            }
        }

        toSendItems.emplace_back(itemEntry, itemCount);
        allItems.emplace_back(toSendItems);
    };

    for (auto& [itemEntry, itemCount] : mailItems)
    {
        AddMailItem(itemEntry, itemCount);
    }

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    uint32 senderEntry = sConfigMgr->GetOption<uint32>("WeeklyRewards.Mail.Sender", 34337);

    MailSender sender(MAIL_CREATURE, senderEntry);
    MailDraft draft(subject, body);

    for (auto const& items : allItems)
    {
        for (auto const& [itemEntry, itemCount] : items)
        {
            if (Item* mailItem = Item::CreateItem(itemEntry, itemCount))
            {
                mailItem->SaveToDB(trans);
                draft.AddItem(mailItem);
            }
        }
    }

    auto player = ObjectAccessor::FindPlayer(ObjectGuid(guid));

    if (player)
    {
        draft.SendMailTo(trans, MailReceiver(player), sender);
        player->SendSystemMessage("|cff00FF00You have received rewards for your weekly activity, check your mailbox!|r");
    }
    else
    {
        draft.SendMailTo(trans, MailReceiver(guid), sender);
    }

    CharacterDatabase.CommitTransaction(trans);
}

uint32 WeeklyRewardsHandler::GetAchievementPoints(uint64 guid)
{
    try
    {
        std::vector<uint32> achievements;
        QueryResult qResult = CharacterDatabase.Query("SELECT achievement FROM character_achievement WHERE guid = {}", guid);

        if (!qResult ||
            qResult->GetRowCount() < 1)
        {
            return 0;
        }

        do
        {
            Field* fields = qResult->Fetch();
            if (fields->IsNull())
            {
                return 0;
            }

            achievements.push_back(fields[0].Get<uint32>());
        } while (qResult->NextRow());

        uint32 sum = 0;
        for (auto it = achievements.begin(); it != achievements.end(); ++it)
        {
            auto entry = sAchievementStore.LookupEntry(*it);
            sum += entry->points;
        }

        return sum;
    }
    catch (std::exception ex)
    {
        LOG_INFO("module", "WeeklyRewardsHandler failed to load achievements from DB for guid {}: {}", guid, ex.what());
        return 0;
    }

    return 0;
}

bool WeeklyRewardsHandler::IsCreatureBlacklisted(Creature* creature)
{
    uint32 entry = creature->GetCreatureTemplate()->Entry;

    auto it = BlacklistCreatures.find(entry);
    if (it == BlacklistCreatures.end())
    {
        return false;
    }

    return true;
}

void WeeklyRewardsHandler::PrintActivity(Player* player)
{
    if (!player)
    {
        return;
    }

    auto guid = player->GetGUID();
    auto activity = sWeeklyRewards->GetPlayerActivity(guid.GetRawValue());
    if (!activity)
    {
        return;
    }

    if (activity->Points == 0)
    {
        player->SendSystemMessage(Acore::StringFormatFmt("|cffffffffYou have not collected any activity points this week.|r"));
        return;
    }

    auto maxActivity = sConfigMgr->GetOption<uint32>("WeeklyRewards.ActivityPoints.Maximum", 100);

    if (activity->Points >= maxActivity)
    {
        player->SendSystemMessage(Acore::StringFormatFmt("|cffffffffYou have reached your maximum activity points for this week ({}).|r", maxActivity));
        return;
    }

    player->SendSystemMessage(Acore::StringFormatFmt("|cffffffffYou have collected |cff00ff00{} |cffffffffactivity points this week.|r", activity->Points));
}

void WeeklyRewardsHandler::FlushWeeklyRewards()
{
    LOG_INFO("module", "Flushing weekly rewards..");

    uint32 count = 0;
    for (auto entry : WeeklyActivities)
    {
        auto activity = entry.second;
        if (activity.Points < 1)
        {
            continue;
        }

        sWeeklyRewards->SendWeeklyRewards(activity.Guid, activity.Points);
        sWeeklyRewards->ResetWeeklyActivity(activity.Guid);

        count++;
    }

    LOG_INFO("module", ">> Finished flushing '{}' weekly rewards.", count);
}

void WeeklyRewardsHandler::ResetWeeklyActivity(uint64 guid)
{
    auto it = WeeklyActivities.find(guid);
    if (it == WeeklyActivities.end())
    {
        return;
    }

    it->second.Points = 0;
}

bool WeeklyRewardsHandler::CanSendWeeklyRewards()
{
    return true;
}

void WeeklyRewardsGlobalScript::OnBeforeSetBossState(uint32 id, EncounterState newState, EncounterState /*oldState*/, Map* instance)
{
    if (!instance)
    {
        return;
    }

    uint32 instanceId = instance->GetId();

    if (sWeeklyRewards->IsSpecialEncounter(instanceId, id) && newState == DONE)
    {
        uint32 points = 0;

        if (instance->IsHeroic())
        {
            points = sConfigMgr->GetOption<uint32>("WeeklyRewards.Rewards.ActivityPoints.Raid.Heroic.Boss", 6);
        }
        else
        {
            points = sConfigMgr->GetOption<uint32>("WeeklyRewards.Rewards.ActivityPoints.Raid.Normal.Boss", 3);
        }

        instance->DoForAllPlayers([&](Player* player)
        {
            if (!player || !player->IsInWorld())
            {
                return;
            }

            sWeeklyRewards->AddPlayerActivity(player, points,
                Acore::StringFormatFmt("|cffffffffYou have earned |cff00ff00{} |cffffffffactivity point(s) for killing a raid boss!|r", points));
        });
    }
}

ChatCommandTable WeeklyRewardsCommandScript::GetCommands() const
{
    static ChatCommandTable wrCommandTable =
    {
        { "activity", HandleActivityCommand, SEC_PLAYER, Console::No }
    };

    return wrCommandTable;
}

bool WeeklyRewardsCommandScript::HandleActivityCommand(ChatHandler* handler)
{
    if (!handler)
    {
        handler->SetSentErrorMessage(true);
        return false;
    }

    auto player = handler->GetPlayer();
    if (!player)
    {
        handler->SetSentErrorMessage(true);
        return false;
    }

    sWeeklyRewards->PrintActivity(player);
    return true;
}

void SC_AddWeekyRewardsScripts()
{
    new WeeklyRewardsWorldScript();
    new WeeklyRewardsPlayerScript();
    new WeeklyRewardsEventScript();
    new WeeklyRewardsGlobalScript();
    new WeeklyRewardsCommandScript();
}
