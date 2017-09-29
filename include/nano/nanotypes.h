/*-
 * Copyright (c) 2017 Lawrence Esswood
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract (FA8750-10-C-0237)
 * ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef CHERIOS_NANOTYPES_H
#define CHERIOS_NANOTYPES_H

#include "string_enums.h"

// FIXME we need to choose appropriate types and remove their accessibility from the rest of the system

/* The types used for nano kernel objects. */
#define CONTEXT_TYPE       0x5555         // The type of contexts
#define NANO_KERNEL_TYPE   0x6666         // The type of sealed local data
#define RES_TYPE           0x7777         // The type of a reservation handle
#define FOUND_ENTRY_TYPE   0x9990         // The type of a foundation entry handle
#define FOUND_CERT_TYPE    0x9991         // The type of a foundation certificate handle
#define FOUND_LOCKED_TYPE  0x9992         // The type of a foundation locked message hnadle
#define VTABLE_TYPE_L0     0x8880         // The type of the top level page table
#define VTABLE_TYPE_L1     VTABLE_TYPE_L0 + 1  // The type of the L1 level page table
#define VTABLE_TYPE_L2     VTABLE_TYPE_L0 + 2  // The type of the L2 level page table

#define VTABLE_LEVELS      3              // Number of table levels


/* Size of metadata for reservations. Split into private and user data */
#define RES_PRIV_SIZE                   (32)
#define RES_META_SIZE                   (64)
#define RES_USER_SIZE                   (RES_META_SIZE - RES_PRIV_SIZE)


#define RES_LENGTH_OFFSET               0
#define RES_PID_OFFSET                  8
#define RES_STATE_OFFSET                31
#define STORE_RES_STATE                 csb
#define LOAD_RES_STATE                  clb
#define RES_SUBFIELD_SIZE_OFFSET        30
#define RES_SUBFIELD_BITMAP_OFFSET      16
#define RES_SUBFIELD_BITMAP_BITS        (14 * 8)


//TODO make this dynamic
/* Page sizes etc */
#define PHY_MEM_SIZE                    (1L << 32)


/* TODO: Would like to increase this but QEMU does not seem to support a page pask not 0 */
#define PHY_PAGE_SIZE_BITS              (12)
#define PHY_PAGE_SIZE                   (1 << PHY_PAGE_SIZE_BITS)
#define TOTAL_PHY_PAGES                 (PHY_MEM_SIZE/PAGE_SIZE)

#define PHY_ADDR_TO_PAGEN(addr)         ((addr >> PHY_PAGE_SIZE_BITS) & (TOTAL_PHY_PAGES-1))


/* Physical page records */
#define PHY_PAGE_ENTRY_SIZE_BITS        (REG_SIZE_BITS + 2)
#define PHY_PAGE_ENTRY_SIZE             (1L << PHY_PAGE_ENTRY_SIZE_BITS)

/* Virtual page table records */

/* In this version we are using one physical page for each page table at each level */

#define PAGE_TABLE_BITS                 PHY_PAGE_SIZE_BITS
#define PAGE_TABLE_SIZE                 PHY_PAGE_SIZE
#define PAGE_TABLE_ENT_SIZE             REG_SIZE
#define PAGE_TABLE_ENT_BITS             REG_SIZE_BITS
#define PAGE_TABLE_ENT_PER_TABLE        (PAGE_TABLE_SIZE / PAGE_TABLE_ENT_SIZE)
#define PAGE_TABLE_BITS_PER_LEVEL       (PAGE_TABLE_BITS - PAGE_TABLE_ENT_BITS)

#define L0_BITS                         PAGE_TABLE_BITS_PER_LEVEL
#define L1_BITS                         PAGE_TABLE_BITS_PER_LEVEL
#define L2_BITS                         PAGE_TABLE_BITS_PER_LEVEL
#define UNTRANSLATED_BITS               (1 + PHY_PAGE_SIZE_BITS) /* +1 for having two PFNs per VPN */

#define TRANSLATED_BITS                 (L0_BITS + L1_BITS + L2_BITS)
#define MAX_VIRTUAL_PAGES               (1 << TRANSLATED_BITS)

#define UNTRANSLATED_PAGE_SIZE          (1 << UNTRANSLATED_BITS)

#define VTABLE_ENTRY_USED               (-1)

#define PFN_SHIFT                       6
/* These bits will eventually be untranslated high bits, but we will check they are equal to a field in the leaf
 * Of the page table. These could be considered a generation count. */

#define CHECKED_BITS                    (64 - L0_BITS - L1_BITS - L2_BITS - UNTRANSLATED_BITS)


#define L0_INDEX(addr)          ((addr << CHECKED_BITS) >> (CHECKED_BITS + L1_BITS + L2_BITS + UNTRANSLATED_BITS))
#define L1_INDEX(addr)          ((addr << CHECKED_BITS + L0_BITS) >> (CHECKED_BITS + L0_BITS + L2_BITS + UNTRANSLATED_BITS))
#define L2_INDEX(addr)          ((addr << CHECKED_BITS + L0_BITS + L1_BITS) >> (CHECKED_BITS + L0_BITS + L1_BITS + UNTRANSLATED_BITS))
#define PAGE_INDEX(addr)        (addr & (PHY_PAGE_SIZE - 1))
#define GENERATION_COUNT(addr)  (addr >> (L0_BITS + L1_BITS + L2_BITS + UNTRANSLATED_BITS))

#define PAGE_SIZE                 (PHY_PAGE_SIZE)

#define NANO_KERNEL_PAGE_STATUS_ENUM_LIST(ITEM)    \
    ITEM(page_unused, 0)                           \
    ITEM(page_nano_owned, 1)                       \
    ITEM(page_system_owned, 2)                     \
    ITEM(page_mapped, 3)                           \
    ITEM(page_ptable, 4)                           \
    ITEM(page_ptable_free, 5)                      \
    ITEM(page_transaction, 6)                      \
    ITEM(page_io, 7)

#define NANO_KERNEL_RES_STATUS_ENUM_LIST(ITEM) \
    ITEM(res_open,          0)                  \
    ITEM(res_taken,         1)                  \
    ITEM(res_merged,        2)                  \
    ITEM(res_revoking,      3)

DECLARE_ENUM(e_res_status, NANO_KERNEL_RES_STATUS_ENUM_LIST)

DECLARE_ENUM(e_page_status, NANO_KERNEL_PAGE_STATUS_ENUM_LIST)

/* Stuff to do with the foundation system */

#define FOUNDATION_ID_SIZE                      (32 + (8 * 4))      // sha256 + other fields
#define FOUNDATION_ID_LEN_OFFSET                32
#define FOUNDATION_ID_E0_OFFSET                 40
#define FOUNDATION_ID_NENT_OFFSET               48
#define FOUNDATION_META_DATA_OFFSET             FOUNDATION_ID_SIZE
#define FOUNDATION_META_ENTRY_VECTOR_OFFSET     (FOUNDATION_ID_SIZE + CAP_SIZE)
#define FOUNDATION_META_SIZE(N)                 (FOUNDATION_ID_SIZE + CAP_SIZE + (N * CAP_SIZE))

#define RES_CERT_META_SIZE                      (3 * CAP_SIZE)

#ifndef __ASSEMBLY__

#include "cheric.h"

#define REG_SIZE        sizeof(register_t)
#define REG_SIZE_BITS   3

_Static_assert((1 << REG_SIZE_BITS) == REG_SIZE, "This should be true");

#define BOOK_END                        ((size_t)(TOTAL_PHY_PAGES))

/* WARN: these structures are used in assembly */

typedef capability context_t;               // Type of a nanokernel context handle
typedef capability res_t;                   // Type of a reservation handle
typedef capability ptable_t;                // Type of a page table handle


typedef capability entry_t;                 // Type of a foundation entry handle
typedef capability cert_t;                  // A certificate for capability
typedef capability locked_t;                // A locked capability

/* Identifying information for a foundation */
typedef struct found_id_t {
    char sha256[256/8];
    size_t length;
    size_t e0;
    size_t nentries;
    size_t pad;
} found_id_t;

/* This is how big the structure is in the nano kernel */
_Static_assert(sizeof(found_id_t) == FOUNDATION_ID_SIZE, "Assumed by nano kernel");

typedef capability cert_t;                  // A certified capability
typedef capability locked_t;               // A capability that can be unlocked by intended code

/* This structure can't actually be read from c, its just here in case you want to better understand how these
 * are used in the nano kernel pid is 'parent id'. Reservations can only be merged if their pid is the same
 * and are adjacent. \* /*
struct res_str {
    struct res_str_priv {
        struct res_str_used {
            enum {
                res_state_open,
                res_state_taken,
                res_state_merged,
                res_state_revoking
            } state;                                            // 64 bit
            size_t length;                                      // Length of data
            size_t pid;                                         // Parent ID. pid must match for merge.
        };
        char padding[RES_PRIV_SIZE - sizeof(struct res_str_used)]; // Un-used
    } priv;
    struct res_str_user {
        char user_defined[RES_USER_SIZE];                       // The result of calling get_userdata_for_res
    } user;
};
*/

typedef struct {
    e_page_status	status;
    size_t	len; /* number of pages in this chunk */
    size_t	prev; /* start of previous chunk */
    size_t  spare; /* Will probably use this to store a VPN or user data */
} page_t;

typedef register_t table_entry_t;
typedef struct {
    table_entry_t entries[PAGE_TABLE_ENT_PER_TABLE];
} readable_table_t;

_Static_assert(sizeof(table_entry_t) == PAGE_TABLE_ENT_SIZE, "Used by nano kernel");

/* This is how big the structure is in the nano kernel */
_Static_assert(sizeof(page_t) == PHY_PAGE_ENTRY_SIZE, "Assumed by nano kernel");

typedef struct {
    context_t victim_context;
    register_t cause;
    register_t ccause;
    register_t badvaddr;
    register_t ex_level;
} exection_cause_t;

typedef register_t ex_lvl_t;
typedef register_t cause_t;

#endif // __ASSEMBLY__

#endif //CHERIOS_NANOTYPES_H