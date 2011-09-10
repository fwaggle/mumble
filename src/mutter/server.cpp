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

#include <iostream>
#include <iomanip>
#include <iterator>
#include <Ice/Ice.h>
#include <Murmur.h>

#include <mutter.h>

using namespace std;
using namespace Murmur;

void
serv_list(void)
{
	unsigned int i;
	int id;
	string name, host, port;
	vector<ServerPrx> servers = meta->getAllServers(ctx);
	
	cerr << left << "ID    " << setw(40) << "Server Name" << setw(5) << " On?"
		<< "Host" << endl;
	cerr << left << "~~~~~ " << setw(40) << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
		<< setw(5) << " ~~~" << "~~~~~~~~~~~~~~~~~~~" << endl;
	
	for (i=0; i < servers.size(); i++)
	{
		id = servers[i]->id(ctx);
		name = servers[i]->getConf("registername", ctx);
		host = servers[i]->getConf("host", ctx);
		port = servers[i]->getConf("port", ctx);
		
		cout << setw(5) << right << id << " " << setw(40) << left << name;
		if (servers[i]->isRunning(ctx))
			cout << " On  ";
		else
			cout << " Off ";
		
		cout << host << ":" << port << endl;
	}
}

void
serv_start(void)
{
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	server->start(ctx);
}

void
serv_stop(void)
{
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	server->stop(ctx);
}

void
serv_new(void)
{
	ServerPrx server;
	int id;
	
	server = meta->newServer(ctx);
	id = server->id(ctx);
	
	cout << "New server ID: " << id << endl;
}

void
serv_del(void)
{
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	/*
	 * This is removed, because I think it's a bit dangerous..
	 */
//	if (server->isRunning(ctx))
//		server->stop(ctx);
	server->_cpp_delete(ctx);
	cout << "Server deleted!" << endl;
}

