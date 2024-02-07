#include "WeeklyRewards.h"

#include "Config.h"
#include "GameTime.h"
#include "Player.h"

void WeeklyRewardsPlayerScript::OnPlayerCompleteQuest(Player* player, Quest const* questId)
{
    if (!player)
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

    sWeeklyRewards->UpdatePlayerActivity(guid.GetRawValue(), activity->Points + 1);

    player->SendSystemMessage("|cffffffffYou have earned |cff00ff001 |cffffffffactivity point for completing a quest!|r");
}

void WeeklyRewardsPlayerScript::OnLogin(Player* player)
{
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
}

void WeeklyRewardsPlayerScript::OnLogout(Player* player)
{
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
    sWeeklyRewards->LoadWeeklyRewards();
    sWeeklyRewards->LoadWeeklyActivity();
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

        reward.ItemEntry = itemEntry;
        reward.Count = count;
        reward.MaxCount = maxCount;

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

void WeeklyRewardsHandler::UpdatePlayerActivity(uint64 guid, uint32 points)
{
    auto it = WeeklyActivities.find(guid);
    if (it == WeeklyActivities.end())
    {
        return;
    }

    auto activity = &it->second;
    activity->Points = points;
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

        auto itemCount = reward.Count * points;

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

    MailSender sender(MAIL_CREATURE, 2442); // Cow
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
    UpdatePlayerActivity(guid, 0);
}

bool WeeklyRewardsHandler::CanSendWeeklyRewards()
{
    return true;
}

void SC_AddWeekyRewardsScripts()
{
    new WeeklyRewardsWorldScript();
    new WeeklyRewardsPlayerScript();
    new WeeklyRewardsEventScript();
}
