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