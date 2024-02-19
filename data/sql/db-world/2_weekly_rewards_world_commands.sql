DELETE FROM `command` WHERE `name`='activity';
INSERT INTO `command` (`name`, `security`, `help`) VALUES ('activity', 0, 'Syntax: .activity\r\n\r\nDisplays the current activity points you have collected for the week.');
