#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <inc/types.h>

#define PCI_VENDOR_82540EM   0x8086
#define PCI_PRODUCT_82540EM  0x100E

#define E1000_CTRL           0x00000   /* Device Control - R/W */
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
#define E1000_TXD_DTYP_D     0x00100000 /* Data Descriptor */
#define E1000_TXD_DTYP_C     0x00000000 /* Context Descriptor */
#define E1000_TXD_POPTS_IXSM 0x01       /* Insert IP checksum */
#define E1000_TXD_POPTS_TXSM 0x02       /* Insert TCP/UDP checksum */
#define E1000_TXD_CMD_EOP    0x01000000 /* End of Packet */
#define E1000_TXD_CMD_IFCS   0x02000000 /* Insert FCS (Ethernet CRC) */
#define E1000_TXD_CMD_IC     0x04000000 /* Insert Checksum */
#define E1000_TXD_CMD_RS     0x08000000 /* Report Status */
#define E1000_TXD_CMD_RPS    0x10000000 /* Report Packet Sent */
#define E1000_TXD_CMD_DEXT   0x20000000 /* Descriptor extension (0 = legacy) */
#define E1000_TXD_CMD_VLE    0x40000000 /* Add VLAN tag */
#define E1000_TXD_CMD_IDE    0x80000000 /* Enable Tidv register */
#define E1000_TXD_CMD_TCP    0x01000000 /* TCP packet */
#define E1000_TXD_CMD_IP     0x02000000 /* IP packet */
#define E1000_TXD_CMD_TSE    0x04000000 /* TCP Seg enable */
#define E1000_TXD_STA_DD     0x00000001 /* Descriptor Done */
#define E1000_TXD_STA_EC     0x00000002 /* Excess Collisions */
#define E1000_TXD_STA_LC     0x00000004 /* Late Collisions */
#define E1000_TXD_STA_TU     0x00000008 /* Transmit underrun */
#define E1000_TXD_STA_TC     0x00000004 /* Tx Underrun */


/* Transmit Control */
#define E1000_TCTL_RST    0x00000001    /* software reset */
#define E1000_TCTL_EN     0x00000002    /* enable */
#define E1000_TCTL_BCE    0x00000004    /* busy check enable */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_CT     0x00000ff0    /* collision threshold */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */
#define E1000_TCTL_SWXOFF 0x00400000    /* SW Xoff transmission */
#define E1000_TCTL_PBE    0x00800000    /* Packet Burst Enable */
#define E1000_TCTL_RTLC   0x01000000    /* Re-transmit on late collision */
#define E1000_TCTL_NRTU   0x02000000    /* No Re-transmit on underrun */
#define E1000_TCTL_MULR   0x10000000    /* Multiple request support */




volatile uint32_t *e1000_memreg_vaddr;   /* weak symbol */  

#define E1000_REG(offset) e1000_memreg_vaddr[(offset)/4]

#define ETHERNET_PACKET_SIZE  1518 

extern void e1000_tx_init(void);
extern int  e1000_transmit(void *data, size_t len);

#endif	// JOS_KERN_E1000_H
