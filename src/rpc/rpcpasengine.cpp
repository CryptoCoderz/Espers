// Copyright (c) 2022-2023 The CryptoCoderz Team / Espers project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core/main.h"
#include "core/chain.h"
#include "database/db.h"
#include "util/init.h"

#include "pas/pasregister.h"
#include "pas/pasman.h"
#include "pas/pasconf.h"
#include "rpcserver.h"
#include <boost/lexical_cast.hpp>
#include "util/util.h"

#include <fstream>
using namespace json_spirit;
using namespace std;


Value pubkeyaliasservice(const Array& params, bool fHelp)
{
    string strCommand;
    if (params.size() >= 1)
        strCommand = params[0].get_str();

    if (fHelp  ||
        (strCommand != "count" && strCommand != "debug" && strCommand != "genkey" && strCommand != "enforce" && strCommand != "list" && strCommand != "list-conf"
        	&& strCommand != "start" && strCommand != "start-alias" && strCommand != "start-many" && strCommand != "status" && strCommand != "stop" && strCommand != "stop-alias"
                && strCommand != "stop-many" && strCommand != "outputs"))
        throw runtime_error(
                "pubkeyaliasservice \"command\"... ( \"passphrase\" )\n"
                "Set of commands to execute pubkeyaliasservice related actions\n"
                "\nArguments:\n"
                "1. \"command\"        (string or set of strings, required) The command to execute\n"
                "2. \"passphrase\"     (string, optional) The wallet passphrase\n"
                "\nAvailable commands:\n"
                "  count        - Print number of all known pubkeyaliasservices (optional: 'enabled', 'both')\n"
                "  debug        - Print pubkeyaliasservice status\n"
                "  genkey       - Generate new pubkeyaliasservice privkey\n"
                "  enforce      - Enforce pubkeyaliasservice versioning\n"
                "  list         - Print list of all known pubkeyaliasservices (see pubkeyaliasservicelist for more info)\n"
                "  list-conf    - Print pubkeyaliasservice.conf in JSON format\n"
                "  outputs      - Print pubkeyaliasservice compatible outputs\n"
                "  start        - Start pubkeyaliasservice configured in Espers.conf\n"
                "  start-alias  - Start single pubkeyaliasservice by assigned alias configured in pubkeyaliasservice.conf\n"
                "  start-many   - Start all pubkeyaliasservices configured in pubkeyaliasservice.conf\n"
                "  status       - Print pubkeyaliasservice status information\n"
                "  stop         - Stop pubkeyaliasservice configured in Espers.conf\n"
                "  stop-alias   - Stop single pubkeyaliasservice by assigned alias configured in pubkeyaliasservice.conf\n"
                "  stop-many    - Stop all pubkeyaliasservices configured in pubkeyaliasservice.conf\n"
                );

    if (strCommand == "stop")
    {
        if(!fPubkeyAliasService) return "you must set pubkeyaliasservice=1 in the configuration";

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
        if(!pasReg.StopPaService(errorMessage)) {
            return "stop failed: " + errorMessage;
        }
        pwalletMain->Lock();

        if(pasReg.status == PUBKEYALIASSERVICE_STOPPED) return "successfully stopped pubkeyaliasservice";
        if(pasReg.status == PUBKEYALIASSERVICE_NOT_CAPABLE) return "not capable pubkeyaliasservice";

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

        BOOST_FOREACH(CPASConfig::CPASEntry pae, pasConfig.getEntries()) {
            CTxIn vin;
            CPubKey pubKeyFeeAddress;
            CKey keyFeeAddress;
            std::string errorMessage;

            //TODO: Account for output index!!!
            if(!pasReg.GetPaServiceVin(vin, pubKeyFeeAddress, keyFeeAddress)) {
                errorMessage = "could not allocate vin";
                LogPrintf("CPASregister::Register() - Error: %s\n", errorMessage.c_str());
                return false;
            }
            if(pae.getAlias() == alias) {
                found = true;
                errorMessage = "";
                bool result = pasReg.StopPaService(vin, errorMessage);

				statusObj.push_back(Pair("result", result ? "successful" : "failed"));
    			if(!result) {
                    errorMessage = "Failed to stop alias service! Error: CX0";
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

        BOOST_FOREACH(CPASConfig::CPASEntry pae, pasConfig.getEntries()) {
			total++;

            CTxIn vin;
            CPubKey pubKeyFeeAddress;
            CKey keyFeeAddress;
            std::string errorMessage;

            //TODO: Account for output index!!!
            if(!pasReg.GetPaServiceVin(vin, pubKeyFeeAddress, keyFeeAddress)) {
                errorMessage = "could not allocate vin";
                LogPrintf("CPASregister::Register() - Error: %s\n", errorMessage.c_str());
                return false;
            }

            bool result = pasReg.StopPaService(vin, errorMessage);

			Object statusObj;
            statusObj.push_back(Pair("alias", pae.getAlias()));
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
		returnObj.push_back(Pair("overall", "Successfully stopped " + boost::lexical_cast<std::string>(successful) + " pubkeyaliasservices, failed to stop " +
				boost::lexical_cast<std::string>(fail) + ", total " + boost::lexical_cast<std::string>(total)));
		returnObj.push_back(Pair("detail", resultsObj));

		return returnObj;

    }

    if (strCommand == "list")
    {
        Array newParams(params.size() - 1);
        std::copy(params.begin() + 1, params.end(), newParams.begin());
        return pubkeyaliasservicelist(newParams, fHelp);
    }

    if (strCommand == "count")
    {
        if (params.size() > 2){
            throw runtime_error(
                "too many parameters\n");
        }

        if (params.size() == 2)
        {
            if(params[1] == "enabled") return paserviceman.CountEnabledPAS();
            if(params[1] == "both") return boost::lexical_cast<std::string>(paserviceman.CountEnabledPAS()) + " / " + boost::lexical_cast<std::string>(paserviceman.size());
        }
        return paserviceman.size();
    }

    if (strCommand == "start")
    {
        if(!fPubkeyAliasService) return "you must set pubkeyaliasservice=1 in the configuration";

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

        if(pasReg.status != PUBKEYALIASSERVICE_IS_CAPABLE){
            pasReg.status = PUBKEYALIASSERVICE_NOT_PROCESSED; // TODO: consider better way
            std::string errorMessage;
            pasReg.ManageStatus();
            pwalletMain->Lock();
        }

        if(pasReg.status == PUBKEYALIASSERVICE_INPUT_TOO_NEW) return "pubkeyaliasservice input must have at least 15 confirmations";
        if(pasReg.status == PUBKEYALIASSERVICE_STOPPED) return "pubkeyaliasservice is stopped";
        if(pasReg.status == PUBKEYALIASSERVICE_IS_CAPABLE) return "successfully started pubkeyaliasservice";
        if(pasReg.status == PUBKEYALIASSERVICE_NOT_CAPABLE) return "not capable pubkeyaliasservice: " + pasReg.notCapableReason;
        if(pasReg.status == PUBKEYALIASSERVICE_SYNC_IN_PROCESS) return "sync in process. Must wait until client is synced to start.";

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

        BOOST_FOREACH(CPASConfig::CPASEntry pae, pasConfig.getEntries()) {
            if(pae.getAlias() == alias) {
    			found = true;
    			std::string errorMessage;
                std::string strDonateAddress = "";
                std::string strDonationPercentage = "";

                bool result = pasReg.Register(pae.getTxHash(), pae.getOutputIndex(), errorMessage);
  
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

        std::vector<CPASConfig::CPASEntry> paEntries;
        paEntries = pasConfig.getEntries();

		int total = 0;
		int successful = 0;
		int fail = 0;

		Object resultsObj;

        BOOST_FOREACH(CPASConfig::CPASEntry pae, pasConfig.getEntries()) {
			total++;

			std::string errorMessage;
            std::string strDonateAddress = "";
            std::string strDonationPercentage = "";

            bool result = pasReg.Register(pae.getTxHash(), pae.getOutputIndex(), errorMessage);

			Object statusObj;
            statusObj.push_back(Pair("alias", pae.getAlias()));
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
		returnObj.push_back(Pair("overall", "Successfully started " + boost::lexical_cast<std::string>(successful) + " pubkeyaliasservices, failed to start " +
				boost::lexical_cast<std::string>(fail) + ", total " + boost::lexical_cast<std::string>(total)));
		returnObj.push_back(Pair("detail", resultsObj));

		return returnObj;
    }

    if (strCommand == "debug")
    {
        if(pasReg.status == PUBKEYALIASSERVICE_INPUT_TOO_NEW) return "pubkeyaliasservice input must have at least 15 confirmations";
        if(pasReg.status == PUBKEYALIASSERVICE_IS_CAPABLE) return "successfully started pubkeyaliasservice";
        if(pasReg.status == PUBKEYALIASSERVICE_STOPPED) return "pubkeyaliasservice is stopped";
        if(pasReg.status == PUBKEYALIASSERVICE_NOT_CAPABLE) return "not capable pubkeyaliasservice: " + pasReg.notCapableReason;
        if(pasReg.status == PUBKEYALIASSERVICE_SYNC_IN_PROCESS) return "sync in process. Must wait until client is synced to start.";

        CTxIn vin = CTxIn();
        CPubKey pubkey = CScript();
        CKey key;
        //TODO: engage active checks once pas registration is done...
        bool found = pasReg.GetPaServiceVin(vin, pubkey, key);
        if(!found){
            return "Missing pubkeyaliasservice input, please look at the documentation for instructions on pubkeyaliasservice creation";
        } else {
            return "No problems were found";
        }
    }

    if (strCommand == "create")
    {

        return "Not implemented yet, please look at the documentation for instructions on pubkeyaliasservice creation";
    }

    if (strCommand == "genkey")
    {
        CKey secret;
        secret.MakeNewKey(false);

        return CBitcoinSecret(secret).ToString();
    }

    if(strCommand == "enforce")
    {
        return "Not implemented yet, please look at the documentation for instructions on pubkeyaliasservice version enforcement";
    }

    if(strCommand == "list-conf")
    {
        std::vector<CPASConfig::CPASEntry> paEntries;
        paEntries = pasConfig.getEntries();

        Object resultObj;

        BOOST_FOREACH(CPASConfig::CPASEntry pae, pasConfig.getEntries()) {
    		Object mnObj;
            mnObj.push_back(Pair("alias", pae.getAlias()));
            mnObj.push_back(Pair("privateKey", pae.getPrivKey()));
            mnObj.push_back(Pair("txHash", pae.getTxHash()));
            mnObj.push_back(Pair("outputIndex", pae.getOutputIndex()));
    		resultObj.push_back(Pair("pubkeyaliasservice", mnObj));
    	}

    	return resultObj;
    }

    if (strCommand == "outputs"){
        // Find possible candidates
        vector<COutput> possibleCoins = pasReg.SelectCoinsPaservice();

        Object obj;
        BOOST_FOREACH(COutput& out, possibleCoins) {
            obj.push_back(Pair(out.tx->GetHash().ToString().c_str(), boost::lexical_cast<std::string>(out.i)));
        }

        return obj;

    }

    if(strCommand == "status")
    {
        std::vector<CPASConfig::CPASEntry> paEntries;
        paEntries = pasConfig.getEntries();

        CScript pubkey;
        pubkey = GetScriptForDestination(pasReg.pubKeyPaservice.GetID());
        CTxDestination address1;
        ExtractDestination(pubkey, address1);
        CBitcoinAddress address2(address1);

        Object mnObj;
        CPubkeyaliasservice *ppas = paserviceman.Find(pasReg.vin);

        mnObj.push_back(Pair("vin", pasReg.vin.ToString().c_str()));
        mnObj.push_back(Pair("status", pasReg.status));
        if (ppas) mnObj.push_back(Pair("pubkey", CBitcoinAddress(ppas->pubkey.GetID()).ToString()));
        mnObj.push_back(Pair("notCapableReason", pasReg.notCapableReason.c_str()));

        return mnObj;
    }

    return Value::null;
}

Value pubkeyaliasservicelist(const Array& params, bool fHelp)
{
    std::string strMode = "status";
    std::string strFilter = "";

    if (params.size() >= 1) strMode = params[0].get_str();
    if (params.size() == 2) strFilter = params[1].get_str();

    if (fHelp ||
            (strMode != "full" && strMode != "protocol"
                && strMode != "pubkey" && strMode != "status"))
    {
        throw runtime_error(
                "pubkeyaliasservicelist ( \"mode\" \"filter\" )\n"
                "Get a list of pubkeyaliasservices in different modes\n"
                "\nArguments:\n"
                "1. \"mode\"      (string, optional/required to use filter, defaults = status) The mode to run list in\n"
                "2. \"filter\"    (string, optional) Filter results. Partial match by IP by default in all modes, additional matches in some modes\n"
                "\nAvailable modes:\n"
                "  full           - Print info in format 'status protocol pubkey vin lastseen activeseconds' (can be additionally filtered, partial match)\n"
                "  protocol       - Print protocol of a pubkeyaliasservice (can be additionally filtered, exact match)\n"
                "  pubkey         - Print public key associated with a pubkeyaliasservice (can be additionally filtered, partial match)\n"
                "  status         - Print pubkeyaliasservice status: ENABLED / EXPIRED / VIN_SPENT / REMOVE / POS_ERROR (can be additionally filtered, partial match)\n"
                );
    }

    Object obj;
//
        std::vector<CPubkeyaliasservice> vPubkeyaliasservices = paserviceman.GetFullPubkeyaliasserviceVector();
        BOOST_FOREACH(CPubkeyaliasservice& pas, vPubkeyaliasservices) {
            std::string strVin = pas.vin.prevout.ToStringShort();
            if (strMode == "full") {
                CScript pubkey;
                pubkey.SetDestination(pas.pubkey.GetID());
                CTxDestination address1;
                ExtractDestination(pubkey, address1);
                CBitcoinAddress address2(address1);

                std::ostringstream addrStream;
                addrStream << setw(21) << strVin;

                std::ostringstream stringStream;
                stringStream << setw(10) <<
                               pas.Status() << " " <<
                               pas.protocolVersion << " " <<
                               address2.ToString();
                std::string output = stringStream.str();
                stringStream << " " << strVin;
                if(strFilter !="" && stringStream.str().find(strFilter) == string::npos &&
                        strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(addrStream.str(), output));
            } else if (strMode == "protocol") {
                if(strFilter !="" && strFilter != boost::lexical_cast<std::string>(pas.protocolVersion) &&
                    strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       (int64_t)pas.protocolVersion));
            } else if (strMode == "pubkey") {
                CScript pubkey;
                pubkey.SetDestination(pas.pubkey.GetID());
                CTxDestination address1;
                ExtractDestination(pubkey, address1);
                CBitcoinAddress address2(address1);

                if(strFilter !="" && address2.ToString().find(strFilter) == string::npos &&
                    strVin.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       address2.ToString().c_str()));
            } else if(strMode == "status") {
                std::string strStatus = pas.Status();
                if(strFilter !="" && strVin.find(strFilter) == string::npos && strStatus.find(strFilter) == string::npos) continue;
                obj.push_back(Pair(strVin,       strStatus.c_str()));
            }
        }
//
    return obj;

}
