/* Copyright (C) 2011, Jamie Fraser <jamie.f@mumbledog.com>

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   - Neither the name of the Mumble Developers nor the names of its
     contributors may be used to endorse or promote products derived from this
     software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <iostream>
#include <iomanip>
#include <iterator>
#include <Ice/Ice.h>
#include <Murmur.h>

#include <mutter.h>
#include <mtr_config.h>
#include <mtr_player.h>
#include <mtr_user.h>
#include <mtr_server.h>

using namespace std;
using namespace Murmur;

MetaPrx meta;
Ice::Context ctx;
int serverId;

int
main (int argc, char **argv)
{
	string iceProxy;
	string iceSecret;
	string configKey;
	string configValue;
	string username;
	string reason;
	
	int action;
	int ret;
	int playerSession;
	
	Ice::CommunicatorPtr ic;
	
	/*
	 * Start with some sensible defaults
	 */
	iceProxy = "Meta:tcp -h localhost -p 6502";
	serverId = 1;
	
	action = 0;
	ret = 0;
	
	/*
	 * Use boost to parse command line arguments
	 */
	try {
		po::options_description desc("Usage:");
		desc.add_options()
			("help", "Produce this help message.")
			("sid,s", po::value<int>(), "Set virtual server ID.")
			("ice-proxy,i", po::value<string>(), "Set the proxy to use for ICE.")
			("ice-secret,z", po::value<string>(), "Set the secret given to Murmur.")

			("config,C", po::value<string>(), "Peek/Poke Configuration setting <arg>")
			("value,V", po::value<string>(), "Data to be poked into Configuration setting (requires -C)")

			("list-servers,L", "List all virtual servers on the Murmur.")
			("start,S", "Start virtual server (specify ID # with -s).")
			("stop,T", "Stop virtual server (specify ID # with -s).")
			("add-server,N", "Add a new virtual server (returns ID #).")
			("del-server,R", "Remove a virtual server (specify ID # with -s).")

			("list-users,l", "List all registered users on a virtual server.")
			("add-user,a", po::value<string>(), "Add user <arg> to registered users.")
			("password,p", po::value<string>(), "Change <arg>'s password.")
			("del-user,d", po::value<string>(), "Delete registered user <arg>.")
			
			("list-players", "List users currently connected to server.")
			("kick-player,k", po::value<int>(), "Kick player with session # <arg> (get session with --list-players).")
			("reason", po::value<string>(&reason)->default_value("Via Console"), "Reason for kick/ban.")
		;
		
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
		
		if (vm.count("help")) {
			cout << desc << endl;
			return (-1);
		}
		
		if (vm.count("sid")) {
			serverId = vm["sid"].as<int>();
		}
		
		if (vm.count("ice-secret")) {
			iceSecret = vm["ice-secret"].as<string>();
		}
		
		if (vm.count("ice-proxy")) {
			iceProxy = vm["ice-proxy"].as<string>();
		}
		
		if (vm.count("config")) {
			action = ACT_CONFPEEK;
			configKey = vm["config"].as<string>();
		}

		if (vm.count("value")) {
			action = ACT_CONFPOKE;
			configValue = vm["value"].as<string>();
			
			if (configValue == "-")
			{
				string line;
				
				configValue = "";
				
				while (cin)
				{
					getline(cin, line);
					configValue = configValue + line + "\n";
				}
			}
		}
		
		if (vm.count("list-servers")) {
			action = ACT_SERVLIST;
		}
		
		if (vm.count("list-users")) {
			action = ACT_USERLIST;
		}
		
		if (vm.count("start")) {
			action = ACT_START;
		}
		
		if (vm.count("stop")) {
			action = ACT_STOP;
		}
		
		if (vm.count("add-server")) {
			action = ACT_SERVNEW;
		}
		
		if (vm.count("del-server")) {
			action = ACT_SERVDEL;
		}
		
		if (vm.count("add-user")) {
			action = ACT_USERADD;
			username = vm["add-user"].as<string>();
		}
		
		if (vm.count("password")) {
			action = ACT_USERPASS;
			username = vm["password"].as<string>();
		}
		
		if (vm.count("del-user")) {
			action = ACT_USERDEL;
			username = vm["del-user"].as<string>();
		}
		
		if (vm.count("list-players")) {
			action = ACT_PLAYERLIST;
		}

		if (vm.count("kick-player")) {
			playerSession = vm["kick-player"].as<int>();
			action = ACT_PLAYERKICK;
		}
	}
	catch (exception &e) {
		cerr << "error: " << e.what() << endl;
		return (-1);
	}
	catch (...) {
		cerr << "ouch!" << endl;
		return (-1);
	}
	
	/*
	 * Start Ice stuff
	 */
	if (!action) {
		cerr << "Nothing to do - exiting. See mutter --help" << endl;
		return (-1);
	} else try {
		ic = Ice::initialize(argc, argv);
		Ice::ObjectPrx base = ic->stringToProxy(iceProxy);
		if (iceSecret.length() > 0)
			ctx["secret"] = (string)iceSecret;
		
		meta = MetaPrx::checkedCast(base);
		if (!meta)
			throw "Ice Error: Invalid Proxy";
		
		/*
		 * Actually perform the action
		 */
		
		switch (action)
		{
		case ACT_CONFPEEK:
			config_peek(configKey);
			break;
		case ACT_CONFPOKE:
			config_poke(configKey, configValue);
			break;
		case ACT_SERVLIST:
			serv_list();
			break;
		case ACT_USERLIST:
			user_list();
			break;
		case ACT_START:
			serv_start();
			break;
		case ACT_STOP:
			serv_stop();
			break;
		case ACT_SERVNEW:
			serv_new();
			break;
		case ACT_SERVDEL:
			serv_del();
			break;
		case ACT_USERADD:
			user_add(username);
			break;
		case ACT_USERDEL:
			user_del(username);
			break;
		case ACT_USERPASS:
			user_pass(username);
			break;
		case ACT_PLAYERLIST:
			player_list();
			break;
		case ACT_PLAYERKICK:
			player_kick(playerSession, reason);
			break;
		}
	} catch (const Ice::Exception& ex) {
		cerr << ex << endl;
		ret = -1;
	} catch (const char* msg) {
		cerr << msg << endl;
		ret = -1;
	}
	
	/*
	 * Show's over, clean up after Ice
	 */
	if (ic)
		ic->destroy();
	
	return (ret);
}
