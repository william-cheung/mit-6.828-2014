#include <kern/e1000.h>

#include <inc/string.h>
#include <inc/error.h>

#include <kern/pmap.h>

#define NTXDESC  16
static struct e1000_tx_desc tx_queue[NTXDESC] __attribute__ ((aligned(16)));

#define TX_PKT_BUFF_SIZE ETHERNET_PACKET_SIZE
static char tx_buffs[NTXDESC][TX_PKT_BUFF_SIZE];

#define NRXDESC  128
static struct e1000_rx_desc rx_queue[NRXDESC] __attribute__ ((aligned(16)));

#define RX_PKT_BUFF_SIZE 2048
static char rx_buffs[NRXDESC][RX_PKT_BUFF_SIZE];


int
e1000_tx_init(void) 
{
    int i;
    
    // Check if alignment requirements are satisfied  
    assert(sizeof(struct e1000_tx_desc) == 16);
    assert(((uint32_t)(&tx_queue[0]) & 0xf) == 0);
    assert(sizeof(tx_queue) % 128 == 0);
	
    // Initialize packet buffers
    memset(tx_queue, 0, sizeof(tx_queue));
    for (i = 0; i < NTXDESC; i++) 
        tx_queue[i].buff_addr = PADDR(tx_buffs[i]);
    
    // Initialize regs of transmit descriptor ring (a.k.a. transmit queue)
    E1000_REG(E1000_TDBAL) = PADDR(tx_queue); 
    E1000_REG(E1000_TDBAH) = 0;
    E1000_REG(E1000_TDLEN) = sizeof(tx_queue);
    E1000_REG(E1000_TDH)   = 0;
    E1000_REG(E1000_TDT)   = 0;
    
    // Program TCTL & TIPG
    E1000_REG(E1000_TCTL) |= E1000_TCTL_EN;
    E1000_REG(E1000_TCTL) |= E1000_TCTL_PSP;

    E1000_REG(E1000_TCTL) &= ~E1000_TCTL_COLD;
    E1000_REG(E1000_TCTL) |= 0x00040000; // TCTL.COLD: 40h

    E1000_REG(E1000_TIPG)  = 10;
    
    return 0;
}

int
e1000_rx_init(void)
{
    int i;
    memset(rx_queue, 0, sizeof(rx_queue));
    for (i = 0; i < NRXDESC; i++)
        rx_queue[i].buff_addr = PADDR(rx_buffs[i]);
    
    // configure the Receive Adress Registers with the card's 
    // own MAC address ( 52:54:00:12:34:56 ) in order to accept
    // packets addressed to the card
    E1000_REG(E1000_RAL0)  = 0x12005452;
    E1000_REG(E1000_RAH0)  = 0x80005634;   
    
    // initialize regs of receive descriptor ring
    E1000_REG(E1000_RDBAL) = PADDR(rx_queue); 
    E1000_REG(E1000_RDBAH) = 0;
    E1000_REG(E1000_RDLEN) = sizeof(rx_queue);
    E1000_REG(E1000_RDH)   = 0;
    E1000_REG(E1000_RDT)   = NRXDESC - 1;
    
    // enable receive
    E1000_REG(E1000_RCTL) |= E1000_RCTL_EN;
    
    // configure e1000 to strip the Ethernet CRC
    E1000_REG(E1000_RCTL) |= E1000_RCTL_SECRC;
     
    return 0;
}

int 
e1000_transmit(const void *data, size_t len) 
{
    uint32_t tail = E1000_REG(E1000_TDT);

    if (len > TX_PKT_BUFF_SIZE)
        return -E_PKT_TOO_LONG;	

    if ((tx_queue[tail].cmd & E1000_TXD_CMD_RS) 
        && !(tx_queue[tail].sta & E1000_TXD_STA_DD))
        return -E_TX_FULL;

    memcpy(tx_buffs[tail], data, len);
    tx_queue[tail].length = len;
    tx_queue[tail].cmd |= E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
    tx_queue[tail].sta &= ~E1000_TXD_STA_DD;

    E1000_REG(E1000_TDT) = (tail + 1) % NTXDESC;
   
    return 0;
}

int 
e1000_receive(void *buff, size_t size)
{
    uint32_t tail = E1000_REG(E1000_RDT);
    uint32_t next = (tail + 1) % NRXDESC;
    int len;

    if (!(rx_queue[next].sta & E1000_RXD_STA_DD))
        return -E_RX_EMPTY;
 
    len = rx_queue[next].length;
    if (size < len)
        return -E_PKT_TOO_LONG;
   
    memcpy(buff, rx_buffs[next], len);
    rx_queue[next].sta &= ~E1000_RXD_STA_DD;

    E1000_REG(E1000_RDT) = next;
    return len;
}
