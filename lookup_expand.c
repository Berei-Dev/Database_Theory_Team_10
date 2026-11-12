
#include "postgres.h"

#include "access/htup_details.h"
#include "executor/tuptable.h"
#include "utils/hsearch.h"
#include "utils/memutils.h"

#include "lookup_expand.h"


/*
 * BUILD
 *
 * Create a hash table with its own memory context,
 * subordinate to the supplied parent context.
 */
HTAB *
le_build_hash_table(
    int64 expected_entries,
    MemoryContext parent_context)
{
    HASHCTL hash_ctl;
    HTAB *hash_table;

    MemSet(&hash_ctl, 0, sizeof(hash_ctl));

    hash_ctl.keysize = sizeof(int64);
    hash_ctl.entrysize = sizeof(LE_HashEntry);
    hash_ctl.hcxt = parent_context;

    hash_table = hash_create(
        "Lookup Expand Hash Table",
        expected_entries,
        &hash_ctl,
        HASH_ELEM | HASH_BLOBS | HASH_CONTEXT
    );

    return hash_table;
}


/*
 * INSERT
 *
 * Insert one tuple into the hash table.
 *
 * Duplicate keys are supported.
 */
void
le_insert_slot(
    HTAB *hash_table,
    TupleTableSlot *input_slot,
    AttrNumber key_attribute)
{
    bool is_null;
    bool found;

    Datum key_datum;
    int64 key;

    LE_HashEntry *entry;
    LE_Match *match;

    MemoryContext old_context;

    /* Extract join key */
    key_datum = slot_getattr(
        input_slot,
        key_attribute,
        &is_null
    );

    /* Skip NULL join keys */
    if (is_null)
        return;

    key = DatumGetInt64(key_datum);

    /* Find or create hash entry */
    entry = (LE_HashEntry *) hash_search(
        hash_table,
        &key,
        HASH_ENTER,
        &found
    );

    /* Initialize new entry */
    if (!found)
    {
        entry->head = NULL;
        entry->tail = NULL;
    }

    /*
     * Allocate tuple and linked-list node in
     * the hash table's memory context.
     */
    old_context = MemoryContextSwitchTo(hash_table->hcxt);

    match = palloc(sizeof(LE_Match));

    match->tuple = ExecCopySlotMinimalTuple(input_slot);
    match->next = NULL;

    /* Append to linked list */
    if (entry->head == NULL)
    {
        entry->head = match;
        entry->tail = match;
    }
    else
    {
        entry->tail->next = match;
        entry->tail = match;
    }

    MemoryContextSwitchTo(old_context);
}


/*
 * LOOKUP
 *
 * Locate matching records and initialize iterator.
 */
bool
le_lookup(
    HTAB *hash_table,
    int64 key,
    LE_Iterator *iterator)
{
    LE_HashEntry *entry;

    entry = hash_search(
        hash_table,
        &key,
        HASH_FIND,
        NULL
    );

    if (entry == NULL || entry->head == NULL)
    {
        iterator->first = NULL;
        iterator->current = NULL;

        return false;
    }

    iterator->first = entry->head;
    iterator->current = entry->head;

    return true;
}


/*
 * RESET
 *
 * Return iterator to its first matching tuple.
 */
void
le_reset(LE_Iterator *iterator)
{
    iterator->current = iterator->first;
}


/*
 * EXPAND
 *
 * Produce every matching tuple.
 */
void
le_expand(
    LE_Iterator *iterator,
    TupleTableSlot *output_slot,
    void (*produce)(TupleTableSlot *))
{
    while (iterator->current != NULL)
    {
        ExecForceStoreMinimalTuple(
            iterator->current->tuple,
            output_slot,
            false
        );

        produce(output_slot);

        iterator->current = iterator->current->next;
    }
}
