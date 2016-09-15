#include "ns.h"

extern union Nsipc nsipcbuf;

static const int INPUT_WAIT_COUNT = 10;

void
input(envid_t ns_envid)
{
    size_t bufsiz = PGSIZE - sizeof(struct jif_pkt);
    int i, r;

    binaryname = "ns_input";

    // the corresponding page frame is an COW page after fork, we must allocate 
    // our own address space before entering kernel mode (sys_nic_recv)
    memcpy(nsipcbuf.pkt.jp_data, "copy page on write", 1);

    while (1) {
        // read a packet from the device driver
        while ((r = sys_nic_recv(nsipcbuf.pkt.jp_data, bufsiz)) == -E_RX_EMPTY)
            sys_yield();
        if (r < 0)
            panic("%s: sys_nic_recv: %e", binaryname, r);
        nsipcbuf.pkt.jp_len = r;
        
        // send the packet to the network server
        ipc_send(ns_envid, NSREQ_INPUT, &nsipcbuf, PTE_P|PTE_W|PTE_U);

        // When you IPC a page to the network server, it will be
        // reading from it for a while, so don't immediately receive
        // another packet in to the same physical page.
        for (i = 0; i < INPUT_WAIT_COUNT; i++)
            sys_yield();
    }
}
