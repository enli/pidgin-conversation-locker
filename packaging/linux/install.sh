#!/bin/bash
# Pidgin plugin Conversation locker installation script

echo -e "\nMode of installation :"
echo -e "1. System wide installation (recommended)\n2. User specific installation"
echo -en "\nPlease enter your choice (1/2) : "
read in

if [ $in == 1 ]; then
	if [ $UID -eq 0 ]; then
		cp conversationlocker.so /usr/lib/pidgin
		echo "Installation successful!"
		echo "If Pidgin is running, restart it and enable the plugin from Tool->Plugins interface"
		exit
	else
		echo "Installation failed!"
		echo "For system wide installation, you need to run this script as \"sudo $0\""
		exit
	fi
elif [ $in == 2 ]; then
	if [ $UID -eq 0 ]; then
		echo "Installation failed!"
		echo "For user specific installation, you don't need to run this script as sudo."
		echo "Please re-run this script with \"$0\""
		exit
	else
		mkdir -p ~/.purple/plugins
		cp conversationlocker.so ~/.purple/plugins/
		echo "Installation successful!"
		echo "If Pidgin is running, restart it and enable the plugin from Tool->Plugins interface"
		exit
	fi

else
	echo "Installation failed!"
	echo "Invalid choice entered. Valid choices were 1/2. Please re-run this script."
fi

