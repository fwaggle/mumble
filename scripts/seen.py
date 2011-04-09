#!/usr/bin/env python
# -*- coding: utf-8
import Ice, sys, sha
from M2Crypto import X509;

Ice.loadSlice('', ['-I' + Ice.getSliceDir(), 'Murmur.ice'])
import Murmur

class MetaCallbackI(Murmur.MetaCallback):
    def started(self, s, current=None):
        print "started"
        serverR=Murmur.ServerCallbackPrx.uncheckedCast(adapter.addWithUUID(ServerCallbackI(server, current.adapter)))
        s.addCallback(serverR)

    def stopped(self, s, current=None):
        print "stopped"

class ServerCallbackI(Murmur.ServerCallback):
    def __init__(self, server, adapter):
      self.server = server
      self.contextR=Murmur.ServerContextCallbackPrx.uncheckedCast(adapter.addWithUUID(ServerContextCallbackI(server)))

    def userTextMessage(self, p, msg, current=None):
      if msg.text[0:5] == '!seen':
          u = None
          ulookup = server.getRegisteredUsers(msg.text[6:])
          for uw in ulookup:
              if ulookup[uw] == msg.text[6:]:
                  u = server.getRegistration(uw)
                  break
          
          if u is None:
              server.sendMessage(p.session, "I don't know who user %s is" % (msg.text[6:]))
          else:
              server.sendMessage(p.session, "User %s was last active %s" % (u[Murmur.UserInfo.UserName], u[Murmur.UserInfo.UserLastActive]))

class ServerContextCallbackI(Murmur.ServerContextCallback):
    def __init__(self, server):
      self.server = server

if __name__ == "__main__":
    global contextR

    prop = Ice.createProperties(sys.argv)
    prop.setProperty("Ice.ImplicitContext", "Shared")

    idd = Ice.InitializationData()
    idd.properties = prop

    ice = Ice.initialize(idd)

    print "Creating callbacks...",

    # If icesecret is set, we need to set it here as well.
    ice.getImplicitContext().put("secret", "fourtytwo")

    meta = Murmur.MetaPrx.checkedCast(ice.stringToProxy('Meta:tcp -h 127.0.0.1 -p 6502'))
    adapter = ice.createObjectAdapterWithEndpoints("Callback.Client", "tcp -h 127.0.0.1")
    
    metaR=Murmur.MetaCallbackPrx.uncheckedCast(adapter.addWithUUID(MetaCallbackI()))
    adapter.activate()
    
    meta.addCallback(metaR)

    for server in meta.getBootedServers():
      serverR=Murmur.ServerCallbackPrx.uncheckedCast(adapter.addWithUUID(ServerCallbackI(server, adapter)))
      server.addCallback(serverR)

    print "Done"
    print 'Script running (press CTRL-C to abort)';
    try:
        ice.waitForShutdown()
    except KeyboardInterrupt:
        print 'CTRL-C caught, aborting'

    meta.removeCallback(metaR)
    ice.shutdown()
    print "Goodbye"
