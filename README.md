# Simple System Manager

A system management tool with a Qt GUI interface.

## Requirements
- Qt 6.9 (nice)
- Systemd development packages
- Root privileges
- C++23 compatible compiler
- Will to live (optional, like as if I have it, and I created this thing)


> [!IMPORTANT]
> Building guide and dependencies may be wrong or outdated. I just dont have enough time nor do i have a motivation to maintain a readme. Surely nobody's doing this without my guidance anyway. Thanks!

## Building
```bash
mkdir build
cd build
cmake ..
make
```

## Distribution-Specific Dependencies

### Fedora (based distros)
```bash
# In case something goes wrong with the installation try to enable rpmfusion
sudo dnf install https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
sudo dnf install https://download1.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm
#if it still does not work try this
sudo rm -f /etc/yum.repos.d/fedora-cisco-openh264.repo

# Install required packages
sudo dnf install qt6-qtbase qt6-qttools-devel qt6-linguist qt6-qtsvg-devel qt6-qtwayland qt6-qtcharts-devel cmake gcc-g++ systemd-devel
```

### Debian, Ubuntu (based distros)
```bash
sudo apt install qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-l10n-tools libqt6svg6-dev qt6-qpa-plugins qt6-wayland libqt6charts6-dev libqt6charts6 cmake build-essential libsystemd-dev
```

### Arch Linux (based distros)
```bash
sudo pacman -S qt6-base qt6-tools qt6-svg qt6-wayland qt6-charts cmake base-devel systemd
```

### Windows
```bash
curl ascii.live/rick
#there wont be windows support anytime soon.
```

If any of these blocks are not fixing dependency issues contact me!

## Note
If you encounter problems during installation or use:
> I believe in you! You can do it, you can solve them yourself!!

## Information deemed important enough for me
This program is an absolute nightmare to maintain and update btw lol

---

### About me
Here's my Roblox avatar: *(on the right)*  
![Roblox Avatar](https://i.imgur.com/bW3nmVZ.png)

