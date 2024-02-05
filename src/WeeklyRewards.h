#ifndef MODULE_WEEKLY_REWARDS_H
#define MODULE_WEEKLY_REWARDS_H

#include "ScriptMgr.h"

#include <vector>

struct WeeklyReward
{
    uint32 ItemEntry;
    uint32 Count;
    uint32 MaxCount;
};

struct WeeklyActivity
{
    uint32 Guid;
    uint32 Points;
};

class WeeklyRewardsWorldScript : public WorldScript
{
public:
    WeeklyRewardsWorldScript() : WorldScript("WeeklyRewardsWorldScript") { }

public:
    void OnAfterConfigLoad(bool /*reload*/) override;

private:
    void LoadWeeklyRewards();
    void LoadWeeklyActivity();
    void SendWeeklyRewards(uint64 /*guid*/, uint32 /*points*/);
    void ResetWeeklyActivity(uint64 /*guid*/);
    void SendMailItems(uint64 /*guid*/, std::vector<std::pair<uint32, uint32>>& /*items*/, std::string /*subject*/, std::string /*body*/);

private:
    std::vector<WeeklyReward> weeklyRewards;
    std::vector<WeeklyActivity> weeklyActivity;
};

#endif // MODULE_WEEKLY_REWARDS_H
