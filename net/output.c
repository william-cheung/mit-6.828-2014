#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
    int r;

    binaryname = "ns_output";
	
    while (1) { 
        envid_t envid;	
        // read a packet from the network server 
        if ((r = ipc_recv(&envid, &nsipcbuf, NULL)) < 0)
            panic("%s: ipc_recv : %e", binaryname, r);

        // if ipc value is not NSREQ_OUTPUT or the page we received is not from 
        // the Network Server, continue	
        if (r != NSREQ_OUTPUT || envid != ns_envid) 
            continue;
        
        // send the packet to the device driver
        while ((r = sys_nic_try_send(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len)) == -E_TX_FULL)
            sys_yield();
		
        if (r < 0)
            panic("%s: sys_nic_try_send: %e", binaryname, r);
    }
}
