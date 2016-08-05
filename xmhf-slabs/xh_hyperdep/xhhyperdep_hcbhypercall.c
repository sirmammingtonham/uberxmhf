/*
 * @XMHF_LICENSE_HEADER_START@
 *
 * eXtensible, Modular Hypervisor Framework (XMHF)
 * Copyright (c) 2009-2012 Carnegie Mellon University
 * Copyright (c) 2010-2012 VDG Inc.
 * All Rights Reserved.
 *
 * Developed by: XMHF Team
 *               Carnegie Mellon University / CyLab
 *               VDG Inc.
 *               http://xmhf.org
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the names of Carnegie Mellon or VDG Inc, nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @XMHF_LICENSE_HEADER_END@
 */

// hyperdep hypapp hypercall handler
// author: amit vasudevan (amitvasudevan@acm.org)

#include <xmhf.h>
#include <xmhfgeec.h>
#include <xmhf-debug.h>

#include <xc.h>
#include <uapi_gcpustate.h>
#include <xh_hyperdep.h>

//@ ghost bool hyperdep_methodcall_hcbhypercall_activatedep = false;
//@ ghost bool hyperdep_methodcall_hcbhypercall_deactivatedep = false;
//@ ghost bool hyperdep_methodcall_hcbhypercall_invalid = false;
/*@
	ensures (call_id == HYPERDEP_ACTIVATEDEP) ==>
		(hyperdep_methodcall_hcbhypercall_activatedep == true);
	ensures (call_id == HYPERDEP_DEACTIVATEDEP) ==>
		(hyperdep_methodcall_hcbhypercall_deactivatedep == true);
	ensures !(
			(call_id == HYPERDEP_ACTIVATEDEP) ||
			(call_id == HYPERDEP_DEACTIVATEDEP)
		) ==> 	(hyperdep_methodcall_hcbhypercall_invalid == true);
@*/

static inline void hyperdep_hcbhypercall_helper(u32 cpuindex, u32 call_id, u32 guest_slab_index, u64 gpa){
	_XDPRINTF_("%s[%u]: call_id=%x, gpa=%016llx\n", __func__, (u16)cpuindex, call_id, gpa);

	if(call_id == HYPERDEP_ACTIVATEDEP){
		hyperdep_activatedep(cpuindex, guest_slab_index, gpa);
		//@ghost hyperdep_methodcall_hcbhypercall_activatedep = true;

	}else if (call_id == HYPERDEP_DEACTIVATEDEP){
		hyperdep_deactivatedep(cpuindex, guest_slab_index, gpa);
		//@ghost hyperdep_methodcall_hcbhypercall_deactivatedep = true;

	}else{
		//_XDPRINTF_("%s[%u]: unsupported hypercall %x. Ignoring\n",__func__, (u16)cpuindex, call_id);
		//@ghost hyperdep_methodcall_hcbhypercall_invalid = true;

	}
}

//@ghost bool hyperdep_hcbhypercall_invokehelper=false;
/*@
	ensures hyperdep_hcbhypercall_invokehelper == true;
@*/
void hyperdep_hcbhypercall(u32 cpuindex, u32 guest_slab_index){
	slab_params_t spl;
	xmhf_uapi_gcpustate_gprs_params_t *gcpustate_gprs =
		(xmhf_uapi_gcpustate_gprs_params_t *)spl.in_out_params;
	x86regs_t *gprs = (x86regs_t *)&gcpustate_gprs->gprs;
	u32 call_id;
	u64 gpa;

	spl.src_slabid = XMHFGEEC_SLAB_XH_HYPERDEP;
	spl.dst_slabid = XMHFGEEC_SLAB_UAPI_GCPUSTATE;
	spl.cpuid = cpuindex;
	spl.dst_uapifn = XMHF_HIC_UAPI_CPUSTATE_GUESTGPRSREAD;
	XMHF_SLAB_CALLNEW(&spl);

	call_id = gprs->eax;
	gpa = ((u64)gprs->ebx << 32) | gprs->edx;

	hyperdep_hcbhypercall_helper(cpuindex, call_id, guest_slab_index, gpa);
	//@ghost hyperdep_hcbhypercall_invokehelper = true;
}
