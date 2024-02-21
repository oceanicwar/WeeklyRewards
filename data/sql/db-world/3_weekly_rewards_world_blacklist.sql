CREATE TABLE IF NOT EXISTS `weekly_activity_creature_blacklist` (
  `entry` int NOT NULL,
  `comment` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

DELETE FROM `weekly_activity_creature_blacklist`;
INSERT INTO `weekly_activity_creature_blacklist` (`entry`, `comment`) VALUES
	(14988, 'Zul\'Gurub - Bloodlord Mandokir - Ohgan'),
	(15511, 'AQ(40) - Trio Bug - Lord Kri'),
	(15543, 'AQ(40) - Trio Bug - Princess Yauj'),
	(15544, 'AQ(40) - Trio Bug - Vem'),
	(15929, 'Naxxramas - Thaddius - Stalagg'),
	(15930, 'Naxxramas - Thaddius - Feugen'),
	(16063, 'Naxxramas - Four Horsemen - Sir Zeliek'),
	(16064, 'Naxxramas - Four Horsemen - Thane Korthazz'),
	(16065, 'Naxxramas - Four Horsemen - Lady Blaumeux'),
	(16151, 'Karazhan - Attumen the Huntsman - Midnight'),
	(17256, 'Magtheridon\'s Lair - Hellfire Channeler'),
	(18829, 'Magtheridon\'s Lair - Hellfire Warder'),
	(18831, 'Gruuls Lair - High King Maulgar'),
	(18832, 'Gruuls Lair - High King Maulgar - Krosh Firehand'),
	(18834, 'Gruuls Lair - High King Maulgar - Olm the Summoner'),
	(18835, 'Gruuls Lair - High King Maulgar - Kiggler the Crazed'),
	(18836, 'Gruuls Lair - High King Maulgar - Blindeye the Seer'),
	(30549, 'Naxxramas - Four Horsemen - Baron Rivendare');