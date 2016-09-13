#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <inc/types.h>

#define PCI_VENDOR_82540EM   0x8086
#define PCI_PRODUCT_82540EM  0x100E

#define E1000_STATUS         0x00008   /* Device Status  - R   */

#define E1000_TCTL           0x00400   /* Transmit Control - R/W */
#define E1000_TIPG           0x00410   /* Transmit Inter-packet Gap - R/W */

#define E1000_TDBAL          0x03800   /* Transmit Descriptor Base Low  - R/W */
#define E1000_TDBAH          0x03804   /* Transmit Descriptor Base High - R/W */
#define E1000_TDLEN          0x03808   /* Transmit Descriptor Length    - R/W */
#define E1000_TDH            0x03810   /* Transmit Descriptor Head      - R/W */
#define E1000_TDT            0x03818   /* Transmit Descriptor Tail      - R/W */


/* Transmit Descriptor */
struct e1000_tx_desc {
    uint64_t buff_addr;
    uint16_t length;
    uint8_t  cso;
    uint8_t  cmd;
    uint8_t  sta;
    uint8_t  css;
    uint16_t special;
};

/* Transmit Descriptor bit definitions */
#define E1000_TXD_CMD_EOP    0x01 /* End of Packet */
#define E1000_TXD_CMD_RS     0x08 /* Report Status */
#define E1000_TXD_STA_DD     0x01 /* Descriptor Done */


/* Transmit Control */
#define E1000_TCTL_EN     0x00000002    /* enable */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */


volatile uint32_t *e1000_memreg_vaddr;   /* weak symbol, initialized in pci.c */  

#define E1000_REG(offset) e1000_memreg_vaddr[(offset)/4]

#define ETHERNET_PACKET_SIZE  1518 

int e1000_tx_init(void);
int e1000_transmit(const void *data, size_t len);

#endif	// JOS_KERN_E1000_H
