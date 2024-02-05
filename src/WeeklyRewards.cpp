#include "WeeklyRewards.h"

#include "Config.h"
#include "GameTime.h"

void WeeklyRewardsWorldScript::OnAfterConfigLoad(bool reload)
{
    if (!sConfigMgr->GetOption<bool>("WeeklyRewards.Enable", false))
    {
        return;
    }

    /*if (reload)
    {
        return;
    }*/

    LoadWeeklyRewards();
    LoadWeeklyActivity();

    for (auto activity : weeklyActivity)
    {
        if (activity.Points < 1)
        {
            continue;
        }

        SendWeeklyRewards(activity.Guid, activity.Points);
        ResetWeeklyActivity(activity.Guid);
    }
}

void WeeklyRewardsWorldScript::LoadWeeklyRewards()
{
    LOG_INFO("module", "Loading Weekly Rewards from `character_weekly_rewards` table..");

    auto qResult = CharacterDatabase.Query("SELECT * FROM `character_weekly_rewards`");

    if (!qResult)
    {
        LOG_WARN("module", "No weekly rewards found.");
        return;
    }

    weeklyRewards.clear();

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

        weeklyRewards.push_back(reward);
    }
    while (qResult->NextRow());

    LOG_INFO("module", ">> Loaded '{}' weekly rewards.", weeklyRewards.size());
}

void WeeklyRewardsWorldScript::LoadWeeklyActivity()
{
    LOG_INFO("module", "Loading Weekly Activity from `character_weekly_activity` table..");

    auto qResult = CharacterDatabase.Query("SELECT * FROM `character_weekly_activity`");

    if (!qResult)
    {
        LOG_WARN("module", "No weekly activity found.");
        return;
    }

    weeklyActivity.clear();

    do
    {
        auto fields = qResult->Fetch();

        WeeklyActivity activity;

        auto guid = fields[0].Get<uint64>();
        auto points = fields[1].Get<uint32>();

        activity.Guid = guid;
        activity.Points = points;

        weeklyActivity.push_back(activity);
    }
    while (qResult->NextRow());

    LOG_INFO("module", ">> Loaded '{}' weekly activity.", weeklyActivity.size());
}

void WeeklyRewardsWorldScript::SendWeeklyRewards(uint64 guid, uint32 points)
{
    std::vector<std::pair<uint32, uint32>> items;

    for (auto reward : weeklyRewards)
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

void WeeklyRewardsWorldScript::SendMailItems(uint64 guid, std::vector<std::pair<uint32, uint32>>& mailItems, std::string subject, std::string body)
{
    uint32 mailId = sObjectMgr->GenerateMailID();

    uint32 sender = 7;
    uint64 receiver = guid;

    time_t deliveryTime = GameTime::GetGameTime().count();
    time_t expireTime = deliveryTime + (DAY * 30);

    CharacterDatabase.Execute("INSERT INTO `mail` (id, sender, receiver, subject, body, has_items, expire_time, deliver_time, money) VALUES ({}, {}, {}, '{}', '{}', {}, {}, {}, {})", mailId, sender, receiver, subject, body, true, expireTime, deliveryTime, 0);

    for (auto items : mailItems)
    {
        auto item = Item::CreateItem(items.first, items.second);

        if (!item)
        {
            LOG_ERROR("module", "Failed to create mail item '{}' for mail id '{}'.", items.first, mailId);
            continue;
        }

        auto guidLow = item->GetGUID().GetCounter();

        CharacterDatabase.Execute("INSERT INTO `item_instance` (guid, itemEntry, owner_guid, count, enchantments) VALUES ({}, {}, {}, {}, '{}')", guidLow, items.first, receiver, items.second, "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ");
        CharacterDatabase.Execute("INSERT INTO `mail_items` (mail_id, item_guid, receiver) VALUES ({}, {}, {})", mailId, guidLow, receiver);
    }
}

void WeeklyRewardsWorldScript::ResetWeeklyActivity(uint64 guid)
{
    CharacterDatabase.Execute("UPDATE `character_weekly_activity` SET points = 0 WHERE guid = {}", guid);
}

void SC_AddWeekyRewardsScripts()
{
    new WeeklyRewardsWorldScript();
}
