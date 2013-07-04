#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "gracelib.h"

// net.c defines a Grace module "net" which can be compiled for
// dynamic or static linking. The module is a wrapper for network
// and socket related functions common to most *NIX systems.
//
// This module supports both IPv4 and IPv6 and the default methods
// will use whatever the operating system chooses, however there
// are specific methods that can be used for either IPv4 or IPv6
// addresses only.

Object net_module = NULL;
ClassData NetAddress;
extern ClassData Octets;
extern Object none;

// NetAddress object.

struct NetAddressObject {
    OBJECT_HEADER;
    int ai_family;
    struct sockaddr_storage addr;
};

Object NetAddress_asString(Object self, int nparams, int *argcv,
        Object *argv, int flags) {
    struct NetAddressObject *r = (struct NetAddressObject *)self;
    char dst[INET6_ADDRSTRLEN];
    if(r->ai_family == AF_INET) {
        if(inet_ntop(AF_INET, &((struct sockaddr_in *)&r->addr)
                ->sin_addr, dst, INET_ADDRSTRLEN) == NULL)
            dst[0] = 0;
    }
    else if(r->ai_family == AF_INET6) {
        if(inet_ntop(AF_INET6, &((struct sockaddr_in6 *)&r->addr)
                ->sin6_addr, dst, INET6_ADDRSTRLEN) == NULL)
            dst[0] = 0;
    }
    else {
        dst[0] = 0;
    }
    return alloc_String(dst);
}

void NetAddress__mark(struct NetAddressObject *r) {
}

void NetAddress__release(struct NetAddressObject *r) {
}

Object alloc_NetAddress(int ai_family, struct sockaddr *addr) {
    if (!NetAddress) {
        NetAddress = alloc_class3("NetAddress", 4, (void*)&NetAddress__mark,
                (void*)&NetAddress__release);
        add_Method(NetAddress, "==", &Object_Equals);
        add_Method(NetAddress, "!=", &Object_NotEquals);
        add_Method(NetAddress, "asString", &NetAddress_asString);
        add_Method(NetAddress, "asDebugString", &NetAddress_asString);
    }
    Object o = alloc_obj(sizeof(struct NetAddressObject)
            - sizeof(struct Object), NetAddress);
    struct NetAddressObject *r = (struct NetAddressObject *)o;
    r->ai_family = ai_family;
    memset(&r->addr, 0, sizeof(r->addr));
    if(ai_family == AF_INET)
        memcpy(&r->addr, addr, sizeof(struct sockaddr_in));
    else if(ai_family == AF_INET6)
        memcpy(&r->addr, addr, sizeof(struct sockaddr_in6));
    return o;
}

// NetModule object.

struct NetModuleObject {
    OBJECT_HEADER;
};

Object net_hostname(Object self, int nparams, int *argcv,
        Object *argv, int flags) {
    char hostname[256];
    if(gethostname(hostname, sizeof(hostname)) == 0)
        return alloc_String(hostname);
    else
        return alloc_none();
}

// Utility method used by all of the addressFor methods.
Object __net_addrinfo(int ai_family, char *hostname, int n) {
    struct addrinfo hints, *p, *hostinfo;
    Object l = alloc_List();

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = ai_family;
    if(hostname == NULL)
        hints.ai_flags |= AI_PASSIVE;

    if(getaddrinfo(hostname, NULL, &hints, &hostinfo) != 0)
        return l;

    int cargcv[] = {1};
    Object carg;
    char dst[INET6_ADDRSTRLEN];
    int i = 0;

    for(p = hostinfo; p != NULL; p = p->ai_next) {
        if(ai_family != AF_UNSPEC && p->ai_family != ai_family)
            continue;
        carg = alloc_NetAddress(p->ai_family, p->ai_addr);
        if(!istrue(callmethod(l, "contains", 1, cargcv, &carg)))
            callmethod(l, "push", 1, cargcv, &carg);
        if(n > 0 && ++i >= n)
            break;
    }
    freeaddrinfo(hostinfo);

    return l;
}

Object net_addressesFor(Object self, int nparams, int *argcv,
        Object *argv, int flags) {
    return __net_addrinfo(AF_UNSPEC, (argcv[0] > 0 ? grcstring(argv[0]) : NULL), 0);
}

Object net_addressFor(Object self, int nparams, int *argcv,
        Object *argv, int flags) {
    Object addresses = __net_addrinfo(AF_UNSPEC, (argcv[0] > 0 ? grcstring(argv[0]) : NULL), 1);
    // If struct ListObject was in gracelib.h this would be easier.
    if(integerfromAny(callmethod(addresses, "size", 0, NULL, NULL)) > 0) {
        int partcv = 1;
        Object idx = alloc_Integer32(1);
        return callmethod(addresses, "at", 1, &partcv, &idx);
    }
    else
        return alloc_none();
}

Object net_inetaddressesFor(Object self, int nparams, int *argcv,
        Object *argv, int flags) {
    return __net_addrinfo(AF_INET, (argcv[0] > 0 ? grcstring(argv[0]) : NULL), 0);
}

Object net_inetaddressFor(Object self, int nparams, int *argcv,
        Object *argv, int flags) {
    Object addresses = __net_addrinfo(AF_INET, (argcv[0] > 0 ? grcstring(argv[0]) : NULL), 1);
    // If struct ListObject was in gracelib.h this would be easier.
    if(integerfromAny(callmethod(addresses, "size", 0, NULL, NULL)) > 0) {
        int partcv = 1;
        Object idx = alloc_Integer32(1);
        return callmethod(addresses, "at", 1, &partcv, &idx);
    }
    else
        return alloc_none();
}

Object net_inet6addressesFor(Object self, int nparams, int *argcv,
        Object *argv, int flags) {
    return __net_addrinfo(AF_INET6, (argcv[0] > 0 ? grcstring(argv[0]) : NULL), 0);
}

Object net_inet6addressFor(Object self, int nparams, int *argcv,
        Object *argv, int flags) {
    Object addresses = __net_addrinfo(AF_INET6, (argcv[0] > 0 ? grcstring(argv[0]) : NULL), 1);
    // If struct ListObject was in gracelib.h this would be easier.
    if(integerfromAny(callmethod(addresses, "size", 0, NULL, NULL)) > 0) {
        int partcv = 1;
        Object idx = alloc_Integer32(1);
        return callmethod(addresses, "at", 1, &partcv, &idx);
    }
    else
        return alloc_none();
}

// Create and return a Grace object with all the above functions as methods.
Object module_net_init() {
    if (net_module != NULL)
        return net_module;
    ClassData c = alloc_class("Module<net>", 7);
    add_Method(c, "hostname", &net_hostname);
    add_Method(c, "addressFor", &net_addressFor);
    add_Method(c, "addressesFor", &net_addressesFor);
    add_Method(c, "inetAddressFor", &net_inetaddressFor);
    add_Method(c, "inetAddressesFor", &net_inetaddressesFor);
    add_Method(c, "inet6AddressFor", &net_inet6addressFor);
    add_Method(c, "inet6AddressesFor", &net_inet6addressesFor);
    Object o = alloc_newobj(sizeof(struct NetModuleObject)
            - sizeof(struct Object), c);
    net_module = o;
    gc_root(net_module);
    struct NetModuleObject *nmo = (struct NetModuleObject *)o;
    return o;
}

