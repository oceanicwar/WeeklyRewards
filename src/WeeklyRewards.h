#ifndef MODULE_WEEKLY_REWARDS_H
#define MODULE_WEEKLY_REWARDS_H

#include "ScriptMgr.h"

#include <unordered_map>
#include <vector>

struct WeeklyReward
{
    uint32 ItemEntry;
    uint32 Count;
    uint32 MaxCount;
    uint32 Scalar;
};

struct WeeklyActivity
{
    uint32 Guid;
    uint32 Points;
};

#define sWeeklyRewards WeeklyRewardsHandler::GetInstance()

class WeeklyRewardsHandler
{
private:
    WeeklyRewardsHandler() { }

public:
    static WeeklyRewardsHandler* GetInstance()
    {
        static WeeklyRewardsHandler instance;

        return &instance;
    }

public:
    void LoadWeeklyRewards();
    void LoadWeeklyActivity();
    void CreatePlayerActivity(uint64 /*guid*/);
    void SavePlayerActivity(uint64 /*guid*/);
    WeeklyActivity* GetPlayerActivity(uint64 /*guid*/);
    void UpdatePlayerActivity(uint64 /*guid*/, uint32 /*points*/);
    void SendWeeklyRewards(uint64 /*guid*/, uint32 /*points*/);
    void FlushWeeklyRewards();
    void ResetWeeklyActivity(uint64 /*guid*/);
    bool CanSendWeeklyRewards();
    void SendMailItems(uint64 /*guid*/, std::vector<std::pair<uint32, uint32>>& /*items*/, std::string /*subject*/, std::string /*body*/);
    uint32 GetAchievementPoints(uint64 /*guid*/);

    std::vector<WeeklyReward> WeeklyRewards;
    std::unordered_map<uint64, WeeklyActivity> WeeklyActivities;
};

class WeeklyRewardsPlayerScript : public PlayerScript
{
public:
    WeeklyRewardsPlayerScript() : PlayerScript("WeeklyRewardsPlayerScript") { }

private:
    void OnLogin(Player* /*player*/) override;
    void OnLogout(Player* /*player*/) override;

    void OnPlayerCompleteQuest(Player* /*player*/, Quest const* /*quest*/) override;
    void OnRewardKillRewarder(Player* /*player*/, KillRewarder* /*rewarder*/, bool /*isDungeon*/, float& /*rate*/) override;
};

class WeeklyRewardsWorldScript : public WorldScript
{
public:
    WeeklyRewardsWorldScript() : WorldScript("WeekyRewardsWorldScript") { }

private:
    void OnAfterConfigLoad(bool /*reload*/) override;
};

class WeeklyRewardsEventScript : public GameEventScript
{
public:
    WeeklyRewardsEventScript() : GameEventScript("WeeklyRewardsEventScript") { }

private:
    void OnStart(uint16 /*eventId*/) override;
};

#endif // MODULE_WEEKLY_REWARDS_H
