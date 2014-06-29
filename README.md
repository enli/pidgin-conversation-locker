Conversation Locker is a plugin for popular multi-protocol messenger [Pidgin](http://www.pidgin.im) which restricts closing of important conversations (windows/tabs) by locking them. In simpler words, with this plugin you can select which conversations should be prevented from closing so those won't be closed by mistake.

Instead of switching to tab, you may mistakenly close some other important tab. Has it ever happened with you? If yes, this is the plugin you need. Just lock important conversations by clicking lock button (shown in screenshots). So the little "X" button on the tab will be unclickable, thus avoiding losing important conversation.

## Features :

- Locks selected conversations and disables all options("X" button on tabs and windows Close menu) preventing closing
- Unlock previously locked conversation
- Prompts user if he chooses to close window containing locked conversations
- Quickly lock/unlock conversations from system tray menu

## Screenshots :

- Lock/unlock button for each conversation (left)
- Locked conversation is prevented from closing by disabling "X" button on tab as well as "Close" menu (right)

![](https://github.com/enli/pidgin-conversation-locker/wiki/screenshots/conversation-locker-lock-button-small.png) 

![](https://github.com/enli/pidgin-conversation-locker/wiki/screenshots/conversation-locker-hides-window-closing-options.png)

- Warns user when he try to close window containing locked conversations (left)
- Locked conversations have inactive "X" button on the tabs (right)

![](https://github.com/enli/pidgin-conversation-locker/wiki/screenshots/conversation-locker-warning.png)

- Quickly lock/unlock all conversations from system tray menu

![](https://github.com/enli/pidgin-conversation-locker/wiki/screenshots/conversation-locker-tray.png)

(Click on images to view full sized)

## Requirements :

- **Pidgin 2.5.0** or later

## Installation :

Installation is quite easy. You could use automatic installers available on [Releases](https://github.com/enli/pidgin-conversation-locker/releases) page or compile from the source. Please select appropriate version from downloads section. Instructions are included in the README file for each version and also available online on [HowToInstall](https://github.com/enli/pidgin-conversation-locker/wiki/How-To-Install) page.


## Configuration :

All you have to do is enable the plugin from Pidgin plugin's panel. Open Pidgin and go to Tools -> Plugins. Find "Conversation Locker" and enable it by checking the checkbox on left side.


## How to use :

- If you have installed plugin properly, you should see a "Chat Unlocked" button on every conversation window/tab.
- Initially the button should read "Chat Unlocked". For locking conversations simply press this button and the button text will change to "Chat Locked", indicating that current conversation is locked and plugin is preventing it from closing.
- If you wish to close previously locked conversation, click on "Chat Locked" button again and then you will be able to close the window.
- If you try to close window containing locked conversations by clicking on "X" on the window border, the plugin will show a warning. Clicking "Yes" will close all conversations included those which are locked. Clicking "No" will stop window from closing.


## Feedback

Do you find plugin useful or it looks completely waste to you? I would love to hear both reactions (mail [me](mailto:peeyoosh.sangolekar@gmail.com)). Advices for improving plugins as well as patches are most welcome.

If you have found a bug, please report it by opening a new issue under [Issues](https://github.com/enli/pidgin-conversation-locker/issues) tab. That would help plugin improve further.


## Known bug :

- Tab context menus "Close other tabs", "Close all tabs", "Close this tab" will close all conversations including those which are locked. Fixing this bug is almost impossible as Pidgin generates these menus dynamically. I am not going to dig into this bug anytime soon.
