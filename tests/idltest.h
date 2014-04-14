/* Generated automatically -- do not modify!    -*- buffer-read-only: t -*- */

#ifndef IDLTEST_IDL_HEADER
#define IDLTEST_IDL_HEADER 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ovsdb-data.h"
#include "ovsdb-idl-provider.h"
#include "smap.h"
#include "uuid.h"

/* link1 table. */
struct idltest_link1 {
	struct ovsdb_idl_row header_;

	/* i column. */
	int64_t i;

	/* k column. */
	struct idltest_link1 *k;

	/* ka column. */
	struct idltest_link1 **ka;
	size_t n_ka;

	/* l2 column. */
	struct idltest_link2 *l2;
};

enum {
    IDLTEST_LINK1_COL_I,
    IDLTEST_LINK1_COL_K,
    IDLTEST_LINK1_COL_KA,
    IDLTEST_LINK1_COL_L2,
    IDLTEST_LINK1_N_COLUMNS
};

#define idltest_link1_col_i (idltest_link1_columns[IDLTEST_LINK1_COL_I])
#define idltest_link1_col_k (idltest_link1_columns[IDLTEST_LINK1_COL_K])
#define idltest_link1_col_l2 (idltest_link1_columns[IDLTEST_LINK1_COL_L2])
#define idltest_link1_col_ka (idltest_link1_columns[IDLTEST_LINK1_COL_KA])

extern struct ovsdb_idl_column idltest_link1_columns[IDLTEST_LINK1_N_COLUMNS];

const struct idltest_link1 *idltest_link1_first(const struct ovsdb_idl *);
const struct idltest_link1 *idltest_link1_next(const struct idltest_link1 *);
#define IDLTEST_LINK1_FOR_EACH(ROW, IDL) \
        for ((ROW) = idltest_link1_first(IDL); \
             (ROW); \
             (ROW) = idltest_link1_next(ROW))
#define IDLTEST_LINK1_FOR_EACH_SAFE(ROW, NEXT, IDL) \
        for ((ROW) = idltest_link1_first(IDL); \
             (ROW) ? ((NEXT) = idltest_link1_next(ROW), 1) : 0; \
             (ROW) = (NEXT))

void idltest_link1_init(struct idltest_link1 *);
void idltest_link1_delete(const struct idltest_link1 *);
struct idltest_link1 *idltest_link1_insert(struct ovsdb_idl_txn *);

void idltest_link1_verify_i(const struct idltest_link1 *);
void idltest_link1_verify_k(const struct idltest_link1 *);
void idltest_link1_verify_ka(const struct idltest_link1 *);
void idltest_link1_verify_l2(const struct idltest_link1 *);

/* Functions for fetching columns as "struct ovsdb_datum"s.  (This is
   rarely useful.  More often, it is easier to access columns by using
   the members of idltest_link1 directly.) */
const struct ovsdb_datum *idltest_link1_get_i(const struct idltest_link1 *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_link1_get_k(const struct idltest_link1 *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_link1_get_ka(const struct idltest_link1 *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_link1_get_l2(const struct idltest_link1 *, enum ovsdb_atomic_type key_type);

void idltest_link1_set_i(const struct idltest_link1 *, int64_t i);
void idltest_link1_set_k(const struct idltest_link1 *, const struct idltest_link1 *k);
void idltest_link1_set_ka(const struct idltest_link1 *, struct idltest_link1 **ka, size_t n_ka);
void idltest_link1_set_l2(const struct idltest_link1 *, const struct idltest_link2 *l2);


/* link2 table. */
struct idltest_link2 {
	struct ovsdb_idl_row header_;

	/* i column. */
	int64_t i;

	/* l1 column. */
	struct idltest_link1 *l1;
};

enum {
    IDLTEST_LINK2_COL_I,
    IDLTEST_LINK2_COL_L1,
    IDLTEST_LINK2_N_COLUMNS
};

#define idltest_link2_col_i (idltest_link2_columns[IDLTEST_LINK2_COL_I])
#define idltest_link2_col_l1 (idltest_link2_columns[IDLTEST_LINK2_COL_L1])

extern struct ovsdb_idl_column idltest_link2_columns[IDLTEST_LINK2_N_COLUMNS];

const struct idltest_link2 *idltest_link2_first(const struct ovsdb_idl *);
const struct idltest_link2 *idltest_link2_next(const struct idltest_link2 *);
#define IDLTEST_LINK2_FOR_EACH(ROW, IDL) \
        for ((ROW) = idltest_link2_first(IDL); \
             (ROW); \
             (ROW) = idltest_link2_next(ROW))
#define IDLTEST_LINK2_FOR_EACH_SAFE(ROW, NEXT, IDL) \
        for ((ROW) = idltest_link2_first(IDL); \
             (ROW) ? ((NEXT) = idltest_link2_next(ROW), 1) : 0; \
             (ROW) = (NEXT))

void idltest_link2_init(struct idltest_link2 *);
void idltest_link2_delete(const struct idltest_link2 *);
struct idltest_link2 *idltest_link2_insert(struct ovsdb_idl_txn *);

void idltest_link2_verify_i(const struct idltest_link2 *);
void idltest_link2_verify_l1(const struct idltest_link2 *);

/* Functions for fetching columns as "struct ovsdb_datum"s.  (This is
   rarely useful.  More often, it is easier to access columns by using
   the members of idltest_link2 directly.) */
const struct ovsdb_datum *idltest_link2_get_i(const struct idltest_link2 *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_link2_get_l1(const struct idltest_link2 *, enum ovsdb_atomic_type key_type);

void idltest_link2_set_i(const struct idltest_link2 *, int64_t i);
void idltest_link2_set_l1(const struct idltest_link2 *, const struct idltest_link1 *l1);


/* simple table. */
struct idltest_simple {
	struct ovsdb_idl_row header_;

	/* b column. */
	bool b;

	/* ba column. */
	bool *ba;
	size_t n_ba;

	/* i column. */
	int64_t i;

	/* ia column. */
	int64_t *ia;
	size_t n_ia;

	/* r column. */
	double r;

	/* ra column. */
	double *ra;
	size_t n_ra;

	/* s column. */
	char *s;	/* Always nonnull. */

	/* sa column. */
	char **sa;
	size_t n_sa;

	/* u column. */
	struct uuid u;

	/* ua column. */
	struct uuid *ua;
	size_t n_ua;
};

enum {
    IDLTEST_SIMPLE_COL_B,
    IDLTEST_SIMPLE_COL_BA,
    IDLTEST_SIMPLE_COL_I,
    IDLTEST_SIMPLE_COL_IA,
    IDLTEST_SIMPLE_COL_R,
    IDLTEST_SIMPLE_COL_RA,
    IDLTEST_SIMPLE_COL_S,
    IDLTEST_SIMPLE_COL_SA,
    IDLTEST_SIMPLE_COL_U,
    IDLTEST_SIMPLE_COL_UA,
    IDLTEST_SIMPLE_N_COLUMNS
};

#define idltest_simple_col_b (idltest_simple_columns[IDLTEST_SIMPLE_COL_B])
#define idltest_simple_col_ba (idltest_simple_columns[IDLTEST_SIMPLE_COL_BA])
#define idltest_simple_col_i (idltest_simple_columns[IDLTEST_SIMPLE_COL_I])
#define idltest_simple_col_s (idltest_simple_columns[IDLTEST_SIMPLE_COL_S])
#define idltest_simple_col_r (idltest_simple_columns[IDLTEST_SIMPLE_COL_R])
#define idltest_simple_col_u (idltest_simple_columns[IDLTEST_SIMPLE_COL_U])
#define idltest_simple_col_ra (idltest_simple_columns[IDLTEST_SIMPLE_COL_RA])
#define idltest_simple_col_ia (idltest_simple_columns[IDLTEST_SIMPLE_COL_IA])
#define idltest_simple_col_sa (idltest_simple_columns[IDLTEST_SIMPLE_COL_SA])
#define idltest_simple_col_ua (idltest_simple_columns[IDLTEST_SIMPLE_COL_UA])

extern struct ovsdb_idl_column idltest_simple_columns[IDLTEST_SIMPLE_N_COLUMNS];

const struct idltest_simple *idltest_simple_first(const struct ovsdb_idl *);
const struct idltest_simple *idltest_simple_next(const struct idltest_simple *);
#define IDLTEST_SIMPLE_FOR_EACH(ROW, IDL) \
        for ((ROW) = idltest_simple_first(IDL); \
             (ROW); \
             (ROW) = idltest_simple_next(ROW))
#define IDLTEST_SIMPLE_FOR_EACH_SAFE(ROW, NEXT, IDL) \
        for ((ROW) = idltest_simple_first(IDL); \
             (ROW) ? ((NEXT) = idltest_simple_next(ROW), 1) : 0; \
             (ROW) = (NEXT))

void idltest_simple_init(struct idltest_simple *);
void idltest_simple_delete(const struct idltest_simple *);
struct idltest_simple *idltest_simple_insert(struct ovsdb_idl_txn *);

void idltest_simple_verify_b(const struct idltest_simple *);
void idltest_simple_verify_ba(const struct idltest_simple *);
void idltest_simple_verify_i(const struct idltest_simple *);
void idltest_simple_verify_ia(const struct idltest_simple *);
void idltest_simple_verify_r(const struct idltest_simple *);
void idltest_simple_verify_ra(const struct idltest_simple *);
void idltest_simple_verify_s(const struct idltest_simple *);
void idltest_simple_verify_sa(const struct idltest_simple *);
void idltest_simple_verify_u(const struct idltest_simple *);
void idltest_simple_verify_ua(const struct idltest_simple *);

/* Functions for fetching columns as "struct ovsdb_datum"s.  (This is
   rarely useful.  More often, it is easier to access columns by using
   the members of idltest_simple directly.) */
const struct ovsdb_datum *idltest_simple_get_b(const struct idltest_simple *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_simple_get_ba(const struct idltest_simple *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_simple_get_i(const struct idltest_simple *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_simple_get_ia(const struct idltest_simple *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_simple_get_r(const struct idltest_simple *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_simple_get_ra(const struct idltest_simple *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_simple_get_s(const struct idltest_simple *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_simple_get_sa(const struct idltest_simple *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_simple_get_u(const struct idltest_simple *, enum ovsdb_atomic_type key_type);
const struct ovsdb_datum *idltest_simple_get_ua(const struct idltest_simple *, enum ovsdb_atomic_type key_type);

void idltest_simple_set_b(const struct idltest_simple *, bool b);
void idltest_simple_set_ba(const struct idltest_simple *, const bool *ba, size_t n_ba);
void idltest_simple_set_i(const struct idltest_simple *, int64_t i);
void idltest_simple_set_ia(const struct idltest_simple *, const int64_t *ia, size_t n_ia);
void idltest_simple_set_r(const struct idltest_simple *, double r);
void idltest_simple_set_ra(const struct idltest_simple *, const double *ra, size_t n_ra);
void idltest_simple_set_s(const struct idltest_simple *, const char *s);
void idltest_simple_set_sa(const struct idltest_simple *, char **sa, size_t n_sa);
void idltest_simple_set_u(const struct idltest_simple *, struct uuid u);
void idltest_simple_set_ua(const struct idltest_simple *, const struct uuid *ua, size_t n_ua);


enum {
    IDLTEST_TABLE_LINK1,
    IDLTEST_TABLE_LINK2,
    IDLTEST_TABLE_SIMPLE,
    IDLTEST_N_TABLES
};

#define idltest_table_simple (idltest_table_classes[IDLTEST_TABLE_SIMPLE])
#define idltest_table_link1 (idltest_table_classes[IDLTEST_TABLE_LINK1])
#define idltest_table_link2 (idltest_table_classes[IDLTEST_TABLE_LINK2])

extern struct ovsdb_idl_table_class idltest_table_classes[IDLTEST_N_TABLES];

extern struct ovsdb_idl_class idltest_idl_class;

void idltest_init(void);

#endif /* IDLTEST_IDL_HEADER */
