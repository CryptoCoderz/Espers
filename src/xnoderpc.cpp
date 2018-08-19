// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Darkcoin developers
// Copyright (c) 2017-2018 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain.h"
#include "db.h"
#include "init.h"
#include "xnodestart.h"
#include "xnodemngr.h"
#include "xnodesettings.h"
#include "rpcserver.h"
#include <boost/lexical_cast.hpp>
#include "util.h"

#include <fstream>
using namespace json_spirit;
using namespace std;

Value xnode(const Array& params, bool fHelp)
{
    string strCommand;
    if (params.size() >= 1)
        strCommand = params[0].get_str();

    if (fHelp  ||
        (strCommand != "count" && strCommand != "current" && strCommand != "debug" && strCommand != "genkey" && strCommand != "enforce" && strCommand != "list" && strCommand != "list-conf"
            && strCommand != "start" && strCommand != "start-alias" && strCommand != "start-many" && strCommand != "status" && strCommand != "stop" && strCommand != "stop-alias"
                && strCommand != "stop-many" && strCommand != "winners" && strCommand != "connect" && strCommand != "outputs" && strCommand != "vote-many" && strCommand != "vote"))
        throw runtime_error(
                "xnode \"command\"... ( \"passphrase\" )\n"
                "Set of commands to execute xnode related actions\n"
                "\nArguments:\n"
                "1. \"command\"        (string or set of strings, required) The command to execute\n"
                "2. \"passphrase\"     (string, optional) The wallet passphrase\n"
                "\nAvailable commands:\n"
                "  count        - Print number of all known xnodes (optional: 'enabled', 'both')\n"
                "  current      - Print info on current xnode winner\n"
                "  debug        - Print xnode status\n"
                "  genkey       - Generate new xnodeprivkey\n"
                "  enforce      - Enforce xnode payments\n"
                "  list         - Print list of all known xnodes (see xnodelist for more info)\n"
                "  list-conf    - Print xnode.conf in JSON format\n"
                "  outputs      - Print xnode compatible outputs\n"
                "  start        - Start xnode configured in XNode.conf\n"
                "  start-alias  - Start single xnode by assigned alias configured in xnode.conf\n"
                "  start-many   - Start all xnodes configured in xnode.conf\n"
                "  status       - Print xnode status information\n"
                "  stop         - Stop xnode configured in XNode.conf\n"
                "  stop-alias   - Stop single xnode by assigned alias configured in xnode.conf\n"
                "  stop-many    - Stop all xnodes configured in xnode.conf\n"
                "  winners      - Print list of xnode winners\n"
                "  vote-many    - Vote on a XNode initiative\n"
                "  vote         - Vote on a XNode initiative\n"
                );

    if (strCommand == "stop")
    {
        if(!fXnode) return "you must set xnode=1 in the configuration";

        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 2){
                strWalletPass = params[1].get_str().c_str();
            } else {
                throw runtime_error(
                    "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        std::string errorMessage;
        if(!activeXNode.StopXnode(errorMessage)) {
            return "stop failed: " + errorMessage;
        }
        pwalletMain->Lock();

        if(activeXNode.status == XNODE_STOPPED) return "successfully stopped xnode";
        if(activeXNode.status == XNODE_NOT_CAPABLE) return "not capable xnode";

        return "unknown";
    }

    if (strCommand == "stop-alias")
    {
        if (params.size() < 2){
            throw runtime_error(
            "command needs at least 2 parameters\n");
        }

        std::string alias = params[1].get_str().c_str();

        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 3){
                strWalletPass = params[2].get_str().c_str();
            } else {
                throw runtime_error(
                "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        bool found = false;

        Object statusObj;
        statusObj.push_back(Pair("alias", alias));

        BOOST_FOREACH(CXNodeConfig::CXNodeEntry xne, xnodeConfig.getEntries()) {
            if(xne.getAlias() == alias) {
                found = true;
                std::string errorMessage;
                bool result = activeXNode.StopXnode(xne.getIp(), xne.getPrivKey(), errorMessage);

                statusObj.push_back(Pair("result", result ? "successful" : "failed"));
                if(!result) {
                    statusObj.push_back(Pair("errorMessage", errorMessage));
                }
                break;
            }
        }

        if(!found) {
            statusObj.push_back(Pair("result", "failed"));
            statusObj.push_back(Pair("errorMessage", "could not find alias in config. Verify with list-conf."));
        }

        pwalletMain->Lock();
        return statusObj;
    }

    if (strCommand == "stop-many")
    {
        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 2){
                strWalletPass = params[1].get_str().c_str();
            } else {
                throw runtime_error(
                "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        int total = 0;
        int successful = 0;
        int fail = 0;


        Object resultsObj;

        BOOST_FOREACH(CXNodeConfig::CXNodeEntry xne, xnodeConfig.getEntries()) {
            total++;

            std::string errorMessage;
            bool result = activeXNode.StopXnode(xne.getIp(), xne.getPrivKey(), errorMessage);

            Object statusObj;
            statusObj.push_back(Pair("alias", xne.getAlias()));
            statusObj.push_back(Pair("result", result ? "successful" : "failed"));

            if(result) {
                successful++;
            } else {
                fail++;
                statusObj.push_back(Pair("errorMessage", errorMessage));
            }

            resultsObj.push_back(Pair("status", statusObj));
        }
        pwalletMain->Lock();

        Object returnObj;
        returnObj.push_back(Pair("overall", "Successfully stopped " + boost::lexical_cast<std::string>(successful) + " xnodes, failed to stop " +
                boost::lexical_cast<std::string>(fail) + ", total " + boost::lexical_cast<std::string>(total)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;

    }

    if (strCommand == "list")
    {
        Array newParams(params.size() - 1);
        std::copy(params.begin() + 1, params.end(), newParams.begin());
        return xnodelist(newParams, fHelp);
    }

    if (strCommand == "count")
    {
        if (params.size() > 2){
            throw runtime_error(
                "too many parameters\n");
        }

        if (params.size() == 2)
        {
            if(params[1] == "enabled") return xnodeman.CountEnabled();
            if(params[1] == "both") return boost::lexical_cast<std::string>(xnodeman.CountEnabled()) + " / " + boost::lexical_cast<std::string>(xnodeman.size());
        }
        return xnodeman.size();
    }

    if (strCommand == "start")
    {
        if(!fXnode) return "you must set xnode=1 in the configuration";

        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 2){
                strWalletPass = params[1].get_str().c_str();
            } else {
                throw runtime_error(
                    "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        if(activeXNode.status != XNODE_REMOTELY_ENABLED && activeXNode.status != XNODE_IS_CAPABLE){
            activeXNode.status = XNODE_NOT_PROCESSED; // TODO: consider better way
            std::string errorMessage;
            activeXNode.ManageStatus();
            pwalletMain->Lock();
        }

        if(activeXNode.status == XNODE_REMOTELY_ENABLED) return "xnode started remotely";
        if(activeXNode.status == XNODE_INPUT_TOO_NEW) return "xnode input must have at least 15 confirmations";
        if(activeXNode.status == XNODE_STOPPED) return "xnode is stopped";
        if(activeXNode.status == XNODE_IS_CAPABLE) return "successfully started xnode";
        if(activeXNode.status == XNODE_NOT_CAPABLE) return "not capable xnode: " + activeXNode.notCapableReason;
        if(activeXNode.status == XNODE_SYNC_IN_PROCESS) return "sync in process. Must wait until client is synced to start.";

        return "unknown";
    }

    if (strCommand == "start-alias")
    {
        if (params.size() < 2){
            throw runtime_error(
            "command needs at least 2 parameters\n");
        }

        std::string alias = params[1].get_str().c_str();

        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 3){
                strWalletPass = params[2].get_str().c_str();
            } else {
                throw runtime_error(
                "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        bool found = false;

        Object statusObj;
        statusObj.push_back(Pair("alias", alias));

        BOOST_FOREACH(CXNodeConfig::CXNodeEntry xne, xnodeConfig.getEntries()) {
            if(xne.getAlias() == alias) {
                found = true;
                std::string errorMessage;
                std::string strDonateAddress = "";
                std::string strDonationPercentage = "";

                bool result = activeXNode.Register(xne.getIp(), xne.getPrivKey(), xne.getTxHash(), xne.getOutputIndex(), strDonateAddress, strDonationPercentage, errorMessage);

                statusObj.push_back(Pair("result", result ? "successful" : "failed"));
                if(!result) {
                    statusObj.push_back(Pair("errorMessage", errorMessage));
                }
                break;
            }
        }

        if(!found) {
            statusObj.push_back(Pair("result", "failed"));
            statusObj.push_back(Pair("errorMessage", "could not find alias in config. Verify with list-conf."));
        }

        pwalletMain->Lock();
        return statusObj;

    }

    if (strCommand == "start-many")
    {
        if(pwalletMain->IsLocked()) {
            SecureString strWalletPass;
            strWalletPass.reserve(100);

            if (params.size() == 2){
                strWalletPass = params[1].get_str().c_str();
            } else {
                throw runtime_error(
                "Your wallet is locked, passphrase is required\n");
            }

            if(!pwalletMain->Unlock(strWalletPass)){
                return "incorrect passphrase";
            }
        }

        std::vector<CXNodeConfig::CXNodeEntry> xnEntries;
        xnEntries = xnodeConfig.getEntries();

        int total = 0;
        int successful = 0;
        int fail = 0;

        Object resultsObj;

        BOOST_FOREACH(CXNodeConfig::CXNodeEntry xne, xnodeConfig.getEntries()) {
            total++;

            std::string errorMessage;
            std::string strDonateAddress = "";
            std::string strDonationPercentage = "";

            bool result = activeXNode.Register(xne.getIp(), xne.getPrivKey(), xne.getTxHash(), xne.getOutputIndex(), strDonateAddress, strDonationPercentage, errorMessage);

            Object statusObj;
            statusObj.push_back(Pair("alias", xne.getAlias()));
            statusObj.push_back(Pair("result", result ? "succesful" : "failed"));

            if(result) {
                successful++;
            } else {
                fail++;
                statusObj.push_back(Pair("errorMessage", errorMessage));
            }

            resultsObj.push_back(Pair("status", statusObj));
        }
        pwalletMain->Lock();

        Object returnObj;
        returnObj.push_back(Pair("overall", "Successfully started " + boost::lexical_cast<std::string>(successful) + " xnodes, failed to start " +
                boost::lexical_cast<std::string>(fail) + ", total " + boost::lexical_cast<std::string>(total)));
        returnObj.push_back(Pair("detail", resultsObj));

        return returnObj;
    }

    if (strCommand == "debug")
    {
        if(activeXNode.status == XNODE_REMOTELY_ENABLED) return "xnode started remotely";
        if(activeXNode.status == XNODE_INPUT_TOO_NEW) return "xnode input must have at least 15 confirmations";
        if(activeXNode.status == XNODE_IS_CAPABLE) return "successfully started xnode";
        if(activeXNode.status == XNODE_STOPPED) return "xnode is stopped";
        if(activeXNode.status == XNODE_NOT_CAPABLE) return "not capable xnode: " + activeXNode.notCapableReason;
        if(activeXNode.status == XNODE_SYNC_IN_PROCESS) return "sync in process. Must wait until client is synced to start.";

        CTxIn vin = CTxIn();
        CPubKey pubkey = CScript();
        CKey key;
        bool found = activeXNode.GetXnodeVin(vin, pubkey, key);
        if(!found){
            return "Missing xnode input, please look at the documentation for instructions on xnode creation";
        } else {
            return "No problems were found";
        }
    }

    if (strCommand == "create")
    {

        return "Not implemented yet, please look at the documentation for instructions on xnode creation";
    }

    if (strCommand == "current")
    {
        CXNode* winner = xnodeman.GetCurrentXnode(1);
        if(winner) {
            Object obj;
            CScript pubkey;
            pubkey.SetDestination(winner->pubkey.GetID());
            CTxDestination address1;
            ExtractDestination(pubkey, address1);
            //CXNodeAddress address2(address1);
            CBitcoinAddress address2(address1);

            obj.push_back(Pair("IP:port",       winner->addr.ToString().c_str()));
            obj.push_back(Pair("protocol",      (int64_t)winner->protocolVersion));
            obj.push_back(Pair("vin",           winner->vin.prevout.hash.ToString().c_str()));
            obj.push_back(Pair("pubkey",        address2.ToString().c_str()));
            obj.push_back(Pair("lastseen",      (int64_t)winner->lastTimeSeen));
            obj.push_back(Pair("activeseconds", (int64_t)(winner->lastTimeSeen - winner->sigTime)));
            return obj;
        }

        return "unknown";
    }

    if (strCommand == "genkey")
    {
        CKey secret;
        secret.MakeNewKey(false);

        //return CXNodeSecret(secret).ToString();
        return CBitcoinSecret(secret).ToString();
    }

    if (strCommand == "winners")
    {
        Object obj;
        std::string strMode = "addr";

        if (params.size() >= 1) strMode = params[0].get_str();

        for(int nHeight = pindexBest->nHeight-10; nHeight < pindexBest->nHeight+20; nHeight++)
        {
            CScript payee;
            CTxIn vin;
            if(xnodePayments.GetBlockPayee(nHeight, payee, vin)){
                CTxDestination address1;
                ExtractDestination(payee, address1);
                //CXNodeAddress address2(address1);
                CBitcoinAddress address2(address1);

                if(strMode == "addr")
                    obj.push_back(Pair(boost::lexical_cast<std::string>(nHeight),       address2.ToString().c_str()));

                if(strMode == "vin")
                    obj.push_back(Pair(boost::lexical_cast<std::string>(nHeight),       vin.ToString().c_str()));

            } else {
                obj.push_back(Pair(boost::lexical_cast<std::string>(nHeight),       ""));
            }
        }

        return obj;
    }

    if(strCommand == "enforce")
    {
        return (uint64_t)enforceXNodePaymentsTime;
    }

    if(strCommand == "connect")
    {
        std::string strAddress = "";
        if (params.size() == 2){
            strAddress = params[1].get_str().c_str();
        } else {
            throw runtime_error(
                "XNode address required\n");
        }

        CService addr = CService(strAddress);

        if(ConnectNode((CAddress)addr, NULL, true)){
            return "successfully connected";
        } else {
            return "error connecting";
        }
    }

    if(strCommand == "list-conf")
    {
        std::vector<CXNodeConfig::CXNodeEntry> xnEntries;
        xnEntries = xnodeConfig.getEntries();

        Object resultObj;

        BOOST_FOREACH(CXNodeConfig::CXNodeEntry xne, xnodeConfig.getEntries()) {
            Object xnObj;
            xnObj.push_back(Pair("alias", xne.getAlias()));
            xnObj.push_back(Pair("address", xne.getIp()));
            xnObj.push_back(Pair("privateKey", xne.getPrivKey()));
            xnObj.push_back(Pair("txHash", xne.getTxHash()));
            xnObj.push_back(Pair("outputIndex", xne.getOutputIndex()));
            resultObj.push_back(Pair("xnode", xnObj));
        }

        return resultObj;
    }

    if (strCommand == "outputs"){
        // Find possible candidates
        vector<COutput> possibleCoins = activeXNode.SelectCoinsXNode();

        Object obj;
        BOOST_FOREACH(COutput& out, possibleCoins) {
            obj.push_back(Pair(out.tx->GetHash().ToString().c_str(), boost::lexical_cast<std::string>(out.i)));
        }

        return obj;

    }

    if(strCommand == "vote-many")
    {
        std::vector<CXNodeConfig::CXNodeEntry> xnEntries;
        xnEntries = xnodeConfig.getEntries();

        if (params.size() != 2)
            throw runtime_error("You can only vote 'yay' or 'nay'");

        std::string vote = params[1].get_str().c_str();
        if(vote != "yay" && vote != "nay") return "You can only vote 'yay' or 'nay'";
        int nVote = 0;
        if(vote == "yay") nVote = 1;
        if(vote == "nay") nVote = -1;

        int success = 0;
        int failed = 0;

        Object resultObj;

        BOOST_FOREACH(CXNodeConfig::CXNodeEntry xne, xnodeConfig.getEntries()) {
            std::string errorMessage;
            std::vector<unsigned char> vchXnodeSignature;
            std::string strXnodeSignMessage;

            CPubKey pubKeyCollateralAddress;
            CKey keyCollateralAddress;
            CPubKey pubKeyXNode;
            CKey keyXNode;

            if(!xnodeEngineSigner.SetKey(xne.getPrivKey(), errorMessage, keyXNode, pubKeyXNode)){
                printf(" Error upon calling SetKey for %s\n", xne.getAlias().c_str());
                failed++;
                continue;
            }

            CXNode* pxn = xnodeman.Find(pubKeyXNode);
            if(pxn == NULL)
            {
                printf("Can't find xnode by pubkey for %s\n", xne.getAlias().c_str());
                failed++;
                continue;
            }

            std::string strMessage = pxn->vin.ToString() + boost::lexical_cast<std::string>(nVote);

            if(!xnodeEngineSigner.SignMessage(strMessage, errorMessage, vchXnodeSignature, keyXNode)){
                printf(" Error upon calling SignMessage for %s\n", xne.getAlias().c_str());
                failed++;
                continue;
            }

            if(!xnodeEngineSigner.VerifyMessage(pubKeyXNode, vchXnodeSignature, strMessage, errorMessage)){
                printf(" Error upon calling VerifyMessage for %s\n", xne.getAlias().c_str());
                failed++;
                continue;
            }

            success++;

            //send to all peers
            LOCK(cs_vNodes);
            BOOST_FOREACH(CNode* pnode, vNodes)
                pnode->PushMessage("mvote", pxn->vin, vchXnodeSignature, nVote);
        }

        return("Voted successfully " + boost::lexical_cast<std::string>(success) + " time(s) and failed " + boost::lexical_cast<std::string>(failed) + " time(s).");
    }

    if(strCommand == "vote")
    {
        std::vector<CXNodeConfig::CXNodeEntry> xnEntries;
        xnEntries = xnodeConfig.getEntries();

        if (params.size() != 2)
            throw runtime_error("You can only vote 'yay' or 'nay'");

        std::string vote = params[1].get_str().c_str();
        if(vote != "yay" && vote != "nay") return "You can only vote 'yay' or 'nay'";
        int nVote = 0;
        if(vote == "yay") nVote = 1;
        if(vote == "nay") nVote = -1;

        // Choose coins to use
        CPubKey pubKeyCollateralAddress;
        CKey keyCollateralAddress;
        CPubKey pubKeyXNode;
        CKey keyXNode;

        std::string errorMessage;
        std::vector<unsigned char> vchXnodeSignature;
        std::string strMessage = activeXNode.vin.ToString() + boost::lexical_cast<std::string>(nVote);

        if(!xnodeEngineSigner.SetKey(strXnodePrivKey, errorMessage, keyXNode, pubKeyXNode))
            return(" Error upon calling SetKey");

        if(!xnodeEngineSigner.SignMessage(strMessage, errorMessage, vchXnodeSignature, keyXNode))
            return(" Error upon calling SignMessage");

        if(!xnodeEngineSigner.VerifyMessage(pubKeyXNode, vchXnodeSignature, strMessage, errorMessage))
            return(" Error upon calling VerifyMessage");

        //send to all peers
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
            pnode->PushMessage("mvote", activeXNode.vin, vchXnodeSignature, nVote);

    }

    if(strCommand == "status")
    {
        std::vector<CXNodeConfig::CXNodeEntry> xnEntries;
        xnEntries = xnodeConfig.getEntries();

        CScript pubkey;
        pubkey = GetScriptForDestination(activeXNode.pubKeyXNode.GetID());
        CTxDestination address1;
        ExtractDestination(pubkey, address1);
        //CXNodeAddress address2(address1);
        CBitcoinAddress address2(address1);

        Object xnObj;
        xnObj.push_back(Pair("vin", activeXNode.vin.ToString().c_str()));
        xnObj.push_back(Pair("service", activeXNode.service.ToString().c_str()));
        xnObj.push_back(Pair("status", activeXNode.status));
        xnObj.push_back(Pair("pubKeyXNode", address2.ToString().c_str()));
        xnObj.push_back(Pair("notCapableReason", activeXNode.notCapableReason.c_str()));
        return xnObj;
    }

    return Value::null;
}

Value xnodelist(const Array& params, bool fHelp)
{
    std::string strMode = "status";
    std::string strFilter = "";

    if (params.size() >= 1) strMode = params[0].get_str();
    if (params.size() == 2) strFilter = params[1].get_str();

    if (fHelp ||
            (strMode != "activeseconds" && strMode != "donation" && strMode != "full" && strMode != "lastseen" && strMode != "protocol"
                && strMode != "pubkey" && strMode != "rank" && strMode != "status" && strMode != "addr" && strMode != "votes" && strMode != "lastpaid"))
    {
        throw runtime_error(
                "xnodelist ( \"mode\" \"filter\" )\n"
                "Get a list of xnodes in different modes\n"
                "\nArguments:\n"
                "1. \"mode\"      (string, optional/required to use filter, defaults = status) The mode to run list in\n"
                "2. \"filter\"    (string, optional) Filter results. Partial match by IP by default in all modes, additional matches in some modes\n"
                "\nAvailable modes:\n"
                "  activeseconds  - Print number of seconds xnode recognized by the network as enabled\n"
                "  donation       - Show donation settings\n"
                "  full           - Print info in format 'status protocol pubkey vin lastseen activeseconds' (can be additionally filtered, partial match)\n"
                "  lastseen       - Print timestamp of when a xnode was last seen on the network\n"
                "  protocol       - Print protocol of a xnode (can be additionally filtered, exact match)\n"
                "  pubkey         - Print public key associated with a xnode (can be additionally filtered, partial match)\n"
                "  rank           - Print rank of a xnode based on current block\n"
                "  status         - Print xnode status: ENABLED / EXPIRED / VIN_SPENT / REMOVE / POS_ERROR (can be additionally filtered, partial match)\n"
                "  addr            - Print ip address associated with a xnode (can be additionally filtered, partial match)\n"
                "  votes          - Print all xnode votes for a XNode initiative (can be additionally filtered, partial match)\n"
                "  lastpaid       - The last time a node was paid on the network\n"
                );
    }

    Object obj;
    if (strMode == "rank") {
        std::vector<pair<int, CXNode> > vXNodeRanks = xnodeman.GetXNodeRanks(pindexBest->nHeight);
        BOOST_FOREACH(PAIRTYPE(int, CXNode)& s, vXNodeRanks) {
            std::string strVin = s.second.vin.prevout.ToStringShort();
            if(strFilter !="" && strVin.find(strFilter) == string::npos) continue;
            obj.push_back(Pair(strVin,       s.first));
        }
    } else {
        std::vector<CXNode> vXNodes = xnodeman.GetFullXNodeVector();
        BOOST_FOREACH(CXNode& xn, vXNodes) {
            std::string strVin = xn.vin.prevout.ToStringShort();
            if (strMode == "activeseconds") {
                if(strFilter !="" && strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       (int64_t)(xn.lastTimeSeen - xn.sigTime)));
            } else if (strMode == "donation") {
                CTxDestination address1;
                ExtractDestination(xn.donationAddress, address1);
                //CXNodeAddress address2(address1);
                CBitcoinAddress address2(address1);

                if(strFilter !="" && address2.ToString().find(strFilter) == string::npos &&
                    strVin.find(strFilter) == string::npos) continue;

                std::string strOut = "";

                if(xn.donationPercentage != 0){
                    strOut = address2.ToString().c_str();
                    strOut += ":";
                    strOut += boost::lexical_cast<std::string>(xn.donationPercentage);
                }
                obj.push_back(Pair(strVin,       strOut.c_str()));
            } else if (strMode == "full") {
                CScript pubkey;
                pubkey.SetDestination(xn.pubkey.GetID());
                CTxDestination address1;
                ExtractDestination(pubkey, address1);
                //CXNodeAddress address2(address1);
                CBitcoinAddress address2(address1);

                std::ostringstream addrStream;
                addrStream << setw(21) << strVin;

                std::ostringstream stringStream;
                stringStream << setw(10) <<
                               xn.Status() << " " <<
                               xn.protocolVersion << " " <<
                               address2.ToString() << " " <<
                               xn.addr.ToString() << " " <<
                               xn.lastTimeSeen << " " << setw(8) <<
                               (xn.lastTimeSeen - xn.sigTime) << " " <<
                               xn.nLastPaid;
                std::string output = stringStream.str();
                stringStream << " " << strVin;
                if(strFilter !="" && stringStream.str().find(strFilter) == string::npos &&
                        strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(addrStream.str(), output));
            } else if (strMode == "lastseen") {
                if(strFilter !="" && strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       (int64_t)xn.lastTimeSeen));
            } else if (strMode == "protocol") {
                if(strFilter !="" && strFilter != boost::lexical_cast<std::string>(xn.protocolVersion) &&
                    strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       (int64_t)xn.protocolVersion));
            } else if (strMode == "pubkey") {
                CScript pubkey;
                pubkey.SetDestination(xn.pubkey.GetID());
                CTxDestination address1;
                ExtractDestination(pubkey, address1);
                //CXNodeAddress address2(address1);
                CBitcoinAddress address2(address1);

                if(strFilter !="" && address2.ToString().find(strFilter) == string::npos &&
                    strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       address2.ToString().c_str()));
            } else if(strMode == "status") {
                std::string strStatus = xn.Status();
                if(strFilter !="" && strVin.find(strFilter) == string::npos && strStatus.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       strStatus.c_str()));
            } else if (strMode == "addr") {
                if(strFilter !="" && xn.vin.prevout.hash.ToString().find(strFilter) == string::npos &&
                    strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       xn.addr.ToString().c_str()));
            } else if(strMode == "votes"){
                std::string strStatus = "ABSTAIN";

                //voting lasts 30 days, ignore the last vote if it was older than that
                if((GetAdjustedTime() - xn.lastVote) < (60*60*30*24))
                {
                    if(xn.nVote == -1) strStatus = "NAY";
                    if(xn.nVote == 1) strStatus = "YAY";
                }

                if(strFilter !="" && (strVin.find(strFilter) == string::npos && strStatus.find(strFilter) == string::npos)) continue;
                obj.push_back(Pair(strVin,       strStatus.c_str()));
            } else if(strMode == "lastpaid"){
                if(strFilter !="" && xn.vin.prevout.hash.ToString().find(strFilter) == string::npos &&
                    strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,      (int64_t)xn.nLastPaid));
            }
        }
    }
    return obj;
}
