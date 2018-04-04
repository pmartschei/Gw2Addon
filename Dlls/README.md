# IncQol
# Increased Quality of Life Addon for GW2

This is a release without a source.

This addon is literally a hack like arcDPS and should not be public.
It does nothing harmful and you can reverse engineer or decompile my .dll
The .dll is not obfuscated and there is no internet traffic going outwards or vice versa.
You can do whatever you want with the .dll

# How to Install?

Just put the d3d9.dll in your bin64 folder.

The d3d9.dll has a function to chainload other .dlls
To do this, simply rename the other .dll "d3d9_incqol_chainload.dll" (default)
This addon can also be chainloaded.

# What does it do?

With this addon, at the moment, you can sell items from your inventory faster by automatically letting them filter by this addon. 
The user can create different filters to manage which items will be filtered and so on. The addon will filter the inventory in the background and makes it ready to be sold. 
The User has to press "Sell Junk" in order to sell all filtered Items.

At default, the addon will never filter three rarities (junk,ascended,legendary), in which ascended can be unlocked in the config(for securitiy purposes).

# Additional infos

After the first start, there are different config/log files under "%installation-path%\addons\incqol\"
The configs can be modified in the game.
The log is just to there to find out where something went wrong.
When you find bugs or your game crashes I need the .log and if you know how to reproduce the bug/crash.

Some hotkeys could be a bit weird (wrong name or no name, only happend when my OS was on german idk why), it should work anyway.

The addon doesn't need to be updated after every patch(just like arcDPS), when something is not wroking either the addon will tell you in the options window or the addon won't do anything.

# FAQ
### How can I disable selling/filtering?
You can disable the selling/filtering in the config or remove the d3d9.dll completely

### I dont see anything? What are the default hotkeys?
Shift+Alt+1 to open the options window, where you can see all hotkeys
Shift+Alt+2 to open the filter window

### Can I use it on the 32-Bit client?
No I only support the 64-Bit client

### Does the addon connect to a server in any way?
Technically yes(tradingpost api if activated), but no other than gw2 servers.

### Can I get banned using this addon?
Probably not, it does the same what arcDPS does.
You do not gain any advantage (build templates).
The addon will NOT sell anything automatically without user input.

### Can this addon delete items or sell wrong ones?
Pretty unlikely when nothing changes.
I tested the addon a long time (play testing all game modes, let other people test the tool, play 6 hour straight)
It never deleted an item nor did it sell a wrong one.

# Disclaimer
I am NOT responsible if items are deleted / sold by mistake or wrong configuration. Use at your own risk.
