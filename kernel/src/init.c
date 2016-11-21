/*-
 * Copyright (c) 2016 Hadrien Barral
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

#include "klib.h"
#include "boot_info.h"

#define KERN_PRINT_PTR(ptr)						\
	kernel_printf("%s: " #ptr " b:%016jx l:%016zx o:%jx\n",		\
		      __func__,						\
		      cheri_getbase((const __capability void *)(ptr)),	\
		      cheri_getlen((const __capability void *)(ptr)),	\
		      cheri_getoffset((const __capability void *)(ptr)))

#define KERN_PRINT_CAP(cap)						\
	kernel_printf("%-20s: %-16s t:%lx s:%lx p:%08jx "		\
		      "b:%016jx l:%016zx o:%jx\n",			\
		      __func__,						\
		      #cap,						\
		      cheri_gettag(cap),				\
		      cheri_getsealed(cap),				\
		      cheri_getperm(cap),				\
		      cheri_getbase(cap),				\
		      cheri_getlen(cap),				\
		      cheri_getoffset(cap))

static void cache_inv_low(int op, size_t line) {
	__asm__ __volatile__ (
		"cache %[op], 0(%[line]) \n"
		:: [op]"i" (op), [line]"r" (line));
}

static boot_info_t *get_bootinfo() {
	boot_info_t *bi;
	__asm__ __volatile__ (
		"cmove	$c3, %[bi] \n"
		: [bi]"=C" (bi)
		);
	return bi;
}

static void install_exception_vectors(void) {
	/* Copy exception trampoline to exception vector */
	char * all_mem = cheri_getdefault() ;
	void *mips_bev0_exception_vector_ptr =
	                (void *)(all_mem + MIPS_BEV0_EXCEPTION_VECTOR);
	memcpy(mips_bev0_exception_vector_ptr, &kernel_exception_trampoline,
	    (char *)&kernel_exception_trampoline_end - (char *)&kernel_exception_trampoline);
	void *mips_bev0_ccall_vector_ptr =
	                (void *)(all_mem + MIPS_BEV0_CCALL_VECTOR);
	memcpy(mips_bev0_ccall_vector_ptr, &kernel_exception_trampoline,
	    (char *)&kernel_exception_trampoline_end - (char *)&kernel_exception_trampoline);

	/* Invalidate I-cache */
	__asm volatile("sync");
	cache_inv_low((0b100<<2)+0, MIPS_BEV0_EXCEPTION_VECTOR & 0xFFFF);
	/* does not work with kseg0 address, hence the `& 0xFFFF` */
	__asm volatile("sync");
}

int cherios_main(void) {
	boot_info_t *bi = get_bootinfo();
	kernel_puts("Kernel Hello world\n");
	KERN_PRINT_CAP(bi);

	install_exception_vectors();
	act_init();
	kernel_interrupts_init(1);
	KERNEL_TRACE("init", "init done");
	return 0;
}
