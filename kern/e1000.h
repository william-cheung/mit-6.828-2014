#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <inc/types.h>

#define PCI_VENDOR_82540EM   0x8086
#define PCI_PRODUCT_82540EM  0x100E

#define E1000_CTRL           0x00000   /* Device Control - R/W */
#define E1000_STATUS         0x00008   /* Device Status  - R   */

#define E1000_TCTL           0x00400   /* Transmit Control - R/W */

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



volatile uint8_t *e1000_memreg_vaddr;   /* weak symbol */  

#define E1000_REG_LVAL(type, offset) (*(type *)(e1000_memreg_vaddr + (offset)))
#define E1000_REG32_LVAL(offset) E1000_REG_LVAL(uint32_t, offset)
#define E1000_REG64_LVAL(offset) E1000_REG_LVAL(uint64_t, offset)

#endif	// JOS_KERN_E1000_H
