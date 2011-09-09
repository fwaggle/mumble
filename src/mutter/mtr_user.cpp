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
user_list(void)
{
	NameMap users;
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	users = server->getRegisteredUsers("", ctx);
	
	cerr << "ID #:   " << " " << "Username:" << endl;
	cerr << "~~~~~~~~ ~~~~~~~~~" << endl;
	
	for (NameMap::iterator ii=users.begin(); ii != users.end(); ii++) {
		cout << setw(8) << right << (*ii).first << " ";
		cout << left << (*ii).second << endl;
	}
}

void
user_del(string username)
{
	NameList name;
	IdMap users;
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	name.push_back(username);
	
	users = server->getUserIds(name, ctx);
	if (users[username] < 0)
		throw "Invalid User";
	else
	{
		server->unregisterUser(users[username], ctx);
		cout << username << " deleted." << endl;
	}
}

void
user_add(string username)
{
	ServerPrx server;
	UserInfoMap uinfo;
	
	uinfo[UserName] = username;
	
	server = meta->getServer(serverId, ctx);
	cout << "Enter new password for " << uinfo[UserName] << ": ";
	string pass;
	getline(cin, pass);
	
	uinfo[UserPassword] = pass;
	server->registerUser(uinfo, ctx);
	cout << "User " << username << " registered!" << endl;
}

void
user_pass(string username)
{
	NameList name;
	IdMap users;
	ServerPrx server;
	
	server = meta->getServer(serverId, ctx);
	name.push_back(username);
	
	users = server->getUserIds(name, ctx);
	if (users[username] < 0)
		throw "Invalid User";
	else
	{
		UserInfoMap uinfo = server->getRegistration(users[username], ctx);
		cout << "Enter new password for " << uinfo[UserName] << ": ";
		string pass;
		getline(cin, pass);
		
		uinfo[UserPassword] = pass;
		server->updateRegistration(users[username], uinfo, ctx);
		cout << "Password for user " << username << " updated!" << endl;
	}
}

