-- phpMyAdmin SQL Dump
-- version 4.0.10.14
-- http://www.phpmyadmin.net
--
-- Host: localhost:3306
-- Generation Time: Sep 15, 2016 at 10:45 AM
-- Server version: 5.6.31
-- PHP Version: 5.6.20

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `clanmod`
--

-- --------------------------------------------------------

--
-- Table structure for table `jedi_academy`
--

CREATE TABLE IF NOT EXISTS `jedi_academy` (
  `user_id` int(255) NOT NULL,
  `kills` int(255) NOT NULL DEFAULT '0',
  `deaths` int(255) NOT NULL DEFAULT '0',
  `duel_wins` int(255) NOT NULL DEFAULT '0',
  `duel_loses` int(255) NOT NULL DEFAULT '0',
  `flag_captures` int(255) NOT NULL DEFAULT '0',
  `ffa_wins` int(255) NOT NULL DEFAULT '0',
  `ffa_loses` int(255) NOT NULL DEFAULT '0',
  `tdm_wins` int(255) NOT NULL DEFAULT '0',
  `tdm_loses` int(255) NOT NULL DEFAULT '0',
  `siege_wins` int(255) NOT NULL DEFAULT '0',
  `siege_loses` int(255) NOT NULL DEFAULT '0',
  `ctf_wins` int(255) NOT NULL DEFAULT '0',
  `ctf_loses` int(255) NOT NULL DEFAULT '0',
  UNIQUE KEY `user_id` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE IF NOT EXISTS `users` (
  `id` int(255) NOT NULL AUTO_INCREMENT,
  `username` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `ipaddress` varchar(20) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `username` (`username`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
