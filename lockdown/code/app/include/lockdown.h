// lockdown specific declarations/definitions
// author: amit vasudevan (amitvasudevan@acm.org)

#ifndef __LOCKDOWN_H__
#define __LOCKDOWN_H__

#define LDN_ENV_TRUSTED_SIGNATURE  0x45555254   //"TRUE"
#define LDN_ENV_UNTRUSTED_SIGNATURE 0x45544E55  //"UNTE"

//specific defines for VT BOX
//#define LDN_ENV_TRUSTED_STARTSECTOR  (63)
//#define LDN_ENV_TRUSTED_ENDSECTOR  (12578894)
//#define LDN_NULL_SECTOR  (300000000)
//#define LDN_IDE_BUS   0xC000
//#define LDN_ALLOWED_SECTORS 0xFFFFFFFFUL
//#define LDN_MACHINE_NETWORKDEVICES {1,0,0},{2,0,0}
//#define LDN_OUTOFBOUNDS_CHECK	(((u64)lbaaddr >= (u64)LDN_ENV_TRUSTED_STARTSECTOR) && ((u64)lbaaddr <= (u64)LDN_ENV_TRUSTED_ENDSECTOR)) || ((u64)lbaaddr < 63ULL) 

//hyper-partitioning configuration (specific to the platform/installation)
//hp6555b
#define LDN_ENV_TRUSTED_STARTSECTOR  	(159396993)
#define LDN_ENV_TRUSTED_ENDSECTOR  		(222291404)
#define LDN_NULL_SECTOR  				(620000000)
#define LDN_IDE_BUS   					0x6010
#define LDN_ALLOWED_SECTORS 			159396930
#define LDN_OUTOFBOUNDS_CHECK			(((u64)lbaaddr >= (u64)LDN_ENV_TRUSTED_STARTSECTOR) && ((u64)lbaaddr <= (u64)LDN_ENV_TRUSTED_ENDSECTOR)) || ((u64)lbaaddr < 63ULL) || ((u64)lbaaddr >= 63ULL && (u64)lbaaddr <= 33554494ULL)



/*//hyper-partitioning configuration (specific to the platform/installation)
//hp8450p
#define LDN_ENV_TRUSTED_STARTSECTOR  	(159396993)
#define LDN_ENV_TRUSTED_ENDSECTOR  		(222291404)
#define LDN_NULL_SECTOR  				(620000000)
#define LDN_IDE_BUS   					0x1F0
#define LDN_ALLOWED_SECTORS 			33554495, 96470325, 159396991
#define LDN_OUTOFBOUNDS_CHECK			(((u64)lbaaddr >= (u64)LDN_ENV_TRUSTED_STARTSECTOR) && ((u64)lbaaddr <= (u64)LDN_ENV_TRUSTED_ENDSECTOR)) || ((u64)lbaaddr < 63ULL) || ((u64)lbaaddr >= 63ULL && (u64)lbaaddr <= 33554494ULL)
*/

#define LDN_MACHINE_NETWORKDEVICES {0x44,0,0},{0x46,0x06,0x00}

extern u32 LDN_ENV_PHYSICALMEMORYLIMIT;

#ifndef __ASSEMBLY__

typedef struct {
  u32 signature;  //trusted or untrusted env. being switched to

} __attribute__((packed)) LDNPB;


#endif //__ASSEMBLY__

#endif //__LOCKDOWN_H__
