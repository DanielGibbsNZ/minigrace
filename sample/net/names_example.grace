import "net" as net

def hostname = net.hostname
print "My hostname is {hostname}"

print "\nResolving {hostname}..."
for(net.addressesFor(hostname)) do { addr ->
    print "\t{addr}"
}

print "\nResolving an address for www.gracelang.org..."
def addr = net.addressFor("www.gracelang.org")
print "\t{addr}"

print "\nResolving all addresses for www.google.com..."
for(net.addressesFor("www.google.com")) do { addr ->
    print "\t{addr}"
}

print "\nResolving all IPv6 addresses for www.google.com..."
for(net.inet6AddressesFor("www.google.com")) do { addr ->
    print "\t{addr}"
}

print "\nResolving all IPv4 addresses for www.google.com..."
for(net.inetAddressesFor("www.google.com")) do { addr ->
    print "\t{addr}"
}
