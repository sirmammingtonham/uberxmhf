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
 * This file is part of the EMHF historical reference
 * codebase, and is released under the terms of the
 * GNU General Public License (GPL) version 2.
 * Please see the LICENSE file for details.
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

// EMHF memory protection component 
// generic x86 arch. backend implementation
// author: amit vasudevan (amitvasudevan@acm.org)

#include <emhf.h> 

//flush hardware page table mappings (TLB) 
void emhf_memprot_arch_flushmappings(VCPU *vcpu){
	ASSERT(vcpu->cpu_vendor == CPU_VENDOR_AMD || vcpu->cpu_vendor == CPU_VENDOR_INTEL);

	if(vcpu->cpu_vendor == CPU_VENDOR_AMD)
		emhf_memprot_arch_x86svm_flushmappings(vcpu);
	else //CPU_VENDOR_INTEL
		emhf_memprot_arch_x86vmx_flushmappings(vcpu);

}

//set protection for a given physical memory address
void emhf_memprot_arch_setprot(VCPU *vcpu, u64 gpa, u32 prottype){
	//sanity check on protection types
		//if the page protection is set to not-present, we wipe out
		//anything other protections if present
		if(prottype & MEMP_PROT_NOTPRESENT)
			prottype = MEMP_PROT_NOTPRESENT;
	
		//else, force PROT_PRESENT 
		prottype |= MEMP_PROT_NOTPRESENT;
		
		//if both read-only and read-write are specified, fall back
		//to read-only
		if( (prottype & MEMP_PROT_READONLY) && (prottype & MEMP_PROT_READWRITE) )
			prottype &= ~(MEMP_PROT_READWRITE);
		
		//if both no-execute and execute are specified, fall back to
		//no-execute
		if( (prottype & MEMP_PROT_NOEXECUTE) && (prottype & MEMP_PROT_EXECUTE) )
			prottype &= ~(MEMP_PROT_NOEXECUTE);


	//invoke appropriate sub arch. backend
	if(vcpu->cpu_vendor == CPU_VENDOR_AMD)
		emhf_memprot_arch_x86svm_setprot(vcpu, gpa, prottype);
	else //CPU_VENDOR_INTEL
		emhf_memprot_arch_x86vmx_setprot(vcpu, gpa, prottype);
}

//get protection for a given physical memory address
u32 emhf_memprot_arch_getprot(VCPU *vcpu, u64 gpa){
	//invoke appropriate sub arch. backend
	if(vcpu->cpu_vendor == CPU_VENDOR_AMD)
		return emhf_memprot_arch_x86svm_getprot(vcpu, gpa);
	else //CPU_VENDOR_INTEL
		return emhf_memprot_arch_x86vmx_getprot(vcpu, gpa);
}
