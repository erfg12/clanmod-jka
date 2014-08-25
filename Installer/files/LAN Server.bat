rem !!!!!!!!!!!!! change this to the path of your jampded.exe file !!!!!!!!!!!!!!!!!!!!!
cd C:\Program Files\LucasArts\Star Wars Jedi Knight Jedi Academy\GameData

start "Clan Mod Server LAN" /REALTIME jampded.exe +set dedicated 1 +exec server.cfg +set net_port 29070 +set fs_game clanmod