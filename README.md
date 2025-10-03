# muffinlauncher

Simple-ish CLI Minecraft launcher written in C++

Supports launching the rd- versions up until 1.21.9 (latest, currently)

I only tested a few intermediate versions, so it may be possible not all work, but it should be fine.

**This will only work properly on Apple Silicon Macs currently**

I initially wrote this program in a few hours for my own purposes, however, people wanted it to be published, so here we are.

So please do not stare at the code for too long.

Do note that because this was for my own purposes, it may not work under your configuration, and this is also why I will not provide support for this.

**Use at your own risk, I am not responsible for any damage caused by the use of this tool**

## Features
- It currently only supports offline mode login
- It automatically injects [legacyfix](https://github.com/betacraftuk/legacyfix) for old Minecraft versions, to provide various enhancements to old Minecraft builds.
- Automatic Forge injection
- It currently uses the system Java installation, so make sure you have a compatible version installed and stuff like JAVA_HOME set correctly.

## TODO (maybe)
- Automatic modpack installation
- Fabric support
- Clean up the code, make it more proper, make it work on other OS's
- JRE stuff

