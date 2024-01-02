// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "init.h"

#include "node/addrman.h"
#include "core/main.h"
#include "core/chainparams.h"
#include "database/txdb.h"
#include "rpc/rpcserver.h"
#include "node/net.h"
#include "subcore/key.h"
#include "core/main.h"
#include "util.h"
#include "ui/ui_interface.h"
#include "consensus/checkpoints.h"
#ifdef ENABLE_WALLET
#include "database/db.h"
#include "core/wallet.h"
#include "database/walletdb.h"

#include "xnode/xnodestart.h"
#include "xnode/xnodereward.h"
#include "xnode/xnodecomponent.h"
#include "xnode/xnodemngr.h"
#include "xnode/xnodesettings.h"
#include "xnode/xnodecrypter.h"
#include "consensus/fork.h"
#endif

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/function.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/thread.hpp>
#include <openssl/crypto.h>

#ifndef WIN32
#include <signal.h>
#endif

#ifdef Q_OS_WIN
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QFileInfo>
#endif

using namespace std;
using namespace boost;

#ifdef ENABLE_WALLET
CWallet* pwalletMain = NULL;
int nWalletBackups = 10;
int nNewHeight;
#endif
CClientUIInterface uiInterface;
bool fConfChange;
bool fMinimizeCoinAge;
unsigned int nNodeLifespan;
unsigned int nDerivationMethodIndex;
unsigned int nMinerSleep;
bool fUseFastIndex;

//////////////////////////////////////////////////////////////////////////////
//
// Shutdown
//

//
// Thread management and startup/shutdown:
//
// The network-processing threads are all part of a thread group
// created by AppInit() or the Qt main() function.
//
// A clean exit happens when StartShutdown() or the SIGTERM
// signal handler sets fRequestShutdown, which triggers
// the DetectShutdownThread(), which interrupts the main thread group.
// DetectShutdownThread() then exits, which causes AppInit() to
// continue (it .joins the shutdown thread).
// Shutdown() is then
// called to clean up database connections, and stop other
// threads that should only be stopped after the main network-processing
// threads have exited.
//
// Note that if running -daemon the parent process returns from AppInit2
// before adding any threads to the threadGroup, so .join_all() returns
// immediately and the parent exits from main().
//
// Shutdown for Qt is very similar, only it uses a QTimer to detect
// fRequestShutdown getting set, and then does the normal Qt
// shutdown thing.
//

volatile bool fRequestShutdown = false;

void StartShutdown()
{
    fRequestShutdown = true;
}
bool ShutdownRequested()
{
    return fRequestShutdown;
}

void Shutdown()
{
    LogPrintf("Shutdown : In progress...\n");
    static CCriticalSection cs_Shutdown;
    TRY_LOCK(cs_Shutdown, lockShutdown);
    if (!lockShutdown) return;

    RenameThread("Espers-shutoff");
    mempool.AddTransactionsUpdated(1);
    StopRPCThreads();
#ifdef ENABLE_WALLET
    ShutdownRPCMining();
    if (pwalletMain)
        bitdb.Flush(false);
#endif
    StopNode();
    DumpXNodes();
    DumpXNodePayments();
    {
        LOCK(cs_main);
#ifdef ENABLE_WALLET
        if (pwalletMain)
            pwalletMain->SetBestChain(CBlockLocator(pindexBest));
#endif
    }
#ifdef ENABLE_WALLET
    if (pwalletMain)
        bitdb.Flush(true);
#endif
    boost::filesystem::remove(GetPidFile());
    UnregisterAllWallets();
#ifdef ENABLE_WALLET
    delete pwalletMain;
    pwalletMain = NULL;
#endif
    LogPrintf("Shutdown : done\n");
}

//
// Signal handlers are very limited in what they are allowed to do, so:
//
void HandleSIGTERM(int)
{
    fRequestShutdown = true;
}

void HandleSIGHUP(int)
{
    fReopenDebugLog = true;
}

bool static InitError(const std::string &str)
{
    uiInterface.ThreadSafeMessageBox(str, "", CClientUIInterface::MSG_ERROR);
    return false;
}

bool static InitWarning(const std::string &str)
{
    uiInterface.ThreadSafeMessageBox(str, "", CClientUIInterface::MSG_WARNING);
    return true;
}

bool static Bind(const CService &addr, bool fError = true) {
    if (IsLimited(addr))
        return false;
    std::string strError;
    if (!BindListenPort(addr, strError)) {
        if (fError)
            return InitError(strError);
        return false;
    }
    return true;
}

// Core-specific options shared between UI and daemon
std::string HelpMessage()
{
    string strUsage = _("Options:") + "\n";
    strUsage += "  -?                     " + _("This help message") + "\n";
    strUsage += "  -conf=<file>           " + _("Specify configuration file (default: Espers.conf)") + "\n";
    strUsage += "  -pid=<file>            " + _("Specify pid file (default: Espersd.pid)") + "\n";
    strUsage += "  -datadir=<dir>         " + _("Specify data directory") + "\n";
    strUsage += "  -wallet=<dir>          " + _("Specify wallet file (within data directory)") + "\n";
    strUsage += "  -dbcache=<n>           " + _("Set database cache size in megabytes (default: 25)") + "\n";
    strUsage += "  -dblogsize=<n>         " + _("Set database disk log size in megabytes (default: 100)") + "\n";
    strUsage += "  -timeout=<n>           " + _("Specify connection timeout in milliseconds (default: 5000)") + "\n";
    strUsage += "  -proxy=<ip:port>       " + _("Connect through SOCKS5 proxy") + "\n";
    strUsage += "  -tor=<ip:port>         " + _("Use proxy to reach tor hidden services (default: same as -proxy)") + "\n";
    strUsage += "  -dns                   " + _("Allow DNS lookups for -addnode, -seednode and -connect") + "\n";
    strUsage += "  -port=<port>           " + _("Listen for connections on <port> (default: 15714 or testnet: 25714)") + "\n";
    strUsage += "  -maxconnections=<n>    " + _("Maintain at most <n> connections to peers (default: 125)") + "\n";
    strUsage += "  -addnode=<ip>          " + _("Add a node to connect to and attempt to keep the connection open") + "\n";
    strUsage += "  -connect=<ip>          " + _("Connect only to the specified node(s)") + "\n";
    strUsage += "  -seednode=<ip>         " + _("Connect to a node to retrieve peer addresses, and disconnect") + "\n";
    strUsage += "  -externalip=<ip>       " + _("Specify your own public address") + "\n";
    strUsage += "  -onlynet=<net>         " + _("Only connect to nodes in network <net> (IPv4, IPv6 or Tor)") + "\n";
    strUsage += "  -discover              " + _("Discover own IP address (default: 1 when listening and no -externalip)") + "\n";
    strUsage += "  -listen                " + _("Accept connections from outside (default: 1 if no -proxy or -connect)") + "\n";
    strUsage += "  -bind=<addr>           " + _("Bind to given address. Use [host]:port notation for IPv6") + "\n";
    strUsage += "  -dnsseed               " + _("Query for peer addresses via DNS lookup, if low on addresses (default: 1 unless -connect)") + "\n";
    strUsage += "  -forcednsseed          " + _("Always query for peer addresses via DNS lookup (default: 0)") + "\n";
    strUsage += "  -synctime              " + _("Sync time with other nodes. Disable if time on your system is precise e.g. syncing with NTP (default: 1)") + "\n";
    strUsage += "  -cppolicy              " + _("Sync checkpoints policy (default: strict)") + "\n";
    strUsage += "  -banscore=<n>          " + _("Threshold for disconnecting misbehaving peers (default: 100)") + "\n";
    strUsage += "  -bantime=<n>           " + _("Number of seconds to keep misbehaving peers from reconnecting (default: 86400)") + "\n";
    strUsage += "  -maxreceivebuffer=<n>  " + _("Maximum per-connection receive buffer, <n>*1000 bytes (default: 5000)") + "\n";
    strUsage += "  -maxsendbuffer=<n>     " + _("Maximum per-connection send buffer, <n>*1000 bytes (default: 1000)") + "\n";
#ifdef USE_UPNP
#if USE_UPNP
    strUsage += "  -upnp                  " + _("Use UPnP to map the listening port (default: 1 when listening)") + "\n";
#else
    strUsage += "  -upnp                  " + _("Use UPnP to map the listening port (default: 0)") + "\n";
#endif
#endif
    strUsage += "  -paytxfee=<amt>        " + _("Fee per KB to add to transactions you send") + "\n";
    strUsage += "  -mininput=<amt>        " + _("When creating transactions, ignore inputs with value less than this (default: 0.01)") + "\n";
    if (fHaveGUI)
        strUsage += "  -server                " + _("Accept command line and JSON-RPC commands") + "\n";
#if !defined(WIN32)
    if (fHaveGUI)
        strUsage += "  -daemon                " + _("Run in the background as a daemon and accept commands") + "\n";
#endif
    strUsage += "  -testnet               " + _("Use the test network") + "\n";
    strUsage += "  -debug=<category>      " + _("Output debugging information (default: 0, supplying <category> is optional)") + "\n";
    strUsage +=                               _("If <category> is not supplied, output all debugging information.") + "\n";
    strUsage +=                               _("<category> can be:");
    strUsage +=                                 " addrman, alert, db, lock, rand, rpc, selectcoins, mempool, net,"; // Don't translate these and qt below
    strUsage +=                                 " coinage, coinstake, creation, stakemodifier";
    if (fHaveGUI)
    {
        strUsage += ", qt.\n";
    }
    else
    {
        strUsage += ".\n";
    }
    strUsage += "  -logtimestamps         " + _("Prepend debug output with timestamp") + "\n";
    strUsage += "  -shrinkdebugfile       " + _("Shrink debug.log file on client startup (default: 1 when no -debug)") + "\n";
    strUsage += "  -printtoconsole        " + _("Send trace/debug info to console instead of debug.log file") + "\n";
    strUsage += "  -regtest               " + _("Enter regression test mode, which uses a special chain in which blocks can be "
                                                "solved instantly. This is intended for regression testing tools and app development.") + "\n";
    strUsage += "  -rpcuser=<user>        " + _("Username for JSON-RPC connections") + "\n";
    strUsage += "  -rpcpassword=<pw>      " + _("Password for JSON-RPC connections") + "\n";
    strUsage += "  -rpcport=<port>        " + _("Listen for JSON-RPC connections on <port> (default: 15715 or testnet: 25715)") + "\n";
    strUsage += "  -rpcallowip=<ip>       " + _("Allow JSON-RPC connections from specified IP address") + "\n";
    if (!fHaveGUI)
    {
        strUsage += "  -rpcconnect=<ip>       " + _("Send commands to node running on <ip> (default: 127.0.0.1)") + "\n";
        strUsage += "  -rpcwait               " + _("Wait for RPC server to start") + "\n";
    }
    strUsage += "  -rpcthreads=<n>        " + _("Set the number of threads to service RPC calls (default: 4)") + "\n";
    strUsage += "  -blocknotify=<cmd>     " + _("Execute command when the best block changes (%s in cmd is replaced by block hash)") + "\n";
    strUsage += "  -walletnotify=<cmd>    " + _("Execute command when a wallet transaction changes (%s in cmd is replaced by TxID)") + "\n";
    strUsage += "  -confchange            " + _("Require a confirmations for change (default: 0)") + "\n";
    strUsage += "  -minimizecoinage       " + _("Minimize weight consumption (experimental) (default: 0)") + "\n";
    strUsage += "  -alertnotify=<cmd>     " + _("Execute command when a relevant alert is received (%s in cmd is replaced by message)") + "\n";
    strUsage += "  -upgradewallet         " + _("Upgrade wallet to latest format") + "\n";
    strUsage += "  -createwalletbackups=<n> " + _("Number of automatic wallet backups (default: 10)") + "\n";
    strUsage += "  -keypool=<n>           " + _("Set key pool size to <n> (default: 100)") + "\n";
    strUsage += "  -rescan                " + _("Rescan the block chain for missing wallet transactions") + "\n";
    strUsage += "  -salvagewallet         " + _("Attempt to recover private keys from a corrupt wallet.dat") + "\n";
    strUsage += "  -checkblocks=<n>       " + _("How many blocks to check at startup (default: 500, 0 = all)") + "\n";
    strUsage += "  -checklevel=<n>        " + _("How thorough the block verification is (0-6, default: 1)") + "\n";
    strUsage += "  -loadblock=<file>      " + _("Imports blocks from external blk000?.dat file") + "\n";
    strUsage += "  -maxorphanblocks=<n>   " + strprintf(_("Keep at most <n> unconnectable blocks in memory (default: %u)"), DEFAULT_MAX_ORPHAN_BLOCKS) + "\n";
    strUsage += "  -backtoblock=<n>      " + _("Rollback local block chain to block height <n>") + "\n";

    strUsage += "\n" + _("Block creation options:") + "\n";
    strUsage += "  -blockminsize=<n>      "     + _("Set minimum block size in bytes (default: 0)") + "\n";
    strUsage += "  -blockmaxsize=<n>      "     + _("Set maximum block size in bytes (default: 250000)") + "\n";
    strUsage += "  -blockprioritysize=<n> "     + _("Set maximum size of high-priority/low-fee transactions in bytes (default: 27000)") + "\n";
    strUsage += "  -scaleblocksizeoptions=<n>"  + strprintf(_("Adaptively scale block size options (max, min, priority) (default: %d)"), DEFAULT_SCALE_BLOCK_SIZE_OPTIONS) + "\n";

    strUsage += "\n" + _("SSL options: (see the Bitcoin Wiki for SSL setup instructions)") + "\n";
    strUsage += "  -rpcssl                                  " + _("Use OpenSSL (https) for JSON-RPC connections") + "\n";
    strUsage += "  -rpcsslcertificatechainfile=<file.cert>  " + _("Server certificate file (default: server.cert)") + "\n";
    strUsage += "  -rpcsslprivatekeyfile=<file.pem>         " + _("Server private key (default: server.pem)") + "\n";
    strUsage += "  -rpcsslciphers=<ciphers>                 " + _("Acceptable ciphers (default: TLSv1.2+HIGH:TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!3DES:@STRENGTH)") + "\n";
    strUsage += "  -stakethreshold=<n> " + _("This will set the output size of your stakes to never be below this number (default: 100)") + "\n";

    strUsage += "\n" + _("Experimental feature toggle:") + "\n";
    strUsage += "  -liveforktoggle=<n> " + _("Toggle experimental features via block height testing fork, (example: -command=<fork_height>)") + "\n";

    strUsage += "\n" + _("XNode options:") + "\n";
    strUsage += "  -xnode=<n>            " + _("Enable the client to act as a xnode (0-1, default: 0)") + "\n";
    strUsage += "  -xnconf=<file>             " + _("Specify xnode configuration file (default: xnode.conf)") + "\n";
    strUsage += "  -xnconflock=<n>            " + _("Lock xnodes from xnode configuration file (default: 1)") + "\n";
    strUsage += "  -xnodeprivkey=<n>     " + _("Set the xnode private key") + "\n";
    strUsage += "  -xnodeaddr=<n>        " + _("Set external address:port to get to this xnode (example: address:port)") + "\n";
    strUsage += "  -xnodeminprotocol=<n> " + _("Ignore xnodes less than version (example: 61401; default : 0)") + "\n";

    strUsage += "\n" + _("Demi-node feature options:") + "\n";
    strUsage += "  -deminodes=<n> " + _("Toggle Demi-node features on/off, (0-1, default: 0") + "\n";
    strUsage += "  -demimaxdepth=<n> " + _("Set the maximum override depth for chain reorganization, (default: 0") + "\n";
    strUsage += "  -demilocksync=<n> " + _("Lock Demi-nodes to either allow or deny standard node sync failover, (0-1, default: 0") + "\n";
    strUsage += "  -demistrict=<n> " + _("Toggle using Demi-nodes exclusively to accept new blocks on/off, (0-1, default: 0") + "\n";
    strUsage += "  -demipeerlimit=<n> " + _("Allow/Deny blocks from peers using legacy clients/wallets, (0-1, default: 0") + "\n";
    strUsage += "  -demireorgtype=<n> " + _("Allow/Deny reorganize requests from peers as well as Demi-nodes, (0-1, default: 0") + "\n";

    strUsage += "\n" + _("Pubkey-Alias-Service feature options:") + "\n";
    strUsage += "  -paservice=<n> " + _("Toggle Pubkey-Alias-Service features on/off, (0-1, default: 0") + "\n";

    return strUsage;
}

/** Sanity checks
 *  Ensure that Bitcoin is running in a usable environment with all
 *  necessary library support.
 */
bool InitSanityCheck(void)
{
    if(!ECC_InitSanityCheck()) {
        InitError("OpenSSL appears to lack support for elliptic curve cryptography. For more "
                  "information, visit https://en.bitcoin.it/wiki/OpenSSL_and_EC_Libraries");
        return false;
    }

    // TODO: remaining sanity checks, see #4081

    return true;
}

/** Initialize bitcoin.
 *  @pre Parameters should be parsed and config file should be read.
 */
bool AppInit2(boost::thread_group& threadGroup)
{
    // ********************************************************* Step 1: setup
#ifdef _MSC_VER
    // Turn off Microsoft heap dump noise
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, CreateFileA("NUL", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0));
#endif
#if _MSC_VER >= 1400
    // Disable confusing "helpful" text message on abort, Ctrl-C
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#ifdef WIN32
    // Enable Data Execution Prevention (DEP)
    // Minimum supported OS versions: WinXP SP3, WinVista >= SP1, Win Server 2008
    // A failure is non-critical and needs no further attention!
#ifndef PROCESS_DEP_ENABLE
    // We define this here, because GCCs winbase.h limits this to _WIN32_WINNT >= 0x0601 (Windows 7),
    // which is not correct. Can be removed, when GCCs winbase.h is fixed!
#define PROCESS_DEP_ENABLE 0x00000001
#endif
    typedef BOOL (WINAPI *PSETPROCDEPPOL)(DWORD);
    PSETPROCDEPPOL setProcDEPPol = (PSETPROCDEPPOL)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "SetProcessDEPPolicy");
    if (setProcDEPPol != NULL) setProcDEPPol(PROCESS_DEP_ENABLE);
#endif
#ifndef WIN32
    umask(077);

    // Clean shutdown on SIGTERM
    struct sigaction sa;
    sa.sa_handler = HandleSIGTERM;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Reopen debug.log on SIGHUP
    struct sigaction sa_hup;
    sa_hup.sa_handler = HandleSIGHUP;
    sigemptyset(&sa_hup.sa_mask);
    sa_hup.sa_flags = 0;
    sigaction(SIGHUP, &sa_hup, NULL);
#endif

    // ********************************************************* Step 2: parameter interactions

    nNodeLifespan = GetArg("-addrlifespan", 7);
    fUseFastIndex = GetBoolArg("-fastindex", true);
    nMinerSleep = GetArg("-minersleep", 500);

    nDerivationMethodIndex = 0;

    if (!SelectParamsFromCommandLine()) {
        return InitError("Invalid combination of -testnet and -regtest.");
    }

    if (mapArgs.count("-bind")) {
        // when specifying an explicit binding address, you want to listen on it
        // even when -connect or -proxy is specified
        if (SoftSetBoolArg("-listen", true))
            LogPrintf("AppInit2 : parameter interaction: -bind set -> setting -listen=1\n");
    }

    // Process xnode config
    xnodeConfig.read(GetXNodeConfigFile());

    if (mapArgs.count("-connect") && mapMultiArgs["-connect"].size() > 0) {
        // when only connecting to trusted nodes, do not seed via DNS, or listen by default
        if (SoftSetBoolArg("-dnsseed", false))
            LogPrintf("AppInit2 : parameter interaction: -connect set -> setting -dnsseed=0\n");
        if (SoftSetBoolArg("-listen", false))
            LogPrintf("AppInit2 : parameter interaction: -connect set -> setting -listen=0\n");
    }

    if (mapArgs.count("-proxy")) {
        // to protect privacy, do not listen by default if a default proxy server is specified
        if (SoftSetBoolArg("-listen", false))
            LogPrintf("AppInit2 : parameter interaction: -proxy set -> setting -listen=0\n");
        // to protect privacy, do not discover addresses by default
        if (SoftSetBoolArg("-discover", false))
            LogPrintf("AppInit2 : parameter interaction: -proxy set -> setting -discover=0\n");
    }

    if (!GetBoolArg("-listen", true)) {
        // do not map ports or try to retrieve public IP when not listening (pointless)
        if (SoftSetBoolArg("-upnp", false))
            LogPrintf("AppInit2 : parameter interaction: -listen=0 -> setting -upnp=0\n");
        if (SoftSetBoolArg("-discover", false))
            LogPrintf("AppInit2 : parameter interaction: -listen=0 -> setting -discover=0\n");
    }

    if (mapArgs.count("-externalip")) {
        // if an explicit public IP is specified, do not try to find others
        if (SoftSetBoolArg("-discover", false))
            LogPrintf("AppInit2 : parameter interaction: -externalip set -> setting -discover=0\n");
    }

    if (GetBoolArg("-salvagewallet", false)) {
        // Rewrite just private keys: rescan to find transactions
        if (SoftSetBoolArg("-rescan", true))
            LogPrintf("AppInit2 : parameter interaction: -salvagewallet=1 -> setting -rescan=1\n");
    }

    // Process Espers config
    InitializeConfigFile();

    // ********************************************************* Step 3: parameter-to-internal-flags

    fDebug = !mapMultiArgs["-debug"].empty();
    // Special-case: if -debug=0/-nodebug is set, turn off debugging messages
    const vector<string>& categories = mapMultiArgs["-debug"];
    if (GetBoolArg("-nodebug", false) || find(categories.begin(), categories.end(), string("0")) != categories.end())
        fDebug = false;

    // Check for -debugnet (deprecated)
    if (GetBoolArg("-debugnet", false))
        InitWarning(_("Warning: Deprecated argument -debugnet ignored, use -debug=net"));
    // Check for -socks - as this is a privacy risk to continue, exit here
    if (mapArgs.count("-socks"))
        return InitError(_("Error: Unsupported argument -socks found. Setting SOCKS version isn't possible anymore, only SOCKS5 proxies are supported."));

    if (fDaemon)
        fServer = true;
    else
        fServer = GetBoolArg("-server", false);

    /* force fServer when running without GUI */
    if (!fHaveGUI)
        fServer = true;
    fPrintToConsole = GetBoolArg("-printtoconsole", false);
    fLogTimestamps = GetBoolArg("-logtimestamps", false);
#ifdef ENABLE_WALLET
    bool fDisableWallet = GetBoolArg("-disablewallet", false);
#endif

    if (mapArgs.count("-timeout"))
    {
        int nNewTimeout = GetArg("-timeout", 5000);
        if (nNewTimeout > 0 && nNewTimeout < 600000)
            nConnectTimeout = nNewTimeout;
    }

#ifdef ENABLE_WALLET
    if (mapArgs.count("-paytxfee"))
    {
        if (!ParseMoney(mapArgs["-paytxfee"], nTransactionFee))
            return InitError(strprintf(_("Invalid amount for -paytxfee=<amount>: '%s'"), mapArgs["-paytxfee"]));
        if (nTransactionFee > 0.25 * COIN)
            InitWarning(_("Warning: -paytxfee is set very high! This is the transaction fee you will pay if you send a transaction."));
    }
#endif

    fConfChange = GetBoolArg("-confchange", false);
    fMinimizeCoinAge = GetBoolArg("-minimizecoinage", false);

#ifdef ENABLE_WALLET
    if (mapArgs.count("-mininput"))
    {
        if (!ParseMoney(mapArgs["-mininput"], nMinimumInputValue))
            return InitError(strprintf(_("Invalid amount for -mininput=<amount>: '%s'"), mapArgs["-mininput"]));
    }
#endif

    // ********************************************************* Step 4: application initialization: dir lock, daemonize, pidfile, debug log

    // Sanity check
    if (!InitSanityCheck())
        return InitError(_("Initialization sanity check failed. Espers is shutting down."));

    std::string strDataDir = GetDataDir().string();
#ifdef ENABLE_WALLET
    std::string strWalletFileName = GetArg("-wallet", "wallet.dat");

    // strWalletFileName must be a plain filename without a directory
    if (strWalletFileName != boost::filesystem::basename(strWalletFileName) + boost::filesystem::extension(strWalletFileName))
        return InitError(strprintf(_("Wallet %s resides outside data directory %s."), strWalletFileName, strDataDir));
#endif
    // Make sure only a single Bitcoin process is using the data directory.
    boost::filesystem::path pathLockFile = GetDataDir() / ".lock";
    FILE* file = fopen(pathLockFile.string().c_str(), "a"); // empty lock file; created if it doesn't exist.
    if (file) fclose(file);
    static boost::interprocess::file_lock lock(pathLockFile.string().c_str());
    if (!lock.try_lock())
        return InitError(strprintf(_("Cannot obtain a lock on data directory %s. Espers is probably already running."), strDataDir));

    if (GetBoolArg("-shrinkdebugfile", !fDebug))
        ShrinkDebugFile();
    LogPrintf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    LogPrintf("Espers version %s (%s)\n", FormatFullVersion(), CLIENT_DATE);
    LogPrintf("Using OpenSSL version %s\n", SSLeay_version(SSLEAY_VERSION));
    if (!fLogTimestamps)
        LogPrintf("Startup time: %s\n", DateTimeStrFormat("%x %H:%M:%S", GetTime()));
    LogPrintf("Default data directory %s\n", GetDefaultDataDir().string());
    LogPrintf("Used data directory %s\n", strDataDir);
    std::ostringstream strErrors;

    if (mapArgs.count("-xnodepaymentskey")) // xnode payments priv key
    {
        if (!xnodePayments.SetPrivKey(GetArg("-xnodepaymentskey", "")))
            return InitError(_("Unable to sign xnode payment winner, wrong key?"));
        if (!sporkManager.SetPrivKey(GetArg("-xnodepaymentskey", "")))
            return InitError(_("Unable to sign spork message, wrong key?"));
    }

    //ignore xnodes below protocol version
    nXNodeMinProtocol = GetArg("-xnodeminprotocol", MIN_PEER_PROTO_VERSION);

    //ignore pubkeyaliasservices below protocol version
    nPubkeyaliasserviceMinProtocol = GetArg("-pasminprotocol", MIN_PASERVICE_PROTO_VERSION);

    if (fDaemon)
        fprintf(stdout, "Espers server starting\n");

    int64_t nStart;

    // ********************************************************* Step 5: verify database integrity
#ifdef ENABLE_WALLET
    if (!fDisableWallet) {

        boost::filesystem::path backupDir = GetDataDir() / "backups";
        if (!boost::filesystem::exists(backupDir))
        {
            // Always create backup folder to not confuse the operating system's file browser
            boost::filesystem::create_directories(backupDir);
        }
        nWalletBackups = GetArg("-createwalletbackups", 10);
        nWalletBackups = std::max(0, std::min(10, nWalletBackups));
        if(nWalletBackups > 0)
        {
            if (boost::filesystem::exists(backupDir))
            {
                // Create backup of the wallet
                std::string dateTimeStr = DateTimeStrFormat(".%Y-%m-%d-%H.%M", GetTime());
                std::string backupPathStr = backupDir.string();
                backupPathStr += "/" + strWalletFileName;
                std::string sourcePathStr = GetDataDir().string();
                sourcePathStr += "/" + strWalletFileName;
                boost::filesystem::path sourceFile = sourcePathStr;
                boost::filesystem::path backupFile = backupPathStr + dateTimeStr;
                sourceFile.make_preferred();
                backupFile.make_preferred();
                try {
                    boost::filesystem::copy_file(sourceFile, backupFile);
                    LogPrintf("Creating backup of %s -> %s\n", sourceFile, backupFile);
                } catch(boost::filesystem::filesystem_error &error) {
                    LogPrintf("Failed to create backup %s\n", error.what());
                }
                // Keep only the last 10 backups, including the new one of course
                typedef std::multimap<std::time_t, boost::filesystem::path> folder_set_t;
                folder_set_t folder_set;
                boost::filesystem::directory_iterator end_iter;
                boost::filesystem::path backupFolder = backupDir.string();
                backupFolder.make_preferred();
                // Build map of backup files for current(!) wallet sorted by last write time
                boost::filesystem::path currentFile;
                for (boost::filesystem::directory_iterator dir_iter(backupFolder); dir_iter != end_iter; ++dir_iter)
                {
                    // Only check regular files
                    if ( boost::filesystem::is_regular_file(dir_iter->status()))
                    {
                        currentFile = dir_iter->path().filename();
                        // Only add the backups for the current wallet, e.g. wallet.dat.*
                        if(currentFile.string().find(strWalletFileName) != string::npos)
                        {
                            folder_set.insert(folder_set_t::value_type(boost::filesystem::last_write_time(dir_iter->path()), *dir_iter));
                        }
                    }
                }
                // Loop backward through backup files and keep the N newest ones (1 <= N <= 10)
                int counter = 0;
                BOOST_REVERSE_FOREACH(PAIRTYPE(const std::time_t, boost::filesystem::path) file, folder_set)
                {
                    counter++;
                    if (counter > nWalletBackups)
                    {
                        // More than nWalletBackups backups: delete oldest one(s)
                        try {
                            boost::filesystem::remove(file.second);
                            LogPrintf("Old backup deleted: %s\n", file.second);
                        } catch(boost::filesystem::filesystem_error &error) {
                            LogPrintf("Failed to delete backup %s\n", error.what());
                        }
                    }
                }
            }
        }


        uiInterface.InitMessage(_("Verifying database integrity..."));

        if (!bitdb.Open(GetDataDir()))
        {
            // try moving the database env out of the way
            boost::filesystem::path pathDatabase = GetDataDir() / "database";
            boost::filesystem::path pathDatabaseBak = GetDataDir() / strprintf("database.%d.bak", GetTime());
            try {
                boost::filesystem::rename(pathDatabase, pathDatabaseBak);
                LogPrintf("Moved old %s to %s. Retrying.\n", pathDatabase.string(), pathDatabaseBak.string());
            } catch(boost::filesystem::filesystem_error &error) {
                 // failure is ok (well, not really, but it's not worse than what we started with)
            }

            // try again
            if (!bitdb.Open(GetDataDir())) {
                // if it still fails, it probably means we can't even create the database env
                string msg = strprintf(_("Error initializing wallet database environment %s!"), strDataDir);
                return InitError(msg);
            }
        }

        if (GetBoolArg("-salvagewallet", false))
        {
            // Recover readable keypairs:
            if (!CWalletDB::Recover(bitdb, strWalletFileName, true))
                return false;
        }

        if (boost::filesystem::exists(GetDataDir() / strWalletFileName))
        {
            CDBEnv::VerifyResult r = bitdb.Verify(strWalletFileName, CWalletDB::Recover);
            if (r == CDBEnv::RECOVER_OK)
            {
                string msg = strprintf(_("Warning: wallet.dat corrupt, data salvaged!"
                                         " Original wallet.dat saved as wallet.{timestamp}.bak in %s; if"
                                         " your balance or transactions are incorrect you should"
                                         " restore from a backup."), strDataDir);
                InitWarning(msg);
            }
            if (r == CDBEnv::RECOVER_FAIL)
                return InitError(_("wallet.dat corrupt, salvage failed"));
        }

    } // (!fDisableWallet)
#endif // ENABLE_WALLET
    // ********************************************************* Step 6: network initialization

    RegisterNodeSignals(GetNodeSignals());

    // format user agent, check total size
    strSubVersion = FormatSubVersion(CLIENT_NAME, CLIENT_VERSION, mapMultiArgs.count("-uacomment") ? mapMultiArgs["-uacomment"] : std::vector<string>());
    if (strSubVersion.size() > MAX_SUBVERSION_LENGTH) {
        return InitError(strprintf("Total length of network version string %i exceeds maximum of %i characters. Reduce the number and/or size of uacomments.",
            strSubVersion.size(), MAX_SUBVERSION_LENGTH));
    }
    
    if (mapArgs.count("-onlynet")) {
        std::set<enum Network> nets;
        BOOST_FOREACH(std::string snet, mapMultiArgs["-onlynet"]) {
            enum Network net = ParseNetwork(snet);
            if (net == NET_UNROUTABLE)
                return InitError(strprintf(_("Unknown network specified in -onlynet: '%s'"), snet));
            nets.insert(net);
        }
        for (int n = 0; n < NET_MAX; n++) {
            enum Network net = (enum Network)n;
            if (!nets.count(net))
                SetLimited(net);
        }
    } else {
        SetReachable(NET_IPV4);
        SetReachable(NET_IPV6);
    }

    CService addrProxy;
    bool fProxy = false;
    if (mapArgs.count("-proxy")) {
        addrProxy = CService(mapArgs["-proxy"], 9050);
        if (!addrProxy.IsValid())
            return InitError(strprintf(_("Invalid -proxy address: '%s'"), mapArgs["-proxy"]));

        if (!IsLimited(NET_IPV4))
            SetProxy(NET_IPV4, addrProxy);
        if (!IsLimited(NET_IPV6))
            SetProxy(NET_IPV6, addrProxy);
        SetNameProxy(addrProxy);
        fProxy = true;
    }

    // -tor can override normal proxy, -notor disables tor entirely
    if (!(mapArgs.count("-tor") && mapArgs["-tor"] == "0") && (fProxy || mapArgs.count("-tor"))) {
        CService addrOnion;
        if (!mapArgs.count("-tor"))
            addrOnion = addrProxy;
        else
            addrOnion = CService(mapArgs["-tor"], 9050);
        if (!addrOnion.IsValid())
            return InitError(strprintf(_("Invalid -tor address: '%s'"), mapArgs["-tor"]));
        SetProxy(NET_TOR, addrOnion);
        SetReachable(NET_TOR);
    }

    // see Step 2: parameter interactions for more information about these
    fNoListen = !GetBoolArg("-listen", true);
    fDiscover = GetBoolArg("-discover", true);
    fNameLookup = GetBoolArg("-dns", true);

    bool fBound = false;
    if (!fNoListen)
    {
        std::string strError;
        if (mapArgs.count("-bind")) {
            BOOST_FOREACH(std::string strBind, mapMultiArgs["-bind"]) {
                CService addrBind;
                if (!Lookup(strBind.c_str(), addrBind, GetListenPort(), false))
                    return InitError(strprintf(_("Cannot resolve -bind address: '%s'"), strBind));
                fBound |= Bind(addrBind);
            }
        } else {
            struct in_addr inaddr_any;
            inaddr_any.s_addr = INADDR_ANY;
            if (!IsLimited(NET_IPV6))
                fBound |= Bind(CService(in6addr_any, GetListenPort()), false);
            if (!IsLimited(NET_IPV4))
                fBound |= Bind(CService(inaddr_any, GetListenPort()), !fBound);
        }
        if (!fBound)
            return InitError(_("Failed to listen on any port. Use -listen=0 if you want this."));
    }

    if (mapArgs.count("-externalip"))
    {
        BOOST_FOREACH(string strAddr, mapMultiArgs["-externalip"]) {
            CService addrLocal(strAddr, GetListenPort(), fNameLookup);
            if (!addrLocal.IsValid())
                return InitError(strprintf(_("Cannot resolve -externalip address: '%s'"), strAddr));
            AddLocal(CService(strAddr, GetListenPort(), fNameLookup), LOCAL_MANUAL);
        }
    }

#ifdef ENABLE_WALLET
    if (mapArgs.count("-reservebalance")) // ppcoin: reserve balance amount
    {
        if (!ParseMoney(mapArgs["-reservebalance"], nReserveBalance))
        {
            InitError(_("Invalid amount for -reservebalance=<amount>"));
            return false;
        }
    }
#endif

    BOOST_FOREACH(string strDest, mapMultiArgs["-seednode"])
        AddOneShot(strDest);

    // ********************************************************* Step 7: load blockchain

    if (GetBoolArg("-loadblockindextest", false))
    {
        CTxDB txdb("r");
        txdb.LoadBlockIndex();
        PrintBlockTree();
        return false;
    }

    uiInterface.InitMessage(_("Loading block index..."));

    nStart = GetTimeMillis();
    if (!LoadBlockIndex())
        return InitError(_("Error loading block database"));


    // as LoadBlockIndex can take several minutes, it's possible the user
    // requested to kill bitcoin-qt during the last operation. If so, exit.
    // As the program has not fully started yet, Shutdown() is possibly overkill.
    if (fRequestShutdown)
    {
        LogPrintf("Shutdown requested. Exiting.\n");
        return false;
    }
    LogPrintf(" block index %15dms\n", GetTimeMillis() - nStart);

    if (GetBoolArg("-printblockindex", false) || GetBoolArg("-printblocktree", false))
    {
        PrintBlockTree();
        return false;
    }

    if (mapArgs.count("-printblock"))
    {
        string strMatch = mapArgs["-printblock"];
        int nFound = 0;
        for (map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.begin(); mi != mapBlockIndex.end(); ++mi)
        {
            uint256 hash = (*mi).first;
            if (strncmp(hash.ToString().c_str(), strMatch.c_str(), strMatch.size()) == 0)
            {
                CBlockIndex* pindex = (*mi).second;
                CBlock block;
                block.ReadFromDisk(pindex);
                block.BuildMerkleTree();
                LogPrintf("%s\n", block.ToString());
                nFound++;
            }
        }
        if (nFound == 0)
            LogPrintf("No blocks matching %s were found\n", strMatch);
        return false;
    }

    if (mapArgs.count("-backtoblock"))
    {
        strRollbackToBlock = GetArg("-backtoblock", "");
        LogPrintf("Rolling blocks back...\n");
        if(!strRollbackToBlock.empty()){
            nNewHeight = GetArg("-backtoblock", (uint64_t)"");
            fRollbacktoBlock = true;
            CBlockIndex* pindex = pindexBest;
            int pindexGap = (pindex->nHeight - nNewHeight);
            while (pindex != NULL && pindex->nHeight > nNewHeight)
            {
                ostringstream osHeight;
                osHeight << pindex->nHeight;
                string strHeight = osHeight.str();
                uiInterface.InitMessage(strprintf("Rolling blocks back... %s to %i \n", strHeight, nNewHeight));
                pindex = pindex->pprev;
            }
            if (pindex != NULL)
            {
                LogPrintf("Back to block index %d\n", nNewHeight);
                uiInterface.InitMessage(strprintf("Reorganizing %u blocks, please wait... \n", pindexGap));
                CTxDB txdbAddr("rw");
                CBlock block;
                block.ReadFromDisk(pindex);
                block.SetBestChain(txdbAddr, pindex);
            }
            else
            {
                LogPrintf("Block %d not found\n", nNewHeight);
            }
        }
        else
        {
            LogPrintf("Back to block was requested but not defined!\n");
        }
    }

    // ********************************************************* Step 8: load wallet
#ifdef ENABLE_WALLET
    if (fDisableWallet) {
        pwalletMain = NULL;
        LogPrintf("Wallet disabled!\n");
    } else {
        uiInterface.InitMessage(_("Loading wallet..."));

        nStart = GetTimeMillis();
        bool fFirstRun = true;
        pwalletMain = new CWallet(strWalletFileName);
        DBErrors nLoadWalletRet = pwalletMain->LoadWallet(fFirstRun);
        if (nLoadWalletRet != DB_LOAD_OK)
        {
            if (nLoadWalletRet == DB_CORRUPT)
                strErrors << _("Error loading wallet.dat: Wallet corrupted") << "\n";
            else if (nLoadWalletRet == DB_NONCRITICAL_ERROR)
            {
                string msg(_("Warning: error reading wallet.dat! All keys read correctly, but transaction data"
                             " or address book entries might be missing or incorrect."));
                InitWarning(msg);
            }
            else if (nLoadWalletRet == DB_TOO_NEW)
                strErrors << _("Error loading wallet.dat: Wallet requires newer version of Espers") << "\n";
            else if (nLoadWalletRet == DB_NEED_REWRITE)
            {
                strErrors << _("Wallet needed to be rewritten: restart Espers to complete") << "\n";
                LogPrintf("%s", strErrors.str());
                return InitError(strErrors.str());
            }
            else
                strErrors << _("Error loading wallet.dat") << "\n";
        }

        if (GetBoolArg("-upgradewallet", fFirstRun))
        {
            int nMaxVersion = GetArg("-upgradewallet", 0);
            if (nMaxVersion == 0) // the -upgradewallet without argument case
            {
                LogPrintf("Performing wallet upgrade to %i\n", FEATURE_LATEST);
                nMaxVersion = CLIENT_VERSION;
                pwalletMain->SetMinVersion(FEATURE_LATEST); // permanently upgrade the wallet immediately
            }
            else
                LogPrintf("Allowing wallet upgrade up to %i\n", nMaxVersion);
            if (nMaxVersion < pwalletMain->GetVersion())
                strErrors << _("Cannot downgrade wallet") << "\n";
            pwalletMain->SetMaxVersion(nMaxVersion);
        }

        if (fFirstRun)
        {
            // Create new keyUser and set as default key
            RandAddSeedPerfmon();

            CPubKey newDefaultKey;
            if (pwalletMain->GetKeyFromPool(newDefaultKey)) {
                pwalletMain->SetDefaultKey(newDefaultKey);
                if (!pwalletMain->SetAddressBookName(pwalletMain->vchDefaultKey.GetID(), ""))
                    strErrors << _("Cannot write default address") << "\n";
            }

            pwalletMain->SetBestChain(CBlockLocator(pindexBest));
        }

        LogPrintf("%s", strErrors.str());
        LogPrintf(" wallet      %15dms\n", GetTimeMillis() - nStart);

        RegisterWallet(pwalletMain);

        CBlockIndex *pindexRescan = pindexBest;
        if (GetBoolArg("-rescan", false))
            pindexRescan = pindexGenesisBlock;
        else
        {
            CWalletDB walletdb(strWalletFileName);
            CBlockLocator locator;
            if (walletdb.ReadBestBlock(locator))
                pindexRescan = locator.GetBlockIndex();
            else
                pindexRescan = pindexGenesisBlock;
        }
        if (pindexBest != pindexRescan && pindexBest && pindexRescan && pindexBest->nHeight > pindexRescan->nHeight)
        {
            uiInterface.InitMessage(_("Rescanning..."));
            LogPrintf("Rescanning last %i blocks (from block %i)...\n", pindexBest->nHeight - pindexRescan->nHeight, pindexRescan->nHeight);
            nStart = GetTimeMillis();
            pwalletMain->ScanForWalletTransactions(pindexRescan, true);
            LogPrintf(" rescan      %15dms\n", GetTimeMillis() - nStart);
            pwalletMain->SetBestChain(CBlockLocator(pindexBest));
            nWalletDBUpdated++;
        }
    } // (!fDisableWallet)
#else // ENABLE_WALLET
    LogPrintf("No wallet compiled in!\n");
#endif // !ENABLE_WALLET
    // ********************************************************* Step 9: import blocks

    std::vector<boost::filesystem::path> vImportFiles;
    if (mapArgs.count("-loadblock"))
    {
        BOOST_FOREACH(string strFile, mapMultiArgs["-loadblock"])
            vImportFiles.push_back(strFile);
    }
    threadGroup.create_thread(boost::bind(&ThreadImport, vImportFiles));

    // ********************************************************* Step 10: load peers

    uiInterface.InitMessage(_("Loading addresses..."));

    nStart = GetTimeMillis();

    {
        CAddrDB adb;
        if (!adb.Read(addrman))
            LogPrintf("Invalid or missing peers.dat; recreating\n");
    }

    LogPrintf("Loaded %i addresses from peers.dat  %dms\n",
           addrman.size(), GetTimeMillis() - nStart);

    // ********************************************************* Step 10.6: startup pubkey alias service

    // Check for Pubkey Alias Service toggle
    uiInterface.InitMessage(_("Checking Pubkey-Alias-Service feature toggle..."));
    fPubkeyAliasService = GetBoolArg("-paservice", false);
    LogPrintf("Checking for Pubkey-Alias-Service feature toggle...\n");
    if(fPubkeyAliasService) {
        LogPrintf("Continuing with Pubkey-Alias-Service ENABLED\n");
    } else {
        // Pubkey-Alias-Service disabled
        LogPrintf("Continuing with Pubkey-Alias-Service DISABLED\n");
    }

    // ********************************************************* Step 11: start node

    if (!CheckDiskSpace())
        return false;

    if (!strErrors.str().empty())
        return InitError(strErrors.str());

    uiInterface.InitMessage(_("Loading xnode cache..."));

        CXNodeDB xndb;
        CXNodeDB::ReadResult readResult = xndb.Read(xnodeman);
        if (readResult == CXNodeDB::FileError)
            LogPrintf("Missing xnode cache file - xnodelist.dat, will try to recreate\n");
        else if (readResult != CXNodeDB::Ok)
        {
            LogPrintf("Error reading xnodelist.dat: ");
            if(readResult == CXNodeDB::IncorrectFormat)
                LogPrintf("magic is ok but data has invalid format, will try to recreate\n");
            else
                LogPrintf("file format is unknown or invalid, please fix it manually\n");
        }

        uiInterface.InitMessage(_("Loading xnode payment cache..."));

        CXNodePaymentDB xnoderewards;
        CXNodePaymentDB::ReadResult readResult2 = xnoderewards.Read(xnodePayments);

        if (readResult2 == CXNodePaymentDB::FileError)
            LogPrintf("Missing XNode payment cache - xnoderewards.dat, will try to recreate\n");
        else if (readResult2 != CXNodePaymentDB::Ok)
        {
            LogPrintf("Error reading xnoderewards.dat: ");
            if(readResult2 == CXNodePaymentDB::IncorrectFormat)
                LogPrintf("magic is ok but data has invalid format, will try to recreate\n");
            else
                LogPrintf("file format is unknown or invalid, please fix it manually\n");
        }


        fXnode = GetBoolArg("-xnode", false);
        if(fXnode) {
            LogPrintf("IS XNODE\n");
            strXnodeAddr = GetArg("-xnodeaddr", "");

            LogPrintf(" addr %s\n", strXnodeAddr.c_str());

            if(!strXnodeAddr.empty()){
                CService addrTest = CService(strXnodeAddr, fNameLookup);
                if (!addrTest.IsValid()) {
                    return InitError("Invalid -xnodeaddr address: " + strXnodeAddr);
                }
            }

            strXnodePrivKey = GetArg("-xnodeprivkey", "");
            if(!strXnodePrivKey.empty()){
                std::string errorMessage;

                CKey key;
                CPubKey pubkey;

                if(!xnodeEngineSigner.SetKey(strXnodePrivKey, errorMessage, key, pubkey))
                {
                    return InitError(_("Invalid xnodeprivkey. Please see documenation."));
                }

                activeXNode.pubKeyXNode = pubkey;

            } else {
                return InitError(_("You must specify a xnodeprivkey in the configuration. Please see documentation for help."));
            }

            activeXNode.ManageStatus();
        }

        if(GetBoolArg("-xnconflock", true)) {
            LogPrintf("Locking XNodes:\n");
            uint256 xnTxHash;
            BOOST_FOREACH(CXNodeConfig::CXNodeEntry xne, xnodeConfig.getEntries()) {
                LogPrintf("  %s %s\n", xne.getTxHash(), xne.getOutputIndex());
                xnTxHash.SetHex(xne.getTxHash());
                COutPoint outpoint = COutPoint(xnTxHash, boost::lexical_cast<unsigned int>(xne.getOutputIndex()));
                pwalletMain->LockCoin(outpoint);
            }
        }

        threadGroup.create_thread(boost::bind(&ThreadCheckXNodeEnginePool));

    // Check toggle switch for experimental feature testing fork
    uiInterface.InitMessage(_("Checking experimental feature toggle..."));
    strLiveForkToggle = GetArg("-liveforktoggle", "");
    LogPrintf("Checking for experimental testing feature fork toggle...\n");
    if(!strLiveForkToggle.empty()){
        LogPrintf("Verifying height selection for experimental testing feature fork toggle...\n");
        std::istringstream(strLiveForkToggle) >> nLiveForkToggle;
        if(nLiveForkToggle == 0) {
            LogPrintf("Continuing with fork toggle manually disabled by user...\n");
        } else if(nLiveForkToggle < nBestHeight) {
            return InitError(_("Invalid experimental testing feature fork toggle, please select a higher block than currently sync'd height\n"));
        } else {
            LogPrintf("Continuing with fork toggle set for block: %s | Happy testing!\n", strLiveForkToggle.c_str());
        }

    } else {
        nLiveForkToggle = 0;
        LogPrintf("No experimental testing feature fork toggle detected... skipping...\n");
    }

    // Check for Demi-node toggle
    uiInterface.InitMessage(_("Checking Demi-node feature toggle..."));
    fDemiNodes = GetBoolArg("-deminodes", false);
    LogPrintf("Checking for Demi-nodes feature toggle...\n");// BLOCK_REORG_OVERRIDE_DEPTH
    if(fDemiNodes) {
        // Set Demi-node values
        uiInterface.InitMessage(_("Configuring Demi-node systems..."));
        std::string strOverrideDepth = GetArg("-demimaxdepth", "");
        std::string strReorganizeType = GetArg("-demireorgtype", "");
        if(!strOverrideDepth.empty()) {
            std::istringstream(strOverrideDepth) >> BLOCK_REORG_OVERRIDE_DEPTH;
            if(BLOCK_REORG_OVERRIDE_DEPTH == 0) {
                LogPrintf("Continuing with Demi-node depth override manually disabled by user...\n");
            } else if(BLOCK_REORG_OVERRIDE_DEPTH < 0) {
                return InitError(_("Invalid Demi-node depth override, selected value must be higher than Zero!\n"));
            } else {
                BLOCK_REORG_THRESHOLD = (BLOCK_REORG_MAX_DEPTH + BLOCK_REORG_OVERRIDE_DEPTH);
                LogPrintf("Continuing with Demi-node depth override height of: %s\n", strOverrideDepth.c_str());
            }
        } if(!strReorganizeType.empty()) {
            std::istringstream(strReorganizeType) >> PEER_REORG_TYPE;
            if(PEER_REORG_TYPE == 0) {
                LogPrintf("Continuing with Demi-node ONLY reorganize authorization...\n");
            } else if(PEER_REORG_TYPE < 0 || PEER_REORG_TYPE > 1) {
                return InitError(_("Invalid Demi-node reorganize peer type selected, value must be either 0 or 1\n"));
            } else {
                LogPrintf("Continuing with Demi-node AND peer reorganize authorization\n");
            }
        }
    } else {
        // Demi-nodes disabled
        LogPrintf("No Demi-node features selected... skipping...\n");
    }

    // Check/Create 4k support config file for Qt
    if(fHaveGUI) {
        ReadDPIConfigFile();
    }

    RandAddSeedPerfmon();

    //// debug print
    LogPrintf("mapBlockIndex.size() = %u\n",   mapBlockIndex.size());
    LogPrintf("nBestHeight = %d\n",                   nBestHeight);
#ifdef ENABLE_WALLET
    LogPrintf("setKeyPool.size() = %u\n",      pwalletMain ? pwalletMain->setKeyPool.size() : 0);
    LogPrintf("mapWallet.size() = %u\n",       pwalletMain ? pwalletMain->mapWallet.size() : 0);
    LogPrintf("mapAddressBook.size() = %u\n",  pwalletMain ? pwalletMain->mapAddressBook.size() : 0);
#endif

    StartNode(threadGroup);
#ifdef ENABLE_WALLET
    // InitRPCMining is needed here so getwork/getblocktemplate in the GUI debug console works properly.
    InitRPCMining();
#endif
    if (fServer)
        StartRPCThreads();

#ifdef ENABLE_WALLET
    // Mine proof-of-stake blocks in the background
    if (!GetBoolArg("-staking", true)) {
        LogPrintf("Staking disabled\n");
    }
    else if (pwalletMain) {
        threadGroup.create_thread(boost::bind(&ThreadStakeMiner, pwalletMain));
    }

    if(pwalletMain->IsLocked()) {
        // Toggle wallet lock status
        settingsStatus = true;
    } else {
        // Toggle wallet lock status
        settingsStatus = false;
    }
#endif

    // ********************************************************* Step 12: finished

    uiInterface.InitMessage(_("Done loading"));

#ifdef Q_OS_WIN
    //TODO: CORRECT THIS
    QString runcheck = QDir::currentPath() + "/chk.file";
    QFile rcheck(runcheck);
    if(rcheck.open(QIODevice::ReadWrite))
    {
    QTextStream saturate(&rcheck);
    saturate << "Temp File that flags launcher" << endl;
    }
#endif

#ifdef ENABLE_WALLET
    if (pwalletMain) {
        // Add wallet transactions that aren't already in a block to mapTransactions
        pwalletMain->ReacceptWalletTransactions();

        // Run a thread to flush wallet periodically
        threadGroup.create_thread(boost::bind(&ThreadFlushWalletDB, boost::ref(pwalletMain->strWalletFile)));
    }
#endif

    return !fRequestShutdown;
}
