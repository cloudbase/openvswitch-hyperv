typedef struct sock_filter {    /* Filter block */
         __u16   code;   /* Actual filter code */
         __u8    jt;     /* Jump true */
         __u8    jf;     /* Jump false */
         __u32   k;      /* Generic multiuse field */
}sock_filter;

struct sock_fprog {     /* Required for SO_ATTACH_FILTER. */
         unsigned short          len;    /* Number of filter blocks */
         struct sock_filter *filter;
};

/* Socket filtering */
#define SO_ATTACH_FILTER        26
#define SO_DETACH_FILTER        27

#define SO_PEERNAME     28
#define SO_TIMESTAMP        29
#define SCM_TIMESTAMP       SO_TIMESTAMP

#define SO_PEERSEC      30
#define SO_PASSSEC      34
#define SO_TIMESTAMPNS      35
#define SCM_TIMESTAMPNS     SO_TIMESTAMPNS