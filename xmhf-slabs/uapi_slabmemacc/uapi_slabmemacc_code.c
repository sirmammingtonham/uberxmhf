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

/*
 * slab memory access uAPI
 *
 * author: amit vasudevan (amitvasudevan@acm.org)
 */

#include <xmhf.h>
#include <xmhf-debug.h>

#include <xmhfgeec.h>

#include <xc.h>
#include <uapi_slabmemacc.h>
#include <geec_sentinel.h>

/*
static bool _uapicheck_is_within_slab_memory_extents(u64 slab_id, u64 addr, u64 size){
    u64 i;
    bool status=false;

    for(i=0; i < HIC_SLAB_PHYSMEM_MAXEXTENTS; i++){
        if(_xmhfhic_common_slab_info_table[slab_id].slab_physmem_extents[i].addr_start == 0 &&
           _xmhfhic_common_slab_info_table[slab_id].slab_physmem_extents[i].addr_end == 0)
           continue;

        if(addr >= _xmhfhic_common_slab_info_table[slab_id].slab_physmem_extents[i].addr_start &&
           (addr+size) < _xmhfhic_common_slab_info_table[slab_id].slab_physmem_extents[i].addr_end)
            return true;

    }

    return status;
}*/

/////
void slab_main(slab_params_t *sp){

    //xmhf_uapi_params_hdr_t *uapiphdr = (xmhf_uapi_params_hdr_t *)sp->in_out_params;

    switch(sp->dst_uapifn){

        case XMHF_HIC_UAPI_PHYSMEM_PEEK:{
            xmhf_uapi_slabmemacc_params_t *smemaccp = (xmhf_uapi_slabmemacc_params_t *)sp->in_out_params;


            //check slab memory read capabilities
            if( !(_xmhfhic_common_slab_info_table[smemaccp->dst_slabid].slab_memgrantreadcaps & XMHFGEEC_SLAB_MEMGRANTREADCAP_MASK(sp->src_slabid)) ){
                _XDPRINTF_("UAPI_SLABMEMACC: Halt!. memgrantreadcap failed for src(%u)-->dst(%u), dst caps=0x%x\n",
                   sp->src_slabid, smemaccp->dst_slabid, _xmhfhic_common_slab_info_table[smemaccp->dst_slabid].slab_memgrantreadcaps);
                HALT();
            }


            memcpy(smemaccp->addr_to, smemaccp->addr_from, smemaccp->numbytes);
            //_XDPRINTF_("UAPI_SLABMEMACC[%u]: PEEK: addr_to=%x, addr_from=%x, \
            //           numbytes=%x\n", (u16)sp->cpuid, smemaccp->addr_to,
            //           smemaccp->addr_from, smemaccp->numbytes);
        }
        break;

        case XMHF_HIC_UAPI_PHYSMEM_POKE:{
            xmhf_uapi_slabmemacc_params_t *smemaccp = (xmhf_uapi_slabmemacc_params_t *)sp->in_out_params;

            //check slab memory write capabilities
            if( !(_xmhfhic_common_slab_info_table[smemaccp->dst_slabid].slab_memgrantwritecaps & XMHFGEEC_SLAB_MEMGRANTWRITECAP_MASK(sp->src_slabid)) ){
                _XDPRINTF_("UAPI_SLABMEMACC: Halt!. memgrantwritecap failed for src(%u)-->dst(%u), dst caps=0x%x\n",
                   sp->src_slabid, smemaccp->dst_slabid, _xmhfhic_common_slab_info_table[smemaccp->dst_slabid].slab_memgrantwritecaps);
                HALT();
            }


            memcpy(smemaccp->addr_to, smemaccp->addr_from, smemaccp->numbytes);
        }
        break;


        default:
            _XDPRINTF_("UAPI_SLABMEMACC[%u]: Unknown uAPI function %x. Halting!\n",
                    (u16)sp->cpuid, sp->dst_uapifn);
            HALT();
            return;
    }


}
