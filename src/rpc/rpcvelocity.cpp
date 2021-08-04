// Copyright (c) 2014 The Cryptocoin Revival Foundation
// Copyright (c) 2015-2019 The CryptoCoderz Team / Espers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "core/main.h"
#include "rpcvelocity.h"
#include <boost/assign/list_of.hpp>

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

/* Patches
   - bitcoinrpc.cpp:
      #include "rpcvelocity.h"
      CRPCCommand:
       { "getvelocityinfo",           &getvelocityinfo,           true,      false },
*/

/* getvelocityinfo(const Array& params, bool fHelp) : Object
   Expose Velocity-Settings via RPC */
json_spirit::Value getvelocityinfo(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0)
		throw std::runtime_error("getvelocityinfo\nReturns an object containing various velocity info.");
    int i = VelocityI(nBestHeight);
	json_spirit::Object obj;
    obj.push_back(json_spirit::Pair("toggle system",       (int)VELOCITY_HEIGHT[i]));
    obj.push_back(json_spirit::Pair("toggle retarget",       (int)VELOCITY_TERMINAL[i]));
	obj.push_back(json_spirit::Pair("rate",         (int)VELOCITY_RATE[i]));
	if( VELOCITY_MIN_RATE[i] > 0 )
		obj.push_back(json_spirit::Pair("min-rate",   (int)VELOCITY_MIN_RATE[i]));
        obj.push_back(json_spirit::Pair("max-rate",   (int)VELOCITY_MAX_RATE[i]));
	if( VELOCITY_MIN_TX[i] > 0 )
		obj.push_back(json_spirit::Pair("min-tx",     (int)VELOCITY_MIN_TX[i]));
	if( VELOCITY_MIN_VALUE[i] > 0 )
		obj.push_back(json_spirit::Pair("min-value",  (int)VELOCITY_MIN_VALUE[i]));
	if( VELOCITY_MIN_FEE[i] > 0 )
		obj.push_back(json_spirit::Pair("min-fee",    (int)VELOCITY_MIN_FEE[i]));
    obj.push_back(json_spirit::Pair("factor",       VELOCITY_FACTOR));
	obj.push_back(json_spirit::Pair("explicit",     VELOCITY_EXPLICIT[i]));
	return obj;
}
