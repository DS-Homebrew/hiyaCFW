<p align="center">
  <img src="https://github.com/RocketRobz/hiyaCFW/blob/master/logo/logo.png"><br>
  <a href="https://gbatemp.net/threads/release-hiyacfw-worlds-first-nintendo-dsi-cfw.502631/">
   <img src="https://img.shields.io/badge/GBAtemp-Thread-blue.svg" alt="GBAtemp Thread">
  </a>
  <a href="https://discord.gg/yD3spjv">
   <img src="https://img.shields.io/badge/Discord%20Server-%23other--nds--homebrew-green.svg" alt="Discord Server">
  </a>
  <a href="https://github.com/RocketRobz/hiyaCFW/actions?query=workflow%3A%22Build+hiyaCFW%22">
    <img src="https://github.com/RocketRobz/hiyaCFW/workflows/Build%20hiyaCFW/badge.svg" alt="Build status on GitHub Actions">
  </a>
</p>

HiyaCFW is the world's FIRST Nintendo DSi CFW, made by the talented folks over on our Discord server.

# Features

- Run custom DSiWare
- NAND to SD card redirection
- Run NAND backups from any console
- Replace the system menu with **TW**i**L**ight Menu++
- Run blocked flashcards (such as R4 Ultra)
- Remove region-locking
- Change the NAND region (Not compatible with CHN and KOR NANDs)
- Run 3DS-exclusive DSiWare (such as WarioWare Touched)
- Custom splash screens

# Compiling

In order to compile this on your own, you will need [devkitPro](https://devkitpro.org/)'s toolchains with the devkitARM, plus the necessary tools and libraries. `dkp-pacman` is included for easy installation of all components:

```
 $ dkp-pacman -Syu devkitARM general-tools dstools ndstool libnds
```

Once everything is downloaded and installed, `git clone` this repository, navigate to the folder, and run `make` to compile HiyaCFW. If there is an error, let us know.

# Credits
- Apache Thunder, NoCash, StuckPixel, Shutterbug2000, and Gericom.
- Drenn: .bmp loading code from GameYob, for custom splash screens.
- Pk11: .gif loading code for animated splash screens.
- Rocket Robz: Logo graphic, settings screen, support for region-changing and any NAND backup.
- devkitPro: For the majority of the base code like nds-bootloader which this loader uses.
