<p align="center">
  <img src="https://github.com/RocketRobz/hiyaCFW/blob/master/logo/logo.png"><br>
  <span style="padding-right: 5px;">
    <a href="https://travis-ci.org/RocketRobz/hiyaCFW">
      <img src="https://travis-ci.org/RocketRobz/hiyaCFW.svg?branch=unlaunch">
    </a>
  </span>
  <span style="padding-left: 5px;">
    <a href="https://dshomebrew.serveo.net/">
      <img src="https://github.com/ahezard/nds-bootstrap/blob/master/images/Rocket.Chat button.png" height="20">
    </a>
  </span>
</p>

HiyaCFW is the world's FIRST Nintendo DSi CFW, made by the talented folks over on our Rocket.Chat server.

# Features

- Run custom DSiWare
- Have NAND to SD card redirection
- Replace the system menu with TWiLightMenu++
- Run blocked flashcards (such as R4 Ultra)
- Remove Region-Locking
- Run 3DS-exclusive DSiWare (such as WarioWare Touched)

# Compiling

In order to compile this on your own, you will need [devkitPro](https://devkitpro.org/) with the devkitARM toolchain, plus the necessary tools and libraries. DevkitPro includes `dkp-pacman` for easy installation of all components:

```
 $ dkp-pacman -Syu devkitARM general-tools dstools ndstool libnds libfat-nds
```

Once everything is downloaded and installed, `git clone` this repository, navigate to the folder, and run `make` to compile nds-bootstrap. If there is an error, let us know.

# Rocket.Chat Server

Would you like a place to talk about your experiences with nds-bootstrap? Do you need some assistance? Well, why not join our Rocket.Chat server!

Rocket.Chat is a self-hosted communication platform with the ability to share files and switch to an video/audio conferencing.

Come join today by following this link: https://dshomebrew.serveo.net/ ([alternative link](https://b2b38a00.ngrok.io))

# Credits
- Apache Thunder, NoCash, StuckPixel, Shutterbug2000, and Gericom.
- Drenn: .bmp loading code from GameYob, for custom splash screens.
- RocketRobz: Logo graphic, and settings screen.
- WinterMute/devkitPro: For the majority of the base code like nds-bootloader which this loader uses.
