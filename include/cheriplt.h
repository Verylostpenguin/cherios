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

#ifndef CHERIOS_CHERIPLT_H
#define CHERIOS_CHERIPLT_H

// While we do not have linker support, this header is used to create a PLT like construct, but every entry in the
// .plt.got is a SEALED pointer.

// PLT_WTYPE will define a struct representing the plt, it is up to the user to create one of these
// It will also create a set of ccall methods for each member of the struct.
// It will also create a set of default objects for each member
// Again, it is up to the user to populate these default objects

// A helpful init method is also provided. It will populate the default objects from the struct, with a default data

#ifndef __ASSEMBLY__

#include "cheric.h"
#include "ccall.h"
#include "utils.h"
#include "types.h"
#include "dylink.h"

// FIXME: alias needs size too
#define WEAK_DUMMY(name) ".weak " #name "_dummy \n .hidden " #name "_dummy \n"

#if (DEBUG_COUNT_CALLS)

#if (IS_KERNEL)
#define GET_STATS   "clcbi   $c15, " X_STRINGIFY(CTLP_OFFSET_CGP) "($idc) \n"\
                    "cbtu $c15, 77f \n "\
                    "nop \n"\
                    "clcbi	$c15, %captab20(own_stats)($c15)\n"
#else
#define GET_STATS ".type own_stats, \"tls_object\"\n clcbi $c15, %captab_tls20(own_stats)($c26)\n"
#endif

#define BUMP_CSD_COUNTER GET_STATS \
        "clcbi   $c15, 0($c15) \n"\
        "cbtu    $c15, 77f\n"\
        "nop\n"\
        "cld     $t0, $zero, " X_STRINGIFY(STATS_COMMON_DOMAIN_OFFSET) "($c15)\n"\
        "daddiu  $t0, $t0, 1\n"\
        "csd     $t0, $zero, " X_STRINGIFY(STATS_COMMON_DOMAIN_OFFSET) "($c15)\n"\
        "77:\n"

#else
#define BUMP_CSD_COUNTER
#endif


#define PLT_STUB_CGP_ONLY_CSD(name, obj, tls, tls_reg, alias, alias2) \
__asm__ (                       \
    SANE_ASM                    \
    ".text\n"                   \
    ".p2align 3\n"              \
    ".global " #name "\n"       \
    ".ent " #name "\n"          \
    X_STRINGIFY(ASM_VISIBILITY) " " #name "\n"\
    "" #name ":\n"              \
    alias                       \
    WEAK_DUMMY(name)            \
    BUMP_CSD_COUNTER            \
    "clcbi       $c1, %capcall20(" #name "_dummy)($c25)\n"      \
    "clcbi       $c2, %captab" tls "20(" EVAL1(STRINGIFY(obj)) ")(" tls_reg ")\n"   \
    "ccall       $c1, $c2, 2 \n"\
    "nop\n"                     \
    alias2                      \
    ".end " #name "\n"          \
);

#define PLT_STUB_CGP_ONLY_MODE_SEL(name, obj, tls, tls_reg, alias, alias2) \
__asm__ (                       \
    SANE_ASM                    \
    ".text\n"                   \
    ".p2align 3\n"              \
    ".global " #name "\n"       \
    ".ent " #name "\n"          \
    X_STRINGIFY(ASM_VISIBILITY) " " #name "\n"\
    "" #name ":\n"              \
    alias                       \
    WEAK_DUMMY(name)            \
    WEAK_DUMMY(obj)             \
    "clcbi       $c1, %capcall20(" #name "_dummy)($c25)\n" \
    "clcbi       $c12,%capcall20(" EVAL1(STRINGIFY(obj)) "_dummy)($c25)\n"          \
    "cjr         $c12                                 \n"                           \
    "clcbi       $c2, %captab" tls "20(" EVAL1(STRINGIFY(obj)) ")(" tls_reg ")\n"   \
    alias2                      \
    ".end " #name "\n"          \
);

typedef void common_t(void);

    #define PLT_GOT_ENTRY(name, ...) capability name;

    #define PLT_UNIQUE_OBJECT(name) name ## _data_obj

    #define DECLARE_STUB(name, ret, sig, ...) extern ret name sig; extern struct pltstub256 name ## _data;

    #define GET_ALIAS(X, ...) X
    #define GET_ALIAS2(Y,X,...) X
    #define DEFINE_STUB(name, ret, sig, type, ST, tls, tls_reg, ...) ST(name, PLT_UNIQUE_OBJECT(type), tls, tls_reg, GET_ALIAS(__VA_ARGS__,), GET_ALIAS2(__VA_ARGS__,))

    #define DECLARE_DEFAULT(type, per_thr) extern per_thr capability PLT_UNIQUE_OBJECT(type);
    #define ALLOCATE_DEFAULT(type, per_thr) per_thr capability PLT_UNIQUE_OBJECT(type);

    #define INIT_OBJ(name, ret, sig, ...)             \
        __asm__ ("cscbi %[d], %%capcall20(" #name "_dummy)($c25)\n"::[d]"C"(plt_if -> name):);

    #define DECLARE_PLT_INIT(type, LIST, tls_reg, tls)                                 \
    void init_ ## type (type* plt_if, capability data, capability trust_mode);      \
    void init_ ## type ##_change_mode(capability trust_mode);                       \
    capability init_ ## type ##_get_mode(void);                       \
    void init_ ## type ##_new_thread(capability data);

    #define PLT_INIT_MAIN_THREAD(type)  init_ ## type
    #define PLT_INIT_NEW_THREAD(type) init_ ## type ##_new_thread

    #define DECLARE_WEAK_OBJ_DUMMY(type) ".weak " #type "_data_obj_dummy; .hidden " #type "_data_obj_dummy"

    #define DEFINE_PLT_INIT(type, LIST, tls_reg, tls)                                 \
    void PLT_INIT_MAIN_THREAD(type)  (type* plt_if, capability data, capability trust_mode) {      \
        __asm__ ("cscbi %[d], %%captab" tls "20(" #type "_data_obj)(" tls_reg ")\n"::[d]"C"(data):); \
        __asm__ (DECLARE_WEAK_OBJ_DUMMY(type) "; cscbi %[d], %%capcall20(" #type "_data_obj_dummy)($c25)\n"::[d]"C"(trust_mode):); \
        LIST(INIT_OBJ)                                                                \
    }\
    void PLT_INIT_NEW_THREAD(type) (capability data) {      \
            __asm__ ("cscbi %[d], %%captab" tls "20(" #type "_data_obj)(" tls_reg ")\n"::[d]"C"(data):); \
    }\
    void init_ ## type ##_change_mode(capability trust_mode) {\
        __asm__ (DECLARE_WEAK_OBJ_DUMMY(type) "; cscbi %[d], %%capcall20(" #type "_data_obj_dummy)($c25)\n"::[d]"C"(trust_mode):); \
    }\
    capability init_ ## type ##_get_mode(void) {\
        capability trust_mode;\
        __asm__ (DECLARE_WEAK_OBJ_DUMMY(type) "; clcbi %[d], %%capcall20(" #type "_data_obj_dummy)($c25)\n":[d]"=C"(trust_mode)::); \
        return trust_mode;  \
    }

    #define PLT_ty(type, LIST) \
    typedef struct                          \
    {                                       \
        LIST(PLT_GOT_ENTRY,)                \
    } type;


    #define DEFINE_F(name, ret, ty, ...) ret name ty;
    #define PLT_define(LIST) LIST(DEFINE_F)

    #define PLT_common(type, LIST, per_thr, tls_reg, tls)  \
    PLT_ty(type, LIST)                      \
    DECLARE_DEFAULT(type, per_thr)          \
    LIST(DECLARE_STUB,)                     \
    DECLARE_PLT_INIT(type, LIST, tls_reg, tls) \
    DEFINE_DUMMYS(LIST) \
    __attribute__((weak)) VIS_HIDDEN extern void type ## _data_obj_dummy(void);

    #define DUMMY_HELP(name, ret, ty, ...) __attribute__((weak)) extern ret name ## _dummy ty;

    // TODO could achieve lazy link by putting a suitable stub here. Otherwise these must be replaced before use
    #define DEFINE_DUMMYS(LIST) LIST(DUMMY_HELP)
    #define MAKE_DUMMYS(LIST)


    #define PLT(type, LIST) PLT_common(type, LIST,, "$c25",)
    #define PLT_thr(type, LIST) PLT_common(type, LIST,__thread,"$c26", "_tls")

    #define PLT_ALLOCATE_common(type, LIST, thread_loc, tls, tls_reg, ST) \
        ALLOCATE_DEFAULT(type, thread_loc)      \
        DEFINE_PLT_INIT(type, LIST, tls_reg, tls)   \
        MAKE_DUMMYS(LIST)                       \
        LIST(DEFINE_STUB, type, ST, tls, tls_reg)


    #define PLT_ALLOCATE_csd(type, LIST)  PLT_ALLOCATE_common(type, LIST,,,"$c25",PLT_STUB_CGP_ONLY_CSD)
    #define PLT_ALLOCATE(type, LIST) PLT_ALLOCATE_common(type, LIST,,,"$c25",PLT_STUB_CGP_ONLY_MODE_SEL)
    #define PLT_ALLOCATE_tls(type, LIST) PLT_ALLOCATE_common(type, LIST,__thread,"_tls","$c26",PLT_STUB_CGP_ONLY_MODE_SEL)

    // These are the mode stubs
    extern void plt_common_single_domain(void);
    extern void plt_common_complete_trusting(void);
    extern void plt_common_trusting(void);
    extern void plt_common_untrusting(void);

    // This is the fully untrusting entry stub
    extern void entry_stub(void);

    #define OTHER_DOMAIN_FP(X) (&(X ## _dummy))
    #define OTHER_DOMAIN_DATA(X) (__typeof(PLT_UNIQUE_OBJECT(X)))(&(PLT_UNIQUE_OBJECT(X))) // The data is inlined into the table


    typedef void init_if_func_t(capability plt_if, capability data, capability trust_mode);
    typedef void init_if_new_thread_func_t(capability data);

    #define INIT_OTHER_OBJECT(t) __CONCAT(init_other_object_ , t)
    #define INIT_OTHER_OBJECT_IF_ITEM(ITEM, t, ...) ITEM(init_other_object_ ## t, int, (act_control_kt self_ctrl, mop_t mop, queue_t* queue, startup_flags_e start_flags), __VA_ARGS__)

#else // __ASSEMBLY__

    // If this is included from assembly, we should instead create a set of .set directives that define the offsets
    // In the .plt.got
    .set enum_ctr, 0
    #define SET_NAME(NAME, ...) .set NAME ## _offset, (enum_ctr * CAP_SIZE); .set enum_ctr, enum_ctr + 1;
    #define PLT(type, LIST) LIST(SET_NAME,)
    #define PLT_thr(type, LIST, ...) PLT(type, LIST)

#endif // __ASSEMBLY__

#endif //CHERIOS_CHERIPLT_H
