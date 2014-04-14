/* This structure is used in all SIOCxMIIxxx ioctl calls */
struct mii_ioctl_data {
    __u16           phy_id;
    __u16           reg_num;
    __u16           val_in;
    __u16           val_out;
};
#define MII_BMSR            0x01        /* Basic mode status register  */
