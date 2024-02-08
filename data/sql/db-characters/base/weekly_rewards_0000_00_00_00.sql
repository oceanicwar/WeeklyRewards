CREATE TABLE IF NOT EXISTS `character_weekly_activity` (
  `guid` int NOT NULL,
  `points` int DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

CREATE TABLE IF NOT EXISTS `character_weekly_rewards` (
  `item_entry` int NOT NULL,
  `count` int DEFAULT NULL,
  `max_count` int DEFAULT NULL,
  `scalar` int DEFAULT NULL,
  `note` varchar(50) COLLATE utf8mb4_general_ci DEFAULT NULL,
  PRIMARY KEY (`item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
