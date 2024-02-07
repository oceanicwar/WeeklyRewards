#include "WeeklyRewards.h"

#include "Config.h"
#include "GameTime.h"
#include "Player.h"

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

    LoadWeeklyRewards();
    LoadWeeklyActivity();

    if (CanSendWeeklyRewards())
    {
        FlushWeeklyRewards();
    }

    sWorld->SendServerMessage(SERVER_MSG_STRING, "Done flushing weekly rewards.");
}

void WeeklyRewardsEventScript::LoadWeeklyRewards()
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

void WeeklyRewardsEventScript::LoadWeeklyActivity()
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

void WeeklyRewardsEventScript::SendWeeklyRewards(uint64 guid, uint32 points)
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

void WeeklyRewardsEventScript::SendMailItems(uint64 guid, std::vector<std::pair<uint32, uint32>>& mailItems, std::string subject, std::string body)
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

void WeeklyRewardsEventScript::FlushWeeklyRewards()
{
    LOG_INFO("module", "Flushing weekly rewards..");

    uint32 count = 0;
    for (auto activity : weeklyActivity)
    {
        if (activity.Points < 1)
        {
            continue;
        }

        SendWeeklyRewards(activity.Guid, activity.Points);
        ResetWeeklyActivity(activity.Guid);

        count++;
    }

    LOG_INFO("module", ">> Finished flushing '{}' weekly rewards.", count);
}

void WeeklyRewardsEventScript::ResetWeeklyActivity(uint64 guid)
{
    CharacterDatabase.Execute("UPDATE `character_weekly_activity` SET points = 0 WHERE guid = {}", guid);
}

bool WeeklyRewardsEventScript::CanSendWeeklyRewards()
{
    return true;
}

void SC_AddWeekyRewardsScripts()
{
    new WeeklyRewardsEventScript();
}
