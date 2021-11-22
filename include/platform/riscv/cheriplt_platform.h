/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Lawrence Esswood
 *
 * This work was supported by Innovate UK project 105694, "Digital Security
 * by Design (DSbD) Technology Platform Prototype".
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

#ifndef CHERIOS_CHERIPLT_PLATFORM_H
#define CHERIOS_CHERIPLT_PLATFORM_H

// TODO RISCV

// FIXME: These all assume that the offsets for captab symbols are small enough for captab_*_lo to be enough
// FIXME: If this turns out not to be true, also use the captab_*_hi relocations
#define PLT_STUB_CGP_ONLY_CSD(name, obj, tls, tls_reg, alias, alias2)               \
__asm__ (                                                                           \
    ".text\n"                                                                       \
    ".global " #name "\n"                                                           \
    X_STRINGIFY(ASM_VISIBILITY) " " #name "\n"                                      \
    ".type " #name ", \"function\"\n"                                               \
    "" #name ":\n"                                                                  \
    alias                                                                           \
    WEAK_DUMMY(name)                                                                \
    "clc " X_STRINGIFY(PLT_REG_TARGET) ", %captab_call_lo(" #name "_dummy)(" X_STRINGIFY(PLT_REG_GLOB) ")\n" \
    "clc ct0, %captab" tls "_lo(" EVAL1(STRINGIFY(obj)) ")(" tls_reg ")\n"          \
    "cinvoke " X_STRINGIFY(PLT_REG_TARGET) ", ct0\n"                                \
    alias2                                                                          \
    ".size " #name ", 12\n"                                                          \
);

#define PLT_STUB_CGP_ONLY_MODE_SEL(name, obj, tls, tls_reg, alias, alias2)          \
__asm__ (                                                                           \
    ".text\n"                                                                       \
    ".global " #name "\n"                                                           \
    X_STRINGIFY(ASM_VISIBILITY) " " #name "\n"                                      \
    ".type " #name ", \"function\"\n"                                               \
    "" #name ":\n"                                                                  \
    alias                                                                           \
    WEAK_DUMMY(name)                                                                \
    WEAK_DUMMY(obj)                                                                 \
    "clc ct1, %captab_call_lo(" EVAL1(STRINGIFY(obj)) "_dummy)(" X_STRINGIFY(PLT_REG_GLOB) ")\n"   \
    "clc " X_STRINGIFY(PLT_REG_TARGET) ", %captab_call_lo(" #name "_dummy)(" X_STRINGIFY(PLT_REG_GLOB) ")\n" \
    "clc " X_STRINGIFY(PLT_REG_TARGET_DATA) ", %captab" tls "_lo(" EVAL1(STRINGIFY(obj)) ")(" tls_reg ")\n" \
    "cjr ct1\n"                                                                     \
    alias2                                                                          \
    ".size " #name ", 16\n"                                                          \
);

#define INIT_OBJ(name, ret, sig, ...)             \
        __asm__ ("csc %[d], %%captab_call_lo(" #name "_dummy)("X_STRINGIFY(PLT_REG_GLOB)")\n"::[d]"C"(plt_if -> name):);

#define PLT_STORE_CAPTAB(tls, type, reg, data) \
        __asm__ ("csc %[d], %%captab" tls "_lo(" #type "_data_obj)(" reg ")\n"::[d]"C"(data):)

#define PLT_STORE_CAPTAB_CALL(type, trust_mode) \
        __asm__ (DECLARE_WEAK_OBJ_DUMMY(type) "; csc %[d], %%captab_call_lo(" #type "_data_obj_dummy)("X_STRINGIFY(PLT_REG_GLOB)")\n"::[d]"C"(trust_mode):)

#define PLT_LOAD_CAPTAB_CALL(type, trust_mode) \
        __asm__ (DECLARE_WEAK_OBJ_DUMMY(type) "; clc %[d], %%captab_call_lo(" #type "_data_obj_dummy)("X_STRINGIFY(PLT_REG_GLOB)")\n":[d]"=C"(trust_mode)::)

#endif //CHERIOS_CHERIPLT_PLATFORM_H