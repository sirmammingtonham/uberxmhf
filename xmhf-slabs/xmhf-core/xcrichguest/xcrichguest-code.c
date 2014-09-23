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

// XMHF rich guest (xcrichguest)
// author: amit vasudevan (amitvasudevan@acm.org)

//---includes-------------------------------------------------------------------
#include <xmhf.h>
#include <xmhf-core.h>
#include <xmhf-debug.h>

#include <xcrichguest.h>

#include <xcapi.h>
#include <xhhyperdep.h>	//TODO: remove this hardcoded hypapp dependency

slab_retval_t xcrichguest_interface(u32 src_slabid, u32 dst_slabid, u32 fn_id, u32 fn_paramsize, ...){
	slab_retval_t srval;
	va_list args;

	_XDPRINTF_("%s: Got control: src_slabid=%u, dst_slabid=%u, fn_id=%u, fn_paramsize=%u\n", __FUNCTION__, src_slabid, dst_slabid, fn_id, fn_paramsize);

	switch(fn_id){
			case XMHF_SLAB_XCRICHGUEST_FNENTRY:{
				u32 cpuid;
				bool is_bsp;
				va_start(args, fn_paramsize);
				cpuid = va_arg(args, u32);
				is_bsp = va_arg(args, bool);
				srval.retval_bool = xcrichguest_entry(cpuid, is_bsp);
				va_end(args);
			}
			break;

			case XMHF_SLAB_XCRICHGUEST_FNGUESTMEMORYREPORTING:{
				context_desc_t context_desc;
				struct regs r;
				va_start(args, fn_paramsize);
				context_desc = va_arg(args, context_desc_t);
				r = va_arg(args, struct regs);
				srval.retval_regs = xcrichguest_arch_handle_guestmemoryreporting(context_desc, r);
				va_end(args);
			}
			break;

			default:
				_XDPRINTF_("%s: unhandled subinterface %u. Halting\n", __FUNCTION__, fn_id);
				HALT();
	}

	return srval;
}




//add given cpu to the rich guest partition
static context_desc_t _xcrichguest_setup(u32 partition_index, u32 cpuid, bool is_bsp){
	context_desc_t context_desc;
    slab_retval_t srval;

	//add cpu to the richguest partition
    srval = XMHF_SLAB_CALL_P2P(xcapi, XMHF_SLAB_XCRICHGUEST_INDEX, XMHF_SLAB_XCAPI_INDEX, XMHF_SLAB_XCAPI_FNXCAPIPARTITIONADDCPU, XMHF_SLAB_XCAPI_FNXCAPIPARTITIONADDCPU_SIZE, partition_index, cpuid, is_bsp);
    context_desc = srval.retval_context_desc;
	//bail out if we could not add cpu to the rich guest partition
	if(context_desc.cpu_desc.cpu_index == XC_PARTITION_INDEX_INVALID || context_desc.partition_desc.partition_index == XC_PARTITION_INDEX_INVALID){
		_XDPRINTF_("\n%s: could not add cpu to rich guest partition. Halting!", __FUNCTION__);
		return context_desc;
	}

	//setup guest OS state for partition
	xcrichguest_arch_setupguestOSstate(context_desc);

	return context_desc;
}


//we get control here in the context of *each* physical CPU core
bool xcrichguest_entry(u32 cpuid, bool is_bsp){
	static u32 _xc_startup_hypappmain_counter = 0;
	static u32 _xc_startup_hypappmain_counter_lock = 1;
	context_desc_t context_desc;
	static volatile bool bsp_done=false;
	static u32 xc_richguest_partition_index=XC_PARTITION_INDEX_INVALID;
    slab_retval_t srval;

	_XDPRINTF_("\n%s: cpu=%x, is_bsp=%u. Halting\n", __FUNCTION__, cpuid, is_bsp);

	//ensure BSP is the first to grab the lock below
	if(!is_bsp)
		while(!bsp_done);

	//serialize execution
    spin_lock(&_xc_startup_hypappmain_counter_lock);

	//[debug]
	_XDPRINTF_("\n%s: cpuid=%08x, is_bsp=%u...\n", __FUNCTION__, cpuid, is_bsp);

	//create rich guest partition if we are the BSP
	if(is_bsp){
		_XDPRINTF_("\n%s: proceeding to create rich guest partition (esp=%x)\n", __FUNCTION__, read_esp());
		srval = XMHF_SLAB_CALL_P2P(xcapi, XMHF_SLAB_XCRICHGUEST_INDEX, XMHF_SLAB_XCAPI_INDEX, XMHF_SLAB_XCAPI_FNXCAPIPARTITIONCREATE, XMHF_SLAB_XCAPI_FNXCAPIPARTITIONCREATE_SIZE, XC_PARTITION_PRIMARY);
        xc_richguest_partition_index = srval.retval_u32;
		if(xc_richguest_partition_index == XC_PARTITION_INDEX_INVALID){
			_XDPRINTF_("\n%s: Fatal error, could not create rich guest partition!", __FUNCTION__);
			HALT();
		}
		_XDPRINTF_("\n%s: BSP: created rich guest partition %u", __FUNCTION__, xc_richguest_partition_index);
		xcrichguest_arch_initialize(xc_richguest_partition_index);
		_XDPRINTF_("\n%s: BSP: initialized rich guest partition %u", __FUNCTION__, xc_richguest_partition_index);
	}

	//add cpu to rich guest partition
	//TODO: check if this CPU is allocated to the "rich" guest. if so, pass it on to
	//the rich guest initialization procedure. if the CPU is not allocated to the
	//rich guest, enter it into a CPU pool for use by other partitions
	context_desc=_xcrichguest_setup(xc_richguest_partition_index, cpuid, is_bsp);

	if(context_desc.cpu_desc.cpu_index == XC_PARTITION_INDEX_INVALID || context_desc.partition_desc.partition_index == XC_PARTITION_INDEX_INVALID){
		_XDPRINTF_("\n%s: Fatal error, could not add cpu to rich guest. Halting!", __FUNCTION__);
		HALT();
	}

	//call hypapp main function
	{
		hypapp_env_block_t hypappenvb;

		hypappenvb.runtimephysmembase = (u32)xcbootinfo->physmem_base;
		hypappenvb.runtimesize = (u32)xcbootinfo->size;

		//call app main
		_XDPRINTF_("\n%s: proceeding to call xmhfhypapp_main on BSP", __FUNCTION__);
        XMHF_SLAB_CALL_P2P(xhhyperdep, XMHF_SLAB_XCRICHGUEST_INDEX, XMHF_SLAB_XHHYPERDEP_INDEX, XMHF_SLAB_HYPAPP_FNINITIALIZATION, XMHF_SLAB_HYPAPP_FNINITIALIZATION_SIZE, context_desc, hypappenvb );
		_XDPRINTF_("\n%s: came back into core", __FUNCTION__);
	}

    _xc_startup_hypappmain_counter++;

    //end serialized execution
    spin_unlock(&_xc_startup_hypappmain_counter_lock);

	//if we are the BSP, signal that APs can go ahead and do the above setup
	if(is_bsp)
		bsp_done=true;

	_XDPRINTF_("\n%s: cpu %x, isbsp=%u, Waiting for all cpus fo cycle through hypapp main", __FUNCTION__, cpuid, is_bsp);
	_XDPRINTF_("\n\n");

	//wait for hypapp main to execute on all the cpus
	while(_xc_startup_hypappmain_counter < xcbootinfo->cpuinfo_numentries);

	//start cpu in corresponding partition
	_XDPRINTF_("\n%s[%u]: starting in partition...\n", __FUNCTION__, context_desc.cpu_desc.cpu_index);

	//xmhf_partition_start(context_desc.cpu_desc.cpu_index);
	srval = XMHF_SLAB_CALL_P2P(xcapi, XMHF_SLAB_XCRICHGUEST_INDEX, XMHF_SLAB_XCAPI_INDEX, XMHF_SLAB_XCAPI_FNXCAPIPARTITIONSTARTCPU, XMHF_SLAB_XCAPI_FNXCAPIPARTITIONSTARTCPU_SIZE, context_desc);
    if(!srval.retval_bool){
		_XDPRINTF_("\n%s: should not be here. HALTING!", __FUNCTION__);
		HALT();
	}
}

///////
XMHF_SLAB_DEF(xcrichguest)