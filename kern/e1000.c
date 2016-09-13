#include <kern/e1000.h>

#include <inc/string.h>
#include <inc/error.h>

#include <kern/pmap.h>

#define NTXDESC  16
static struct e1000_tx_desc tx_queue[NTXDESC] __attribute__((aligned(16)));

#define TX_PKT_BUFF_SIZE ETHERNET_PACKET_SIZE

static char tx_buffs[NTXDESC][TX_PKT_BUFF_SIZE];

int
e1000_tx_init() 
{
    int i;
    
    assert(E1000_REG(E1000_STATUS) == 0x80080783); 
    cprintf("E1000 Status: %08x\n", E1000_REG(E1000_STATUS));
    
    assert(sizeof(struct e1000_tx_desc) == 16);
    assert(((uint32_t)(&tx_queue[0]) & 0xf) == 0);
    assert(sizeof(tx_queue) % 128 == 0);
	
    memset(tx_queue, 0, sizeof(tx_queue));
    for (i = 0; i < NTXDESC; i++) 
        tx_queue[i].buff_addr = PADDR(tx_buffs[i]);

    E1000_REG(E1000_TDBAL) = PADDR(tx_queue); 
    E1000_REG(E1000_TDBAH) = 0;
    E1000_REG(E1000_TDLEN) = sizeof(tx_queue);
    E1000_REG(E1000_TDH)   = 0;
    E1000_REG(E1000_TDT)   = 0;

    E1000_REG(E1000_TCTL) |= E1000_TCTL_EN;
    E1000_REG(E1000_TCTL) |= E1000_TCTL_PSP;

    E1000_REG(E1000_TCTL) &= ~E1000_TCTL_COLD;
    E1000_REG(E1000_TCTL) |= 0x00040000; // TCTL.COLD: 40h

    E1000_REG(E1000_TIPG)  = 10;
    
    return 0;
}

int 
e1000_transmit(const void *data, size_t len) 
{
    uint32_t tail = E1000_REG(E1000_TDT);

    if (len > TX_PKT_BUFF_SIZE)
        return -E_PKT_TOO_LONG;	

    if ((tx_queue[tail].cmd & E1000_TXD_CMD_RS) && !(tx_queue[tail].sta & E1000_TXD_STA_DD))
        return -E_TX_FULL;

    memcpy(tx_buffs[tail], data, len);
    tx_queue[tail].length = len;
    tx_queue[tail].cmd |= E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
    tx_queue[tail].sta &= ~E1000_TXD_STA_DD;

    E1000_REG(E1000_TDT) = (tail + 1) % NTXDESC;
   
    return 0;
}
