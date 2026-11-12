
#ifndef LOOKUP_EXPAND_H
#define LOOKUP_EXPAND_H

#include "postgres.h"
#include "access/htup_details.h"
#include "executor/tuptable.h"
#include "utils/hsearch.h"
#include "utils/memutils.h"

/*
 * One tuple stored in a hash entry.
 *
 * MinimalTuple is an independent copy of the source tuple.
 */
typedef struct LE_Match
{
    MinimalTuple tuple;
    struct LE_Match *next;
} LE_Match;


/*
 * Hash table entry.
 *
 * The key must be the first field for HASH_BLOBS.
 */
typedef struct LE_HashEntry
{
    int64 key;

    LE_Match *head;
    LE_Match *tail;

} LE_HashEntry;


/*
 * Lookup result.
 */
typedef struct LE_Iterator
{
    LE_Match *first;
    LE_Match *current;

} LE_Iterator;


/*
 * Hash table build.
 */
extern HTAB *le_build_hash_table(
    int64 expected_entries,
    MemoryContext parent_context
);


/*
 * Lookup.
 */
extern bool le_lookup(
    HTAB *hash_table,
    int64 key,
    LE_Iterator *iterator
);

extern void le_reset(
    LE_Iterator *iterator;
)

/*
 * Expand.
 */
extern void le_expand(
    LE_Iterator *iterator,
    TupleTableSlot *output_slot,
    void (*produce)(TupleTableSlot *)
);

extern void le_insert_slot(
    HTAB *hash_table,
    TupleTableSlot *input_slot,
    AttrNumber key_attribute
);

#endif
