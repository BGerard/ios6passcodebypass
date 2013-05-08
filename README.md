iOS 6 Passcode Bypass
======================

This is a hack to disable the iOS6 passcode on a jailbroken device. This is useful if you have forgotten your password and need to retrieve vital information. 

The aim of this tool is to allow you access to your iDevice if you've forgotten your passcode and need to retrieve vital data. It is recommended that this is only used for temporary retrieval of data as this hack can cause WiFi issues with passworded networks. It works on iOS 6.0 - 6.1.2.

Usage
======
To use this tool in its CLI form simply navigate to the sender directory and use the command:
./sender -s

This will then run the iOS6 exploit
You can then run a cleaner by using the command:
./sender -p

I have explained more about the workings via code comments.

You can also build your own copy if you do so wish.

How It Works
=============
This tool works by establishing an AFC2 connection to your device and creating an exploit folder in the root of the device. It then copies a launch daemon and script to this folder and modifies the launched.conf as placed there by the evasion jailbreak (explained more in code comments). Which I believe uses the launchd.conf vulnerability within iOS6, which you can see more here: http://theiphonewiki.com/wiki/Launchd.conf_untether

It then uses this custom launchd.conf file to give the script root permissions so it can be run by the launch daemon.

The script run by the launch daemon installs a debian package which installs a mobile substrate tweak that hooks into the SpringBoard to disable the passcode lock. 

To Do
=====
Create GUI
Add support for iOS 3,4 and 5 (maybe)

Credits
========
Comex - for providing a basis for the afc
Dustin Howett - for creating the amazing tool that is theos (and logo)

Please follow me on twitter @ben_gerard
