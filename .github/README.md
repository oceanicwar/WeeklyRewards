# Weekly Rewards
This module adds a weekly event that rewards players based on their weekly activity.

## Setup
1. Clone this module into your `azerothcore-wotlk/modules` directory.
2. Re-run cmake.
3. Re-compile core.
4. Enable the module in the config.
5. Run the server to populate your database with new tables.
6. Setup your reward items in the `character_weekly_rewards` table in database.

## Extra
- If you want to flush the rewards manually you can use the `.event start 123` command.
- If you want to change the reward cycle you can edit the event with the id `123` in the `game_event` table.
