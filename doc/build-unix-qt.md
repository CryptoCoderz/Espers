Linux/Unix Espers-qt: Qt5 GUI for Espers
===============================

### Navigation of these documents
- [Back to General Readme](../README.md)
- [Back to Installation Guide](../INSTALL.md)


Build instructions
===================

Compiling Espers Qt on Ubuntu 22.04 LTS (Jammy Jellyfish)
---------------------------
### Note: guide should be compatible with other Ubuntu/Linux versions from 14.04+

### Become poweruser
```
sudo -i
```

### Create swap file (if system has less than 2GB of RAM)
```
cd ~; sudo fallocate -l 3G /swapfile; ls -lh /swapfile; sudo chmod 600 /swapfile; ls -lh /swapfile; sudo mkswap /swapfile; sudo swapon /swapfile; sudo swapon --show; sudo cp /etc/fstab /etc/fstab.bak; echo '/swapfile none swap sw 0 0' | sudo tee -a /etc/fstab
```

### Dependencies install
Install standard dependencies:
```
cd ~; apt-get install -y ntp git build-essential libssl-dev libdb-dev libdb++-dev libboost-all-dev libqrencode-dev libcurl4-openssl-dev curl libzip-dev; apt-get update -y; apt-get install -y git make automake build-essential libboost-all-dev; apt-get install -y yasm binutils libcurl4-openssl-dev openssl libssl-dev; sudo apt-get install -y libgmp-dev;
```
Install Qt dependencies:
```
sudo apt-get install -y qtcreator qtbase5-dev qttools5-dev qttools5-dev-tools qt5-qmake cmake
```
Install extended dependencies:
```
sudo apt-get install -y autoconf autotools-dev pkg-config zlib1g-dev
```

### Dependencies build and link
Build standard dependencies:
```
cd ~; wget http://download.oracle.com/berkeley-db/db-6.2.32.NC.tar.gz; tar zxf db-6.2.32.NC.tar.gz; cd db-6.2.32.NC/build_unix; ../dist/configure --enable-cxx; make; make install; ln -s /usr/local/BerkeleyDB.6.2/lib/libdb-6.2.so /usr/lib/libdb-6.2.so;ln -s /usr/local/BerkeleyDB.6.2/lib/libdb_cxx-6.2.so /usr/lib/libdb_cxx-6.2.so; export BDB_INCLUDE_PATH="/usr/local/BerkeleyDB.6.2/include"; export BDB_LIB_PATH="/usr/local/BerkeleyDB.6.2/lib"; cd ~;
```
Qt Dependencies build and link (1 of 2):
```
wget https://ppa.launchpadcontent.net/linuxuprising/libpng12/ubuntu/pool/main/libp/libpng/libpng_1.2.54.orig.tar.xz; tar Jxfv libpng_1.2.54.orig.tar.xz; cd ~/libpng-1.2.54; ./configure; make; sudo make install; cd ~; sudo ln -s /usr/local/lib/libpng12.so.0.54.0 /usr/lib/libpng12.so; sudo ln -s /usr/local/lib/libpng12.so.0.54.0 /usr/lib/libpng12.so.0
```
Qt Dependencies build and link (2 of 2):
```
wget https://fukuchi.org/works/qrencode/qrencode-4.0.2.tar.gz; tar zxfv qrencode-4.0.2.tar.gz; cd ~/qrencode-4.0.2; ./configure; make; sudo make install; sudo ldconfig
```

Ubuntu Legacy Patch (Ubuntu 18.04 and older)
```
cp -r ~/Espers/src/qt/forms/signverifymessagedialog.ui.legacy_qt ~/Espers/src/qt/forms/signverifymessagedialog.ui
cp -r ~/Espers/src/qt/forms/rpcconsolesettings.ui.legacy_qt ~/Espers/src/qt/forms/rpcconsolesettings.ui
cp -r ~/Espers/src/qt/forms/rpcconsole.ui.legacy_qt ~/Espers/src/qt/forms/rpcconsole.ui
```

### GitHub pull (Source Download)
```
cd ~; git clone https://github.com/CryptoCoderz/Espers
```

### Build Espers-Qt (GUI wallet) on Linux 

**All previous steps must be completed first.**
(If you recompile some other time you don't have to repeat previous steps.)

Build Espers Qt
```
cd ~/Espers; qmake -qt=qt5 USE_UPNP=-; make
```
**Upon success the built Espers-qt program will be in the ~/Espers folder (directory)**

### Run Espers daemon
```
cd ~; ~/Espers/Espers-fractal-qt
```

### Troubleshooting
### for basic troubleshooting run the following commands when compiling:
### this is for minupnpc errors compiling
```
make clean
make
```


Dependencies Used
-----------------

 Library     Purpose           Description
 -------     -------           -----------
 libssl      SSL Support       Secure communications
 libdb       Berkeley DB       Blockchain & wallet storage
 libboost    Boost             C++ Library
 miniupnpc   UPnP Support      Optional firewall-jumping support
 libqrencode QRCode generation Optional QRCode generation

Note that libexecinfo should be installed, if you building under BSD systems. 
This library provides backtrace facility.

miniupnpc may be used for UPnP port mapping.  It can be downloaded from
http://miniupnp.tuxfamily.org/files/.  UPnP support is compiled in and
turned off by default.  Set USE_UPNP to a different value to control this:
 USE_UPNP=-    No UPnP support - miniupnp not required
 USE_UPNP=0    (the default) UPnP support turned off by default at runtime
 USE_UPNP=1    UPnP support turned on by default at runtime

libqrencode may be used for QRCode image generation. It can be downloaded
from http://fukuchi.org/works/qrencode/index.html.en, or installed via
your package manager. Set USE_QRCODE to control this:
 USE_QRCODE=0   (the default) No QRCode support - libqrcode not required
 USE_QRCODE=1   QRCode support enabled

Licenses of statically linked libraries:
 Berkeley DB   New BSD license with additional requirement that linked
               software must be free open source
 Boost         MIT-like license
 miniupnpc     New (3-clause) BSD license

Versions used in this release:
 GCC           8.1.0
 OpenSSL       1.0.2u - 3.0.5
 Berkeley DB   6.2.32.NC
 Boost         1.74.0
 miniupnpc     2.1.20190824
 qrencode      4.0.2