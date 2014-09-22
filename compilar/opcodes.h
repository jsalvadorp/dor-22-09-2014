#ifndef OPCODES_H
#define OPCODES_H

enum opcodes { // just for the self increasing constants.
	OPC_nop 		= 0x00,
	OPC_break		,
	OPC_ldarg		,
	OPC_starg		,
	OPC_ldloc		,
	OPC_stloc		,
	OPC_ldcl		,
	OPC_stcl		,
	OPC_ldind		,
	OPC_stind		,
	OPC_ldg			,
	OPC_stg			,
	OPC_ldc			,
	OPC_ldi			,
	OPC_add			,
	OPC_sub			,
	OPC_mul			,
	OPC_div			,
	OPC_rem			,
	OPC_call		,
	OPC_ret			,
	OPC_br			,
	OPC_brz			,
	OPC_clos		,
	OPC_callcl		,
	OPC_dup			,
	OPC_pop			,
	OPC_tail_			// prefix
};

#define JUMP_TABLE \
	static void * jmp_tbl[] = { \
	&&case_nop 			, \
	&&case_break		, \
	&&case_ldarg		, \
	&&case_starg		, \
	&&case_ldloc		, \
	&&case_stloc		, \
	&&case_ldcl			, \
	&&case_stcl			, \
	&&case_ldind		, \
	&&case_stind		, \
	&&case_ldg			, \
	&&case_stg			, \
	&&case_ldc			, \
	&&case_ldi			, \
	&&case_add			, \
	&&case_sub			, \
	&&case_mul			, \
	&&case_div			, \
	&&case_rem			, \
	&&case_call			, \
	&&case_ret			, \
	&&case_br			, \
	&&case_brz			, \
	&&case_clos			, \
	&&case_callcl		, \
	&&case_dup			, \
	&&case_pop 			, \
	&&case_tail_		\
	};
	
#define DEFINE_ARGSIZES \
int argsize[] = { \
	0, /* nop 			*/ \
	0, /* break			*/ \
	2, /* ldarg			*/ \
	2, /* starg			*/ \
	2, /* ldloc			*/ \
	2, /* stloc			*/ \
	2, /* ldcl			*/ \
	2, /* stcl			*/ \
	0, /* ldind			*/ \
	0, /* stind			*/ \
	2, /* ldg			*/ \
	2, /* stg			*/ \
	2, /* ldc			*/ \
	4, /* ldi			*/ \
	0, /* add			*/ \
	0, /* sub			*/ \
	0, /* mul			*/ \
	0, /* div			*/ \
	0, /* rem			*/ \
	2, /* call			*/ \
	0, /* ret			*/ \
	2, /* br			*/ \
	2, /* brz			*/ \
	0, /* clos			*/ \
	0, /* callcl		*/ \
	0, /* dup			*/ \
	0, /* pop			*/ \
	0  /* tail_			*/ \
};

#endif
