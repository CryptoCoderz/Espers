// Copyright (c) 2022-2024 The CryptoCoderz Team / Espers project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pasconf.h"
//TODO: remove linking once converted to Deminode handling below
#include <primitives/base58.h>

CPASConfig pasConfig;

//TODO: Convert read function to match Deminodes (std vs boost)
boost::filesystem::path GetPASConfigFile()
{
    boost::filesystem::path pathConfigFile(GetArg("-pasconf", "pas.conf"));
    if (!pathConfigFile.is_complete()) pathConfigFile = GetDataDir() / pathConfigFile;
    return pathConfigFile;
}

void CPASConfig::add(std::string alias, std::string ip, std::string privKey, std::string txHash, std::string outputIndex) {
    CPASEntry cpe(alias, ip, privKey, txHash, outputIndex);
    entries.push_back(cpe);
}

//TODO: Convert read function to match Deminodes (std vs boost)
bool CPASConfig::read(boost::filesystem::path path) {
    boost::filesystem::ifstream streamConfig(GetPASConfigFile());
    if (!streamConfig.good()) {
        return true; // No pas.conf file is OK
    }

    for(std::string line; std::getline(streamConfig, line); )
    {
        if(line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        std::string alias, ip, privKey, txHash, outputIndex;
        iss.str(line);
        iss.clear();
        if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex)) {
            LogPrintf("Could not parse pas.conf line: %s\n", line.c_str());
            streamConfig.close();
            return false;
        }

        add(alias, ip, privKey, txHash, outputIndex);
    }

    streamConfig.close();
    return true;
}
