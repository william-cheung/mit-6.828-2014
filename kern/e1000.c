#include <kern/e1000.h>

#include <inc/string.h>
#include <inc/error.h>

#include <kern/pmap.h>

#define NTXDESC  16
static struct e1000_tx_desc tx_desc_ring[NTXDESC] __attribute__((aligned(16)));

#define TX_PKT_BUFF_SIZE ETHERNET_PACKET_SIZE
static uint8_t tx_buffs[NTXDESC][TX_PKT_BUFF_SIZE];

void
e1000_tx_init() 
{
    int i;
	
	assert(sizeof(struct e1000_tx_desc) == 16);
	assert(((uint32_t)(&tx_desc_ring[0]) & 0xf) == 0);
	assert(sizeof(tx_desc_ring) % 128 == 0);
	
    memset(tx_desc_ring, 0, sizeof(tx_desc_ring));
    for (i = 0; i < NTXDESC; i++)
        tx_desc_ring[i].buff_addr = PADDR(&tx_buffs[i]);

    E1000_REG32_LVAL(E1000_TDBAL) = PADDR(&tx_desc_ring[0]); 
    E1000_REG32_LVAL(E1000_TDBAH) = 0;
    E1000_REG32_LVAL(E1000_TDLEN) = sizeof(tx_desc_ring);
    E1000_REG32_LVAL(E1000_TDH)   = 0;
    E1000_REG32_LVAL(E1000_TDT)   = 0;

	E1000_REG32_LVAL(E1000_TCTL)  = 0;
    E1000_REG32_LVAL(E1000_TCTL) |= E1000_TCTL_EN;
    E1000_REG32_LVAL(E1000_TCTL) |= E1000_TCTL_PSP;
	//E1000_REG32_LVAL(E1000_TCTL) |= 0x00000100; // TCTL.CT: 10h
    E1000_REG32_LVAL(E1000_TCTL) |= 0x00040000; // TCTL.COLD: 40h

    E1000_REG32_LVAL(E1000_TIPG)  = 10;
}

int 
e1000_transmit(void *data, size_t len) 
{
    volatile uint32_t *tail = &E1000_REG32_LVAL(E1000_TDT);
    struct e1000_tx_desc *desc = &tx_desc_ring[*tail];
	
	if (len > TX_PKT_BUFF_SIZE)
		return -E_PKT_TOO_LONG;	

    if ((desc->cmd & E1000_TXD_CMD_RS) && !(desc->sta & E1000_TXD_STA_DD))
        return -E_TX_FULL;

    memcpy((void *)tx_buffs[*tail], data, len);
    desc->length = len;
    desc->cmd |= E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
    desc->sta &= ~E1000_TXD_STA_DD;
    *tail = (*tail + 1) % NTXDESC;
    return 0;
}
