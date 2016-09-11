#include <kern/e1000.h>

#include <inc/string.h>

#include <kern/pmap.h>

#define NTXDESC  16
static struct e1000_tx_desc tx_desc_ring[NTXDESC];

static uint8_t e1000_tx_buffs[NTXDESC][ETHERNET_PACKET_SIZE];

void
e1000_tx_init() 
{
    int i;

    memset(tx_desc_ring, 0, sizeof(tx_desc_ring));
    for (i = 0; i < NTXDESC; i++) {
        tx_desc_ring[i].buff_addr = PADDR(&e1000_tx_buffs[i]);
        tx_desc_ring[i].length = ETHERNET_PACKET_SIZE;
    }

    E1000_REG32_LVAL(E1000_TDBAL) = PADDR(&tx_desc_ring[0]); 
    //E1000_REG32_LVAL(E1000_TDBAH) = 0;
    E1000_REG32_LVAL(E1000_TDLEN) = sizeof(tx_desc_ring);
    E1000_REG32_LVAL(E1000_TDH)   = 0;
    E1000_REG32_LVAL(E1000_TDT)   = 0;

    E1000_REG32_LVAL(E1000_TCTL) |= E1000_TCTL_EN;
    E1000_REG32_LVAL(E1000_TCTL) |= E1000_TCTL_PSP;
    E1000_REG32_LVAL(E1000_TCTL) |= (0x40 << 12);

    E1000_REG32_LVAL(E1000_TIPG)  = 10;
}

int 
e1000_try_send(void *packet, size_t len) 
{
    volatile uint32_t *tail = &E1000_REG32_LVAL(E1000_TDT);
    struct e1000_tx_desc *desc = &tx_desc_ring[*tail];

    if ((desc->cmd & E1000_TXD_CMD_RS) && !(desc->sta & E1000_TXD_STA_DD))
        return -1;

    memcpy((void *)e1000_tx_buffs[*tail], packet, len);
    desc->length = len;
    desc->cmd |= E1000_TXD_CMD_RS;
    desc->sta &= ~E1000_TXD_STA_DD;
    *tail = (*tail + 1) % NTXDESC;
    return 0;
}
