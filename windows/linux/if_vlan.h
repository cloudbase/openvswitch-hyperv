enum vlan_ioctl_cmds {
        ADD_VLAN_CMD,
        DEL_VLAN_CMD,
        SET_VLAN_INGRESS_PRIORITY_CMD,
        SET_VLAN_EGRESS_PRIORITY_CMD,
        GET_VLAN_INGRESS_PRIORITY_CMD,
        GET_VLAN_EGRESS_PRIORITY_CMD,
        SET_VLAN_NAME_TYPE_CMD,
        SET_VLAN_FLAG_CMD,
        GET_VLAN_REALDEV_NAME_CMD, /* If this works, you know it's a VLAN device, btw */
        GET_VLAN_VID_CMD /* Get the VID of this VLAN (specified by name) */
};

struct vlan_ioctl_args {
        int cmd; /* Should be one of the vlan_ioctl_cmds enum above. */
        char device1[24];
        union {
               char device2[24];
               int VID;
               unsigned int skb_priority;
               unsigned int name_type;
               unsigned int bind_type;
               unsigned int flag; /* Matches vlan_dev_priv flags */
		} u;
       short vlan_qos;
};