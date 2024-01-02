TEMPLATE = app
TARGET = Espers-fractal-qt
VERSION = 0.8.8.9
INCLUDEPATH += src src/json src/qt
QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
DEFINES += ENABLE_WALLET
DEFINES += QT_GUI BOOST_THREAD_USE_LIB BOOST_SPIRIT_THREADSAFE
CONFIG += no_include_pwd
CONFIG += thread
CONFIG += static
CONFIG += openssl

QMAKE_CXXFLAGS += -fpermissive

# WIN32 OS
# for boost 1.66 on windows, add (MinGW_Version)-mt-s-x32-(Boost_Version)
# as a reference refer to the below section

win32{
BOOST_LIB_SUFFIX=-mgw8-mt-s-x32-1_74
BOOST_INCLUDE_PATH=C:/deps/boost_1_74_0
BOOST_LIB_PATH=C:/deps/boost_1_74_0/stage/lib
BDB_INCLUDE_PATH=C:/deps/db-6.2.32.NC/build_unix
BDB_LIB_PATH=C:/deps/db-6.2.32.NC/build_unix
OPENSSL_INCLUDE_PATH=C:/deps/openssl-1.0.2u/include
OPENSSL_LIB_PATH=C:/deps/openssl-1.0.2u
MINIUPNPC_INCLUDE_PATH=C:/deps/
MINIUPNPC_LIB_PATH=C:/deps/miniupnpc-2.1
QRENCODE_INCLUDE_PATH=C:/deps/qrencode-4.0.2
QRENCODE_LIB_PATH=C:/deps/qrencode-4.0.2/.libs
}

!win32:!macx{
BDB_INCLUDE_PATH=/usr/local/BerkeleyDB.6.2/include
BDB_LIB_PATH=/usr/local/BerkeleyDB.6.2/lib
}

# OTHER OS
# for boost 1.37, add -mt to the boost libraries
# use: qmake BOOST_LIB_SUFFIX=-mt
# for boost thread win32 with _win32 sufix
# use: BOOST_THREAD_LIB_SUFFIX=_win32-...
# or when linking against a specific BerkelyDB version: BDB_LIB_SUFFIX=-4.8

# Dependency library locations can be customized with:
#    BOOST_INCLUDE_PATH, BOOST_LIB_PATH, BDB_INCLUDE_PATH,
#    BDB_LIB_PATH, OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively

# workaround for boost 1.58
DEFINES += BOOST_VARIANT_USE_RELAXED_GET_BY_DEFAULT

OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build

# use: qmake "RELEASE=1"
contains(RELEASE, 1) {
    # Mac: compile for maximum compatibility (10.12, 64-bit)
    macx:QMAKE_CXXFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_CFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_LFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk
    macx:QMAKE_OBJECTIVE_CFLAGS += -mmacosx-version-min=10.12 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk


    !windows:!macx {
        # Linux: static link
        # LIBS += -Wl,-Bstatic
    }
}

!win32 {
# for extra security against potential buffer overflows: enable GCCs Stack Smashing Protection
QMAKE_CXXFLAGS *= -fstack-protector-all --param ssp-buffer-size=1
QMAKE_LFLAGS *= -fstack-protector-all --param ssp-buffer-size=1
# We need to exclude this for Windows cross compile with MinGW 4.2.x, as it will result in a non-working executable!
# This can be enabled for Windows, when we switch to MinGW >= 4.4.x.
}
# for extra security (see: https://wiki.debian.org/Hardening): this flag is GCC compiler-specific
QMAKE_CXXFLAGS += -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2
# for extra security on Windows: enable ASLR and DEP via GCC linker flags
win32:QMAKE_LFLAGS *= -Wl,--dynamicbase -Wl,--nxcompat
# on Windows: enable GCC large address aware linker flag
win32:QMAKE_LFLAGS *= -Wl,--large-address-aware -static
# i686-w64-mingw32
win32:QMAKE_LFLAGS *= -static-libgcc -static-libstdc++
# use: qmake "USE_QRCODE=1"
# libqrencode (http://fukuchi.org/works/qrencode/index.en.html) must be installed for support
USE_QRCODE=1
contains(USE_QRCODE, 1) {
    message(Building with QRCode support)
    DEFINES += USE_QRCODE
    LIBS += -lqrencode
}

# use: qmake "USE_UPNP=1" ( enabled by default; default)
#  or: qmake "USE_UPNP=0" (disabled by default)
#  or: qmake "USE_UPNP=-" (not supported)
# miniupnpc (http://miniupnp.free.fr/files/) must be installed for support
contains(USE_UPNP, -) {
message(Building without UPNP support)
} else {
message(Building with UPNP support)
count(USE_UPNP, 0) {
USE_UPNP=1
}
DEFINES += DMINIUPNP_STATICLIB
INCLUDEPATH += $$MINIUPNPC_INCLUDE_PATH
LIBS += $$join(MINIUPNPC_LIB_PATH,,-L,) -lminiupnpc
win32:LIBS += -liphlpapi
}

USE_DBUS=0
# use: qmake "USE_DBUS=1" or qmake "USE_DBUS=0"
linux:count(USE_DBUS, 0) {
    USE_DBUS=1
}
contains(USE_DBUS, 1) {
    message(Building with DBUS (Freedesktop notifications) support)
    DEFINES += USE_DBUS
    QT += dbus
}

contains(ESPERS_NEED_QT_PLUGINS, 1) {
    DEFINES += ESPERS_NEED_QT_PLUGINS
    QTPLUGIN += qcncodecs qjpcodecs qtwcodecs qkrcodecs qtaccessiblewidgets
}

INCLUDEPATH += src/leveldb/include src/leveldb/helpers
LIBS += $$PWD/src/leveldb/libleveldb.a $$PWD/src/leveldb/libmemenv.a
SOURCES += src/database/txdb-leveldb.cpp \
    src/crypto/common/aes_helper.c \
    src/crypto/common/blake.c \
    src/crypto/common/bmw.c \
    src/crypto/common/groestl.c \
    src/crypto/common/jh.c \
    src/crypto/common/keccak.c \
    src/crypto/common//skein.c \
    src/crypto/common/luffa.c \
    src/crypto/common/cubehash.c \
    src/crypto/common/shavite.c \
    src/crypto/common/echo.c \
    src/crypto/common/simd.c \
    src/crypto/common/hamsi.c \
    src/crypto/common/fugue.c \
    src/crypto/common/shabal.c \
    src/crypto/common/whirlpool.c \
    src/crypto/common/haval.c \
    src/crypto/common/sha2big.c
!win32 {
    # we use QMAKE_CXXFLAGS_RELEASE even without RELEASE=1 because we use RELEASE to indicate linking preferences not -O preferences
    genleveldb.commands = cd $$PWD/src/leveldb && CC=$$QMAKE_CC CXX=$$QMAKE_CXX $(MAKE) OPT=\"$$QMAKE_CXXFLAGS $$QMAKE_CXXFLAGS_RELEASE\" libleveldb.a libmemenv.a
} else {
    # make an educated guess about what the ranlib command is called
    isEmpty(QMAKE_RANLIB) {
        QMAKE_RANLIB = $$replace(QMAKE_STRIP, strip, ranlib)
    }
    LIBS += -lshlwapi
    # genleveldb.commands = cd $$PWD/src/leveldb && CC=$$QMAKE_CC CXX=$$QMAKE_CXX TARGET_OS=OS_WINDOWS_CROSSCOMPILE $(MAKE) OPT=\"$$QMAKE_CXXFLAGS $$QMAKE_CXXFLAGS_RELEASE\" libleveldb.a libmemenv.a && $$QMAKE_RANLIB $$PWD/src/leveldb/libleveldb.a && $$QMAKE_RANLIB $$PWD/src/leveldb/libmemenv.a
}
genleveldb.target = $$PWD/src/leveldb/libleveldb.a
genleveldb.depends = FORCE
PRE_TARGETDEPS += $$PWD/src/leveldb/libleveldb.a
QMAKE_EXTRA_TARGETS += genleveldb
# Gross ugly hack that depends on qmake internals, unfortunately there is no other way to do it.
# QMAKE_CLEAN += $$PWD/src/leveldb/libleveldb.a; cd $$PWD/src/leveldb ; $(MAKE) clean

# regenerate src/build.h
!windows|contains(USE_BUILD_INFO, 1) {
    genbuild.depends = FORCE
    genbuild.commands = cd $$PWD; /bin/sh tools/genbuild.sh $$OUT_PWD/build/build.h
    genbuild.target = $$OUT_PWD/build/build.h
    PRE_TARGETDEPS += $$OUT_PWD/build/build.h
    QMAKE_EXTRA_TARGETS += genbuild
    DEFINES += HAVE_BUILD_INFO
}

contains(USE_O3, 1) {
    message(Building O3 optimization flag)
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS += -O3
    QMAKE_CFLAGS += -O3
}

contains(USE_O0, 1) {
    message(Building O0 optimization flag)
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS += -O0
    QMAKE_CFLAGS += -O0
}

*-g++-32 {
    message("32 platform, adding -msse2 flag")

    QMAKE_CXXFLAGS += -msse2
    QMAKE_CFLAGS += -msse2
}

QMAKE_CXXFLAGS_WARN_ON = -fdiagnostics-show-option -Wall -Wextra -Wno-ignored-qualifiers -Wformat -Wformat-security -Wno-unused-parameter -Wstack-protector

# Input
DEPENDPATH += src src/json src/qt
HEADERS += src/qt/bitcoingui.h \
    src/qt/transactiontablemodel.h \
    src/qt/addresstablemodel.h \
    src/qt/bantablemodel.h \
    src/qt/optionsdialog.h \
    src/qt/coincontroldialog.h \
    src/qt/coincontroltreewidget.h \
    src/qt/sendcoinsdialog.h \
    src/qt/addressbookpage.h \
    src/qt/clientcontrolpage.h \
    src/qt/messagepage.h \
    src/qt/blockbrowser.h \
    src/qt/settingspage.h \
    src/qt/signverifymessagedialog.h \
    src/qt/aboutdialog.h \
    src/qt/editaddressdialog.h \
    src/qt/editconfigdialog.h \
    src/qt/importprivatekeydialog.h \
    src/qt/bitcoinaddressvalidator.h \
    src/node/alert.h \
    src/core/blocksizecalculator.h \
    src/node/addrman.h \
    src/primitives/base58.h \
    src/primitives/bignum.h \
    src/core/blockparams.h \
    src/core/chainparams.h \
    src/core/chainparamsseeds.h \
    src/consensus/checkpoints.h \
    src/consensus/cphashes.h \
    src/consensus/fork.h \
    src/consensus/genesis.h \
    src/primitives/compat.h \
    src/subcore/coincontrol.h \
    src/core/sync.h \
    src/util/util.h \
    src/subcore/hash.h \
    src/primitives/uint256.h \
    src/consensus/kernel.h \
    src/crypto/scrypt/scrypt.h \
    src/primitives/pbkdf2.h \
    src/primitives/serialize.h \
    src/core/chain.h \
    src/core/main.h \
    src/consensus/miner.h \
    src/consensus/mining.h \
    src/node/net.h \
    src/subcore/key.h \
    src/database/db.h \
    src/database/txdb.h \
    src/subcore/txmempool.h \
    src/database/walletdb.h \
    src/subcore/script.h \
    src/util/init.h \
    src/primitives/mruset.h \
    src/consensus/velocity.h \
    src/rpc/rpcvelocity.h \
    src/json/json_spirit_writer_template.h \
    src/json/json_spirit_writer.h \
    src/json/json_spirit_value.h \
    src/json/json_spirit_utils.h \
    src/json/json_spirit_stream_reader.h \
    src/json/json_spirit_reader_template.h \
    src/json/json_spirit_reader.h \
    src/json/json_spirit_error_position.h \
    src/json/json_spirit.h \
    src/qt/clientmodel.h \
    src/qt/guiutil.h \
    src/qt/transactionrecord.h \
    src/qt/guiconstants.h \
    src/qt/optionsmodel.h \
    src/qt/monitoreddatamapper.h \
    src/qt/peertablemodel.h \
    src/qt/trafficgraphwidget.h \
    src/qt/transactiondesc.h \
    src/qt/transactiondescdialog.h \
    src/qt/bitcoinamountfield.h \
    src/core/wallet.h \
    src/subcore/keystore.h \
    src/qt/transactionfilterproxy.h \
    src/qt/transactionview.h \
    src/qt/walletmodel.h \
    src/rpc/rpcclient.h \
    src/rpc/rpcprotocol.h \
    src/rpc/rpcserver.h \
    src/subcore/timedata.h \
    src/qt/overviewpage.h \
    src/qt/csvmodelwriter.h \
    src/subcore/crypter.h \
    src/qt/sendcoinsentry.h \
    src/qt/qvalidatedlineedit.h \
    src/qt/bitcoinunits.h \
    src/qt/qvaluecombobox.h \
    src/qt/askpassphrasedialog.h \
    src/subcore/protocol.h \
    src/qt/notificator.h \
    src/qt/paymentserver.h \
    src/primitives/allocators.h \
    src/ui/ui_interface.h \
    src/qt/rpcconsole.h \
    src/qt/rpcconsolesettings.h \
    src/consensus/version.h \
    src/node/netbase.h \
    src/consensus/clientversion.h \
    src/primitives/threadsafety.h \
    src/primitives/tinyformat.h \
    src/primitives/limitedmap.h \
    src/qt/fractalui.h \
    src/qt/tokenui.h \
    src/qt/nftui.h \
    src/qt/contractui.h \
    src/qt/macnotificationhandler.h \
    src/crypto/hmq/hmq1725.h \
    src/crypto/bmw/bmw512.h \
    src/crypto/common/sph_blake.h \
    src/crypto/common/sph_bmw.h \
    src/crypto/common/sph_groestl.h \
    src/crypto/common/sph_jh.h \
    src/crypto/common/sph_keccak.h \
    src/crypto/common/sph_skein.h \
    src/crypto/common/sph_types.h \
    src/crypto/common/sph_luffa.h \
    src/crypto/common/sph_cubehash.h \
    src/crypto/common/sph_echo.h \
    src/crypto/common/sph_shavite.h \
    src/crypto/common/sph_simd.h \
    src/crypto/common/sph_hamsi.h \
    src/crypto/common/sph_fugue.h \
    src/crypto/common/sph_shabal.h \
    src/crypto/common/sph_whirlpool.h \
    src/crypto/common/sph_haval.h \
    src/crypto/common/sph_sha2.h \
    src/stb/stb_image.h \
    src/stb/stb_image_write.h \
    src/fractal/fractalengine.h \
    src/fractal/fractalcontract.h \
    src/fractal/fractalparams.h \
    src/fractal/fractaldataob.h \
    src/fractal/fractalnftbase.h \
    src/fractal/fractalnft.h \
    src/fractal/fractalbvac.h \
    src/xnode/xnodecomponent.h \
    src/xnode/xnodemngr.h \
    src/xnode/xnodereward.h \
    src/xnode/xnodesettings.h \
    src/xnode/xnodestart.h \
    src/xnode/xnodesync.h \
    src/xnode/xnodecrypter.h \
    src/deminode/demimodule.h \
    src/deminode/deminet.h \
    src/deminode/demisync.h \
    src/pas/pas.h \
    src/pas/pasengine.h \
    src/pas/pasman.h \
    src/pas/pasconf.h \
    src/pas/pasregister.h

SOURCES += src/qt/bitcoin.cpp src/qt/bitcoingui.cpp \
    src/qt/transactiontablemodel.cpp \
    src/qt/addresstablemodel.cpp \
    src/qt/bantablemodel.cpp \
    src/qt/optionsdialog.cpp \
    src/qt/sendcoinsdialog.cpp \
    src/qt/coincontroldialog.cpp \
    src/qt/coincontroltreewidget.cpp \
    src/qt/addressbookpage.cpp \
    src/qt/signverifymessagedialog.cpp \
    src/qt/aboutdialog.cpp \
    src/qt/editaddressdialog.cpp \
    src/qt/editconfigdialog.cpp \
    src/qt/importprivatekeydialog.cpp \
    src/qt/bitcoinaddressvalidator.cpp \
    src/node/alert.cpp \
    src/core/blocksizecalculator.cpp \
    src/core/blockparams.cpp \
    src/core/chainparams.cpp \
    src/consensus/version.cpp \
    src/core/sync.cpp \
    src/subcore/txmempool.cpp \
    src/util/util.cpp \
    src/subcore/hash.cpp \
    src/node/netbase.cpp \
    src/subcore/key.cpp \
    src/subcore/script.cpp \
    src/core/chain.cpp \
    src/core/main.cpp \
    src/consensus/miner.cpp \
    src/util/init.cpp \
    src/node/net.cpp \
    src/consensus/velocity.cpp \
    src/rpc/rpcvelocity.cpp \
    src/consensus/checkpoints.cpp \
    src/consensus/fork.cpp \
    src/node/addrman.cpp \
    src/database/db.cpp \
    src/database/walletdb.cpp \
    src/qt/clientmodel.cpp \
    src/qt/clientcontrolpage.cpp \
    src/qt/messagepage.cpp \
    src/qt/blockbrowser.cpp \
    src/qt/settingspage.cpp \
    src/qt/guiutil.cpp \
    src/qt/transactionrecord.cpp \
    src/qt/optionsmodel.cpp \
    src/qt/monitoreddatamapper.cpp \
    src/qt/peertablemodel.cpp \
    src/qt/trafficgraphwidget.cpp \
    src/qt/transactiondesc.cpp \
    src/qt/transactiondescdialog.cpp \
    src/qt/bitcoinstrings.cpp \
    src/qt/bitcoinamountfield.cpp \
    src/core/wallet.cpp \
    src/subcore/keystore.cpp \
    src/qt/transactionfilterproxy.cpp \
    src/qt/transactionview.cpp \
    src/qt/walletmodel.cpp \
    src/rpc/rpcclient.cpp \
    src/rpc/rpcprotocol.cpp \
    src/rpc/rpcserver.cpp \
    src/rpc/rpcdump.cpp \
    src/rpc/rpcmisc.cpp \
    src/rpc/rpcnet.cpp \
    src/rpc/rpcmining.cpp \
    src/rpc/rpcwallet.cpp \
    src/rpc/rpcblockchain.cpp \
    src/rpc/rpcrawtransaction.cpp \
    src/subcore/timedata.cpp \
    src/qt/overviewpage.cpp \
    src/qt/csvmodelwriter.cpp \
    src/subcore/crypter.cpp \
    src/qt/sendcoinsentry.cpp \
    src/qt/qvalidatedlineedit.cpp \
    src/qt/bitcoinunits.cpp \
    src/qt/qvaluecombobox.cpp \
    src/qt/askpassphrasedialog.cpp \
    src/subcore/protocol.cpp \
    src/qt/notificator.cpp \
    src/qt/paymentserver.cpp \
    src/qt/rpcconsole.cpp \
    src/qt/rpcconsolesettings.cpp \
    src/subcore/noui.cpp \
    src/consensus/kernel.cpp \
    src/crypto/scrypt/scrypt-arm.S \
    src/crypto/scrypt/scrypt-x86.S \
    src/crypto/scrypt/scrypt-x86_64.S \
    src/crypto/scrypt/scrypt.cpp \
    src/primitives/pbkdf2.cpp \
    src/qt/fractalui.cpp \
    src/qt/tokenui.cpp \
    src/qt/nftui.cpp \
    src/qt/contractui.cpp \
    src/fractal/fractalengine.cpp \
    src/fractal/fractalcontract.cpp \
    src/fractal/fractalparams.cpp \
    src/fractal/fractaldataob.cpp \
    src/fractal/fractalnftbase.cpp \
    src/fractal/fractalnft.cpp \
    src/fractal/fractalbvac.cpp \
    src/xnode/xnoderpc.cpp \
    src/xnode/xnodecomponent.cpp \
    src/xnode/xnodemngr.cpp \
    src/xnode/xnodereward.cpp \
    src/xnode/xnodesettings.cpp \
    src/xnode/xnodestart.cpp \
    src/xnode/xnodesync.cpp \
    src/xnode/xnodecrypter.cpp \
    src/deminode/demimodule.cpp \
    src/deminode/deminet.cpp \
    src/deminode/demisync.cpp \
    src/rpc/rpcpasengine.cpp \
    src/pas/pas.cpp \
    src/pas/pasengine.cpp \
    src/pas/pasman.cpp \
    src/pas/pasconf.cpp \
    src/pas/pasregister.cpp

RESOURCES += \
    src/qt/bitcoin.qrc

FORMS += \
    src/qt/forms/coincontroldialog.ui \
    src/qt/forms/sendcoinsdialog.ui \
    src/qt/forms/addressbookpage.ui \
    src/qt/forms/signverifymessagedialog.ui \
    src/qt/forms/aboutdialog.ui \
    src/qt/forms/editaddressdialog.ui \
    src/qt/forms/editconfigdialog.ui \
    src/qt/forms/importprivatekeydialog.ui \
    src/qt/forms/transactiondescdialog.ui \
    src/qt/forms/overviewpage.ui \
    src/qt/forms/clientcontrolpage.ui \
    src/qt/forms/messagepage.ui \
    src/qt/forms/blockbrowser.ui \
    src/qt/forms/settingspage.ui \
    src/qt/forms/sendcoinsentry.ui \
    src/qt/forms/askpassphrasedialog.ui \
    src/qt/forms/rpcconsole.ui \
    src/qt/forms/rpcconsolesettings.ui \
    src/qt/forms/optionsdialog.ui \
    src/qt/forms/fractalui.ui \
    src/qt/forms/tokenui.ui \
    src/qt/forms/nftui.ui \
    src/qt/forms/contractui.ui

contains(USE_QRCODE, 1) {
HEADERS += src/qt/qrcodedialog.h
SOURCES += src/qt/qrcodedialog.cpp
FORMS += src/qt/forms/qrcodedialog.ui
}

CODECFORTR = UTF-8

# for lrelease/lupdate
# also add new translations to src/qt/bitcoin.qrc under translations/
TRANSLATIONS = $$files(src/qt/locale/bitcoin_*.ts)

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
isEmpty(QM_DIR):QM_DIR = $$PWD/src/qt/locale
# automatically build translations, so they can be included in resource file
TSQM.name = lrelease ${QMAKE_FILE_IN}
TSQM.input = TRANSLATIONS
TSQM.output = $$QM_DIR/${QMAKE_FILE_BASE}.qm
TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
TSQM.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += TSQM

# "Other files" to show in Qt Creator
OTHER_FILES += \
    doc/*.rst doc/*.txt doc/README README.md res/bitcoin-qt.rc

# platform specific defaults, if not overridden on command line
isEmpty(BOOST_LIB_SUFFIX) {
    macx:BOOST_LIB_SUFFIX = -mt
    windows:BOOST_LIB_SUFFIX = -mt
}

isEmpty(BOOST_THREAD_LIB_SUFFIX) {
    # win32:BOOST_THREAD_LIB_SUFFIX = _win32$$BOOST_LIB_SUFFIX
    # else:
    BOOST_THREAD_LIB_SUFFIX = $$BOOST_LIB_SUFFIX
}

isEmpty(BDB_LIB_PATH) {
    macx:BDB_LIB_PATH = /opt/local/lib/db48
}

isEmpty(BDB_LIB_SUFFIX) {
    macx:BDB_LIB_SUFFIX = -4.8
}

isEmpty(BDB_INCLUDE_PATH) {
    macx:BDB_INCLUDE_PATH = /opt/local/include/db48
}

isEmpty(BOOST_LIB_PATH) {
    macx:BOOST_LIB_PATH = /opt/local/lib
}

isEmpty(BOOST_INCLUDE_PATH) {
    macx:BOOST_INCLUDE_PATH = /opt/local/include
}

windows:DEFINES += WIN32
windows:RC_FILE = src/qt/res/bitcoin-qt.rc

windows:!contains(MINGW_THREAD_BUGFIX, 0) {
    # At least qmake's win32-g++-cross profile is missing the -lmingwthrd
    # thread-safety flag. GCC has -mthreads to enable this, but it doesn't
    # work with static linking. -lmingwthrd must come BEFORE -lmingw, so
    # it is prepended to QMAKE_LIBS_QT_ENTRY.
    # It can be turned off with MINGW_THREAD_BUGFIX=0, just in case it causes
    # any problems on some untested qmake profile now or in the future.
    DEFINES += _MT BOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN
    QMAKE_LIBS_QT_ENTRY = -lmingwthrd $$QMAKE_LIBS_QT_ENTRY
}

macx:HEADERS += src/qt/macdockiconhandler.h
macx:OBJECTIVE_SOURCES += src/qt/macdockiconhandler.mm
macx:LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
macx:DEFINES += MAC_OSX MSG_NOSIGNAL=0
macx:ICON = src/qt/res/icons/Espers.icns
macx:TARGET = "Espers-Qt"
macx:QMAKE_CFLAGS_THREAD += -pthread
macx:QMAKE_LFLAGS_THREAD += -pthread
macx:QMAKE_CXXFLAGS_THREAD += -pthread
macx:QMAKE_INFO_PLIST = share/qt/Info.plist

# Set libraries and includes at end, to use platform-defined defaults if not overridden
INCLUDEPATH += $$BOOST_INCLUDE_PATH $$BDB_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH $$QRENCODE_INCLUDE_PATH
LIBS += $$join(BOOST_LIB_PATH,,-L,) $$join(BDB_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,) $$join(QRENCODE_LIB_PATH,,-L,)
LIBS += -lssl -lcrypto -ldb_cxx$$BDB_LIB_SUFFIX
# -lgdi32 has to happen after -lcrypto (see  #681)
windows:LIBS += -lws2_32 -lshlwapi -lmswsock -lole32 -loleaut32 -luuid -lgdi32
windows:LIBS += libboost_system$$BOOST_LIB_SUFFIX libboost_filesystem$$BOOST_LIB_SUFFIX libboost_program_options$$BOOST_LIB_SUFFIX libboost_thread$$BOOST_THREAD_LIB_SUFFIX
!windows:LIBS += -lboost_system$$BOOST_LIB_SUFFIX -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX -lboost_chrono$$BOOST_LIB_SUFFIX
windows:LIBS += libboost_chrono$$BOOST_LIB_SUFFIX

contains(RELEASE, 1) {
    !windows:!macx {
        # Linux: turn dynamic linking back on for c/c++ runtime libraries
        LIBS += -Wl,-Bdynamic
    }
}

!windows:!macx {
    DEFINES += LINUX
    LIBS += -lrt -ldl
}

system($$QMAKE_LRELEASE -silent $$_PRO_FILE_)

OBJECTIVE_SOURCES += \
    src/qt/macnotificationhandler.mm
