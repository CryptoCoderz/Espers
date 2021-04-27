// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2017-2021 The CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SRC_XNODECONFIG_H_
#define SRC_XNODECONFIG_H_

#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

class CXNodeConfig;
extern CXNodeConfig xnodeConfig;

class CXNodeConfig
{

public:

    class CXNodeEntry {

    private:
        std::string alias;
        std::string ip;
        std::string privKey;
        std::string txHash;
        std::string outputIndex;
    public:

        CXNodeEntry(std::string alias, std::string ip, std::string privKey, std::string txHash, std::string outputIndex) {
            this->alias = alias;
            this->ip = ip;
            this->privKey = privKey;
            this->txHash = txHash;
            this->outputIndex = outputIndex;
        }

        const std::string& getAlias() const {
            return alias;
        }

        void setAlias(const std::string& alias) {
            this->alias = alias;
        }

        const std::string& getOutputIndex() const {
            return outputIndex;
        }

        void setOutputIndex(const std::string& outputIndex) {
            this->outputIndex = outputIndex;
        }

        const std::string& getPrivKey() const {
            return privKey;
        }

        void setPrivKey(const std::string& privKey) {
            this->privKey = privKey;
        }

        const std::string& getTxHash() const {
            return txHash;
        }

        void setTxHash(const std::string& txHash) {
            this->txHash = txHash;
        }

        const std::string& getIp() const {
            return ip;
        }

        void setIp(const std::string& ip) {
            this->ip = ip;
        }
    };

    CXNodeConfig() {
        entries = std::vector<CXNodeEntry>();
    }

    void clear();
    bool read(boost::filesystem::path path);
    void add(std::string alias, std::string ip, std::string privKey, std::string txHash, std::string outputIndex);

    std::vector<CXNodeEntry>& getEntries() {
        return entries;
    }

private:
    std::vector<CXNodeEntry> entries;


};


#endif /* SRC_XNODECONFIG_H_ */
