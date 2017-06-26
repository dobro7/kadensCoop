CREATE TABLE IF NOT EXISTS `tempLog` (
  `timeStamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `coopTemp` float NOT NULL,
  `runTemp` float NOT NULL,
  `doorStatus` varchar(20) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'Processing',
  `photocellStatus` varchar(20) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'Processing',
  `heatStatus` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `fanStatus` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `lightStatus` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `waterHeaterStatus` varchar(20) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
