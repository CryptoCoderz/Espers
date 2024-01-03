// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2017-2024 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xnodesettings.h"
//TODO: remove linking once converted to Deminode handling below
#include <primitives/base58.h>

CXNodeConfig xnodeConfig;

//TODO: Convert read function to match Deminodes (std vs boost)
boost::filesystem::path GetXNodeConfigFile()
{
    boost::filesystem::path pathConfigFile(GetArg("-xnconf", "xnode.conf"));
    if (!pathConfigFile.is_complete()) pathConfigFile = GetDataDir() / pathConfigFile;
    return pathConfigFile;
}

void CXNodeConfig::add(std::string alias, std::string ip, std::string privKey, std::string txHash, std::string outputIndex) {
    CXNodeEntry cme(alias, ip, privKey, txHash, outputIndex);
    entries.push_back(cme);
}

//TODO: Convert read function to match Deminodes (std vs boost)
bool CXNodeConfig::read(boost::filesystem::path path) {
    boost::filesystem::ifstream streamConfig(GetXNodeConfigFile());
    if (!streamConfig.good()) {
        return true; // No xnode.conf file is OK
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
            LogPrintf("Could not parse xnode.conf line: %s\n", line.c_str());
            streamConfig.close();
            return false;
        }

        add(alias, ip, privKey, txHash, outputIndex);
    }

    streamConfig.close();
    return true;
}
