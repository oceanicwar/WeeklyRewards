#ifndef MODULE_WEEKLY_REWARDS_H
#define MODULE_WEEKLY_REWARDS_H

#include "ScriptMgr.h"
#include "KillRewarder.h"

#include <unordered_map>
#include <vector>

struct WeeklyReward
{
    uint32 ItemEntry;
    uint32 Count;
    uint32 MaxCount;
    uint32 Multiplier;
};

struct WeeklyActivity
{
    uint32 Guid;
    uint32 Points;
};

enum WeeklyRewardsUpdateResult
{
    WEEKLY_REWARD_UPDATE_RESULT_RECENTLY_MAX = 0,
    WEEKLY_REWARD_UPDATE_RESULT_MAX = 1,
    WEEKLY_REWARD_UPDATE_RESULT_NO_ACTIVITY = 2,
    WEEKLY_REWARD_UPDATE_RESULT_OK = 3
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
    void LoadBlacklist();
    void CreatePlayerActivity(uint64 /*guid*/);
    void SavePlayerActivity(uint64 /*guid*/);
    WeeklyActivity* GetPlayerActivity(uint64 /*guid*/);
    WeeklyRewardsUpdateResult UpdatePlayerActivity(uint64 /*guid*/, uint32 /*points*/);
    void AddPlayerActivity(Player* /*player*/, uint32 /*newPoints*/, std::string msg = "");
    void SendWeeklyRewards(uint64 /*guid*/, uint32 /*points*/);
    void FlushWeeklyRewards();
    void ResetWeeklyActivity(uint64 /*guid*/);
    bool CanSendWeeklyRewards();
    void SendMailItems(uint64 /*guid*/, std::vector<std::pair<uint32, uint32>>& /*items*/, std::string /*subject*/, std::string /*body*/);
    uint32 GetAchievementPoints(uint64 /*guid*/);
    bool IsCreatureBlacklisted(Creature* /*creature*/);

    std::vector<WeeklyReward> WeeklyRewards;
    std::unordered_map<uint64, WeeklyActivity> WeeklyActivities;
    std::unordered_set<uint32> BlacklistCreatures;
};

class WeeklyRewardsPlayerScript : public PlayerScript
{
public:
    WeeklyRewardsPlayerScript() : PlayerScript("WeeklyRewardsPlayerScript") { }

private:
    enum WeeklyRewardsConstants
    {
        QUEST_ID_DAILY_HEROIC_FIRST = 24788,
        QUEST_ID_DAILY_HEROIC_NTH = 24789,
        QUEST_ID_DAILY_NORMAL_FIRST = 24790,
        QUEST_ID_DAILY_NORMAL_NTH = 24791
    };
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

class WeeklyRewardsGlobalScript : public GlobalScript
{
public:
    WeeklyRewardsGlobalScript() : GlobalScript("WeeklyRewardsGlobalScript") { }

private:
    enum WeeklyRewardsGlobalScriptConstants
    {
        INSTANCE_NAXXRAMAS = 533,
        INSTANCE_NAXXRAMAS_BOSS_STATE_HORSEMEN = 12
    };
private:
    void OnBeforeSetBossState(uint32 /*id*/, EncounterState /*newState*/, EncounterState /*oldState*/, Map* /*instance*/) override;
};

#endif // MODULE_WEEKLY_REWARDS_H
