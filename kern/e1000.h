#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <inc/types.h>

#define PCI_VENDOR_82540EM   0x8086
#define PCI_PRODUCT_82540EM  0x100E

#define E1000_STATUS         0x00008   /* Device Status  - R   */

#define E1000_TDBAL          0x03800   /* Transmit Descriptor Base Low  - R/W */
#define E1000_TDBAH          0x03804   /* Transmit Descriptor Base High - R/W */
#define E1000_TDLEN          0x03808   /* Transmit Descriptor Length    - R/W */
#define E1000_TDH            0x03810   /* Transmit Descriptor Head      - R/W */
#define E1000_TDT            0x03818   /* Transmit Descriptor Tail      - R/W */


/* Transmit Descriptor */
struct e1000_tx_desc {
    uint64_t buff_addr;  /* data buffer address */
    uint16_t length;     /* data buffer length */
    uint8_t  cso;        /* checksum offset */
    uint8_t  cmd;        /* descriptor control */
    uint8_t  sta;        /* descriptor status */
    uint8_t  css;        /* checksum start */
    uint16_t special;    
};

/* Transmit Descriptor bit definitions */
#define E1000_TXD_CMD_EOP    0x01 /* End of Packet */
#define E1000_TXD_CMD_RS     0x08 /* Report Status */
#define E1000_TXD_STA_DD     0x01 /* Descriptor Done */



/* Transmit Control */
#define E1000_TCTL           0x00400    /* Transmit Control - R/W */
#define E1000_TCTL_EN     0x00000002    /* enable */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */


#define E1000_TIPG           0x00410    /* Transmit Inter-packet Gap - R/W */


/* Receive Control */
#define E1000_RCTL                 0x00100  /* Receive Control - R/W */
#define E1000_RCTL_EN           0x00000002  /* enable */
#define E1000_RCTL_LPE          0x00000020  /* long packet enable */
#define E1000_RCTL_LBM_NO       0x00000000  /* no loopback mode */
#define E1000_RCTL_LBM_MAC      0x00000040  /* mac loopback mode */
#define E1000_RCTL_LBM_SLP      0x00000080  /* serial loopback mode */
#define E1000_RCTL_LBM_TCVR     0x000000C0  /* tcvr loopback mode */
#define E1000_RCTL_RDMTS_HALF   0x00000000  /* rx desc min threshold size */
#define E1000_RCTL_RDMTS_QUAT   0x00000100
#define E1000_RCTL_RDMTS_EIGTH  0x00000200      
#define E1000_RCTL_MO_SHIFT     12          /* multicast offset shift */
#define E1000_RCTL_MO_0         0x00000000  /* multicast offset */
#define E1000_RCTL_MO_1         0x00001000
#define E1000_RCTL_MO_2         0x00002000
#define E1000_RCTL_MO_3         0x00003000
#define E1000_RCTL_BAM          0x00008000  /* broadcast enable */ 
#define E1000_RCTL_SZ_2048      0x00000000  /* rx buffer size 2048 */ 
#define E1000_RCTL_BSEX         0x02000000  /* buffer size extension */
#define E1000_RCTL_SECRC        0x04000000  /* strip ethernet crc */

#define E1000_RAL0           0x05400   /* Receive Address Low  - R/W */
#define E1000_RAH0           0x05404   /* Receive Adreess High - R/W */

#define E1000_RDBAL          0x02800   /* Receive Descriptor Base Low  - R/W */
#define E1000_RDBAH          0x02804   /* Receive Descriptor Base High - R/W */
#define E1000_RDLEN          0x02808   /* Receive Descriptor Length    - R/W */
#define E1000_RDH            0x02810   /* Receive Descriptor Head      - R/W */
#define E1000_RDT            0x02818   /* Receive Descriptor Tail      - R/W */


/* Receive Descriptor */
struct e1000_rx_desc {
    uint64_t buff_addr;   /* data buffer address */
    uint16_t length;      /* data buffer length */
    uint16_t csm;         /* packet checksum */
    uint8_t  sta;         /* descriptor status */
    uint8_t  err;         /* descriptor errors */
    uint16_t special;
};

/* Receive Descriptor bit definitions */
#define E1000_RXD_STA_DD     0x01 /* Descriptor Done */
#define E1000_RXD_STA_EOP    0x02 /* End of Packet */


volatile uint32_t *e1000_memreg_vaddr;   /* weak symbol, initialized in pci.c */  

#define E1000_REG(offset) e1000_memreg_vaddr[(offset)/4]

#define ETHERNET_PACKET_SIZE  1518 

int e1000_tx_init(void);
int e1000_rx_init(void);
int e1000_transmit(const void *data, size_t len);

#endif	// JOS_KERN_E1000_H
