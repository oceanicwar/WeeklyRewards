DELETE FROM `game_event` WHERE `eventEntry`=123;
INSERT INTO `game_event` (`eventEntry`, `start_time`, `end_time`, `occurence`, `length`, `holiday`, `holidayStage`, `description`, `world_event`, `announce`) VALUES (123, '2000-01-02 04:00:00', '2031-01-02 02:00:00', 2, 1, 0, 0, 'Weekly Rewards Flush', 0, 2);
