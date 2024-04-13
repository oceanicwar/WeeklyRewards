#ifndef MODULE_WEEKLY_REWARDS_H
#define MODULE_WEEKLY_REWARDS_H

#include "ChatCommand.h"
#include "ScriptMgr.h"
#include "KillRewarder.h"

#include <unordered_map>
#include <vector>

using namespace Acore::ChatCommands;

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
private:
    enum WeeklyRewardsHandlerConstants
    {
        INSTANCE_NAXXRAMAS = 533,
        INSTANCE_NAXXRAMAS_BOSS_STATE_HORSEMEN = 12,

        INSTANCE_MOLTEN_CORE = 409,
        INSTANCE_MOLTEN_CORE_BOSS_STATE_MAJORDOMO = 8
    };
public:
    void LoadWeeklyRewards();
    void LoadWeeklyActivity();
    void LoadBlacklist();
    void LoadSpecialEncounters();
    void AddSpecialEncounter(uint32 /*instanceId*/, uint32 /*encounterId*/);
    bool IsSpecialEncounter(uint32 /*instanceId*/, uint32 /*encounterId*/);
    void CreatePlayerActivity(uint64 /*guid*/);
    void SavePlayerActivity(uint64 /*guid*/);
    WeeklyActivity* GetPlayerActivity(uint64 /*guid*/);
    WeeklyRewardsUpdateResult UpdatePlayerActivity(uint64 /*guid*/, uint32 /*points*/);
    void AddPlayerActivity(Player* /*player*/, uint32 /*newPoints*/, std::string msg = "");
    void SendWeeklyRewards(uint64 /*guid*/, uint32 /*points*/);
    void FlushWeeklyRewards();
    void ResetWeeklyActivity(WeeklyActivity* /*activity*/);
    bool CanSendWeeklyRewards();
    void SendMailItems(uint64 /*guid*/, std::vector<std::pair<uint32, uint32>>& /*items*/, std::string /*subject*/, std::string /*body*/);
    uint32 GetAchievementPoints(uint64 /*guid*/);
    bool IsCreatureBlacklisted(Creature* /*creature*/);
    void PrintActivity(Player* /*player*/);

    std::vector<WeeklyReward> WeeklyRewards;
    std::unordered_map<uint64, WeeklyActivity> WeeklyActivities;
    std::unordered_set<uint32> BlacklistCreatures;
    std::unordered_map<uint32, std::unordered_set<uint32>> SpecialEncounters;
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
    void OnUpdateGatheringSkill(Player* /*player*/, uint32 /*skillId*/, uint32 /*current*/, uint32 /*gray*/, uint32 /*green*/, uint32 /*yellow*/, uint32& /*gain*/) override;
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
    void OnBeforeSetBossState(uint32 /*id*/, EncounterState /*newState*/, EncounterState /*oldState*/, Map* /*instance*/) override;
};

class WeeklyRewardsCommandScript : public CommandScript
{
public:
    WeeklyRewardsCommandScript() : CommandScript("WeeklyRewardsCommandScript") { }

private:
    ChatCommandTable GetCommands() const override;
    static bool HandleActivityCommand(ChatHandler* /*handler*/);
};

#endif // MODULE_WEEKLY_REWARDS_H
