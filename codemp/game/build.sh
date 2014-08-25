#!/bin/bash

#Built for the Clan Mod, by NeWaGe
#---------------------------------
#|     www.theroarclan.com       |
#---------------------------------

#PRE-DEFINE STUFF!
ICC_base="/opt/intel/cc/9.0/bin/iccvars.sh"
SOURCE_base="/media/USB20FD/\"Clan Mod source v1.08\"/\"MP Mod\"/codemp/game"

read -p "Would you like to build the Clan Mod source? (yes/no): " UserResponse

#-if{
if echo $UserResponse | grep -q -s -i ^yes$; then
echo "Where is your ICC compiler located?"
echo "DEFAULT = ($ICC_base)"
echo "Type in \"default\" if your want to use this."
read -p ": " UserResponseICC
elif echo $UserResponse | grep -q -s -i ^no$; then
echo "Stoping build..."
echo ""
echo "Thanks for downloading the Clan Mod source."
echo "Visit our forums for support! -> http://forums.theroarclan.com"
echo "or our website for more information -> www.theroarclan.com"
echo ""
echo "Type in exit if you wish to leave now."
read -p "Would you like to exit? (exit): " UserResponse2
#--if{
if echo $UserResponse2 | grep -q -s -i ^exit$; then
echo "Bye!"
exit 1
fi
#-}
fi
#--}
#---if{
if echo $UserResponseICC | grep -q -s -i ^default$; then
source "$ICC_base"
read -p "Type in \"continue\" to continue building: " UserResponseCONTINUE
else 
source "$UserResponseICC"
read -p "Type in \"continue\" to continue building: " UserResponseCONTINUE
#----if{
#---}
fi
if echo $UserResponseCONTINUE | grep -q -s -i ^continue$; then
make
echo "Done compiling the Clan Mod source!"
echo "The jampgamei386.so file is in your game folder."
echo "Type in \"exit\" to leave now."
read -p ": " UserResponse2
fi
#----}
