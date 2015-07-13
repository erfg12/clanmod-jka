#include "disasm.h"

// Definitions for g_engine.c6

#include "q_shared.h"
#include "g_local.h"


#define MAX_DOWNLOAD_WINDOW			8		// max of eight download frames
#define MAX_DOWNLOAD_BLKSIZE		2048	// 2048 byte block chunks

#define	PACKET_BACKUP	32	// number of old messages that must be kept on client and
							// server for delta comrpession and ping estimation
#define	PACKET_MASK		(PACKET_BACKUP-1)

#define	MAX_PACKET_USERCMDS		32		// max number of usercmd_t in a packet

#define	PORT_ANY			-1

#define	MAX_RELIABLE_COMMANDS	128			// max string commands buffered for restransmit

#define	MAX_MSGLEN				49152		// max length of a message, which may
											// be fragmented into multiple packets
typedef struct {
	int	type;

	byte	ip[4];
	byte	ipx[10];

	unsigned short	port;
} netadr_t;

typedef enum {
	NS_CLIENT,
	NS_SERVER
} netsrc_t;

typedef struct {
	netsrc_t	sock;

	int			dropped;			// between last packet and previous

	netadr_t	remoteAddress;
	int			qport;				// qport value to write when transmitting

	// sequencing variables
	int			incomingSequence;
	int			outgoingSequence;

	// incoming fragment assembly buffer
	int			fragmentSequence;
	int			fragmentLength;	
	byte		fragmentBuffer[MAX_MSGLEN];

	// outgoing fragment buffer
	// we need to space out the sending of large fragmented messages
	qboolean	unsentFragments;
	int			unsentFragmentStart;
	int			unsentLength;
	byte		unsentBuffer[MAX_MSGLEN];
} netchan_t;


typedef struct {
	int				areabytes;
	byte			areabits[MAX_MAP_AREA_BYTES];		// portalarea visibility bits
	playerState_t	ps;
	playerState_t	vps; //vehicle I'm riding's playerstate (if applicable) -rww

	int				num_entities;
	int				first_entity;		// into the circular sv_packet_entities[]
										// the entities MUST be in increasing state number
										// order, otherwise the delta compression will fail
	int				messageSent;		// time the message was transmitted
	int				messageAcked;		// time the message was acked
	int				messageSize;		// used to rate drop packets
} clientSnapshot_t;

typedef enum {
	CS_FREE,		// can be reused for a new connection
	CS_ZOMBIE,		// client has been disconnected, but don't reuse
					// connection for a couple seconds
	CS_CONNECTED,	// has been assigned to a client_t, but no gamestate yet
	CS_PRIMED,		// gamestate has been sent, but client hasn't sent a usercmd
	CS_ACTIVE		// client is fully in game
} clientState_t;


typedef struct client_s {
	clientState_t	state;
	char			userinfo[MAX_INFO_STRING];		// name, etc

	qboolean		sentGamedir; //see if he has been sent an svc_setgame

	char			reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];
	int				reliableSequence;		// last added reliable message, not necesarily sent or acknowledged yet
	int				reliableAcknowledge;	// last acknowledged reliable message
	int				reliableSent;			// last sent reliable message, not necesarily acknowledged yet
	int				messageAcknowledge;

	int				gamestateMessageNum;	// netchan->outgoingSequence of gamestate
	int				challenge;

	usercmd_t		lastUsercmd;
	int				lastMessageNum;		// for delta compression
	int				lastClientCommand;	// reliable client message sequence
	char			lastClientCommandString[MAX_STRING_CHARS];
	sharedEntity_t	*gentity;			// SV_GentityNum(clientnum)
	char			name[MAX_NAME_LENGTH];			// extracted from userinfo, high bits masked

	// downloading
	char			downloadName[MAX_QPATH]; // if not empty string, we are downloading
	fileHandle_t	download;			// file being downloaded
 	int				downloadSize;		// total bytes (can't use EOF because of paks)
 	int				downloadCount;		// bytes sent
	int				downloadClientBlock;	// last block we sent to the client, awaiting ack
	int				downloadCurrentBlock;	// current block number
	int				downloadXmitBlock;	// last block we xmited
	unsigned char	*downloadBlocks[MAX_DOWNLOAD_WINDOW];	// the buffers for the download blocks
	int				downloadBlockSize[MAX_DOWNLOAD_WINDOW];
	qboolean		downloadEOF;		// We have sent the EOF block
	int				downloadSendTime;	// time we last got an ack from the client

	int				deltaMessage;		// frame last client usercmd message
	int				nextReliableTime;	// svs.time when another reliable command will be allowed
	int				lastPacketTime;		// svs.time when packet was last received
	int				lastConnectTime;	// svs.time when connection started
	int				nextSnapshotTime;	// send another snapshot when svs.time >= nextSnapshotTime
	qboolean		rateDelayed;		// true if nextSnapshotTime was set based on rate instead of snapshotMsec
	int				timeoutCount;		// must timeout a few frames in a row so debugging doesn't break
	clientSnapshot_t	frames[PACKET_BACKUP];	// updates can be delta'd from here
	int				ping;
	int				rate;				// bytes / second
	int				snapshotMsec;		// requests a snapshot every snapshotMsec unless rate choked
	int				pureAuthentic;
	netchan_t		netchan;

	int				lastUserInfoChange; //if > svs.time && count > x, deny change -rww
	int				lastUserInfoCount; //allow a certain number of changes within a certain time period -rww

} client_t;

#undef INFINITE

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/mman.h>
	#include <unistd.h>
	#include <string.h>
    #include <stdlib.h>
    typedef unsigned char byte;
	#define _stricmp strcasecmp
#endif

#ifdef _WIN32
// Windows defs here
static void (*NET_OutOfBandPrint)( int sock, netadr_t adr, const char *format, ... ) = (void (*)(int, netadr_t, const char*,...))0x41A230;
static void (*Com_Printf2)( const char *fmt, ... ) = (void (*)(const char *, ...))0x40FBE0;
static void (*Com_DPrintf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x40FDB0;
static const char *(*Cmd_Argv)( int arg ) = (const char *(*)(int))0x40F490;
static void (*SV_SendServerCommand)(client_t *cl, const char *fmt, ...) = (void(*)(client_t *cl, const char *fmt, ...))0x4435D0;
#else
// Linux defs here
static void (*NET_OutOfBandPrint)( int sock, netadr_t adr, const char *format, ... ) = (void (*)(int, netadr_t, const char*,...))0x807B744;
static void (*Com_Printf2)( const char *fmt, ... ) = (void (*)(const char *, ...))0x8072CA4;
static void (*Com_DPrintf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x8072ED4;
static const char *(*Cmd_Argv)( int arg ) = (const char *(*)(int))0x812C264;
static void (*SV_SendServerCommand)(client_t *cl, const char *fmt, ...) = (void(*)(client_t *cl, const char *fmt, ...))0x8056214;

#endif

typedef struct {
	int addr;
	int size;
	char origbytes[24];
} PatchData_t;

typedef enum {
	PATCH_JUMP,
	PATCH_CALL,
} PatchType_e;

// ==================================================
// UnlockMemory (WIN32 & Linux compatible)
// --------------------------------------------------
// Makes the memory at address writable for at least
// size bytes.
// Returns 1 if successful, returns 0 on failure.
// ==================================================
static int UnlockMemory(int address, int size) {
	int ret;
	int dummy;
#ifdef _WIN32
	ret = VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &dummy);
	return (ret != 0);
#else
	// Linux is a bit more tricky
	int page1, page2;
	page1 = address & ~( getpagesize() - 1);
	page2 = (address+size) & ~( getpagesize() - 1);
	if( page1 == page2 ) {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
	} else {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
		if (ret) return 0;
		ret = mprotect((char *)page2, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
		return (ret == 0);
	}
#endif
}

// ==================================================
// LockMemory (WIN32 & Linux compatible)
// --------------------------------------------------
// Makes the memory at address read-only for at least
// size bytes.
// Returns 1 if successful, returns 0 on failure.
// ==================================================
static int LockMemory(int address, int size) {
	int ret;
#ifdef _WIN32
	ret = VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READ, NULL);
	return (ret != 0);
#else
	// Linux is a bit more tricky
	int page1, page2;
	page1 = address & ~( getpagesize() - 1);
	page2 = (address+size) & ~( getpagesize() - 1);
	if( page1 == page2 ) {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_EXEC);
	} else {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_EXEC);
		if (ret) return 0;
		ret = mprotect((char *)page2, getpagesize(), PROT_READ | PROT_EXEC);
		return (ret == 0);
	}
#endif
}

// ==================================================
// JKG_PlacePatch (WIN32 & Linux compatible)
// --------------------------------------------------
// Patches the code at address to make it go towards
// destination.
// The instruction used is either JMP or CALL, 
// depending on the type specified.
//
// Before the code is modified, the code page is
// unlocked. If you wish to stop it from being locked
// again, specify 1 for nolock
//
// This function returns a malloced PatchData_t.
// To remove the patch, call JKG_RemovePatch. This
// will also free the PatchData_t.
// ==================================================

static PatchData_t *JKG_PlacePatch( int type, unsigned int address, unsigned int destination ) {
	PatchData_t *patch = malloc(sizeof(PatchData_t));
	t_disasm dasm;
	int addr = address;
	int sz = 0;
	int opsz;
	// Disassemble the code and determine the size of the code we have to replace
	while (sz < 5) {
		opsz = Disasm((char*)addr, 16, addr, &dasm, DISASM_CODE);
		if (opsz == 0) {
			return NULL;	// Should never happen
		}
		sz += opsz;
		addr += opsz;
	}
	if (sz == 0 || sz > 24) {
		// This really shouldnt ever happen, in the worst case scenario,
		// the block is 20 bytes (4 + 16), so if we hit 24, something went wrong
		return NULL;
	}
	patch->addr = address;
	patch->size = sz;
	memcpy(patch->origbytes, (const void *)address, sz);
	UnlockMemory(address, sz); // Make the memory writable
	*(unsigned char *)address = type == PATCH_JUMP ? 0xE9 : 0xE8;
	*(unsigned int *)(address+1) = destination - (address + 5);
	memset((void *)(address+5),0x90,sz-5);	// Nop the rest
	LockMemory(address, sz);
	return patch;
}

static void JKG_RemovePatch(PatchData_t **patch) {
	if (!*patch)
		return;
	UnlockMemory((*patch)->addr, (*patch)->size);
	memcpy((void *)(*patch)->addr, (*patch)->origbytes, (*patch)->size);
	LockMemory((*patch)->addr, (*patch)->size);
	*patch = 0;
}



// ==================================================
// Shell System for Hooks (By BobaFett & Deathspike)
// --------------------------------------------------
// This system, that I (BobaFett) call:
// The Shell System, allows for hooks to be created
// on both windows and linux with exactly the same code
// (OS specific asm changes not included)
//
// The system works as follows:
// Since compilers have the tendancy to add prologue
// and epilogue code to functions, we put our asm
// inside a safe 'shell'
// The shell is defined by:
//
// void *MyHook()
// {
//		__JKG_StartHook;			<-- Shell
//		{
//			// Hook code here		<-- Contents of shell
//		}
//		__JKG_EndHook;				<-- Shell
// }
//
// This code should be placed in a function returning
// a void *, as shown above.
// When called, it will return the pointer to the
// shell's contents, which can then be used to place
// hooks (ie. jumps).
//
// Note that the shell's contents (the hook in question)
// are not executed!
//
// For the actual asm, 3 defines are available:
// __asm1__ for zero/single operand opcodes	(push 10	)
// __asm2__ for dual operand opcodes		(mov eax, 1	)
// __asmL__ for labels						(mylabel:	)
//
// To compile this code on linux, you require the
// following in the gcc command line:
//  -masm=intel
// 
// NOTE: The hook's execution flow must NEVER get to
//       the shell layer! Always ensure the code is
//       ended with a jump or a return!
//
// ==================================================

#ifdef _WIN32
	#define __asm1__( a )		__asm a
	#define __asm2__( a, b )	__asm a, b
	#define __asmL__( a )		__asm a
	#define __JKG_StartHook		__asm lea eax, [__hookstart] \
								__asm jmp __hookend \
								__asm __hookstart:
	#define __JKG_EndHook		__asm __hookend:
#else 
	#define __asm1__( a )		__asm__( #a "\n" );
	#define __asm2__( a, b )	__asm__( #a ", " #b "\n" );
	#define __asmL__( a )       __asm__( ".att_syntax\n" ); \
								__asm__( #a ":\n" ); \
								__asm__( ".intel_syntax noprefix\n" );
	#define __JKG_StartHook		__asm__( ".intel_syntax noprefix\n" ); \
								__asm__("lea eax, [__hookstart]\n"); \
								__asm__("jmp __hookend\n"); \
								__asm__(".att_syntax\n"); \
								__asm__("__hookstart:\n"); \
								__asm__(".intel_syntax noprefix\n");
	
	#define __JKG_EndHook		__asm__(".att_syntax\n"); \
								__asm__("__hookend:\n"); \
								__asm__(".intel_syntax noprefix\n");
#endif


// =================================================
// Hook 7:
// Download exploit fix
// -------------------------------------------------
// This hook here fixes the download exploit
// If a download is requested while ingame, it
// gets denied using a nice lil print, otherwise
// the download is redirected to a placeholder file
// that'll be sent instead.
//
// Hook located in SV_BeginDownload_f
// =================================================
//

#ifdef __linux__
// Define linux symbols
#define _DHFIX_PATCHPOS			0x804D723
#define _DHFIX_RETPOS1			0x804D544
#define _DHFIX_RETPOS2			0x804D751
#define _DHFIX_CLREG			ebx
#define _DHFIX_SVSCLIENTSPOS	0x83121EC

#else
// Define windows symbols
#define _DHFIX_PATCHPOS			0x43B3C7
#define _DHFIX_RETPOS1			0x43B2E0
#define _DHFIX_RETPOS2			0x43B3E8
#define _DHFIX_CLREG			esi
#define _DHFIX_SVSCLIENTSPOS	0x606224

#endif

static PatchData_t *pDHFIX;

int JKG_CheckDownloadRequest(int clientNum, client_t *cl, const char *filename);

static void *_Hook_DownloadHackFix()
{
	__JKG_StartHook;
	{
		__asm1__(	pushad									);	// Secure registers
		__asm2__(	mov		eax, _DHFIX_CLREG				);	// Get location of client_t
		__asm2__(	sub		eax, DS:[_DHFIX_SVSCLIENTSPOS]	);  // Work out the clientnum
		__asm2__(	xor		edx, edx						);  // by doing cl - svs.clients
		__asm2__(	mov		ecx, 0x51478					);  //
		__asm1__(	div		ecx								);  // Do division to get clientNum
		__asm1__(	push	0								);  // Push filename (placeholder)
		__asm1__(	push	_DHFIX_CLREG					);  // Push client_t *cl
		__asm1__(	push	eax								);  // Push clientNum
		__asm1__(	push	1								);  // Push 1 (for Cmd_Argv)
		__asm1__(	call	Cmd_Argv						);  // Call Cmd_Argv to get filename
		__asm2__(	add		esp, 4							);  // Clean up stack
		__asm2__(	mov		[esp+8], eax					);  // Replace the 0 we pushed
		__asm1__(	call	JKG_CheckDownloadRequest		);  // Call JKG_CheckDownloadRequest
		__asm2__(	add		esp, 0xC						);  // Clean up stack
		__asm2__(	test	eax, eax						);  // Check return value
		__asm1__(	popad									);  // Restore registers
		__asm1__(	je		bail							);  // If return value = 0, goto bail
		__asm1__(	push	_DHFIX_RETPOS1					);  // Push-ret call forward to
		__asm1__(	ret										);  // SV_CloseDownload
		__asmL__( bail:										);	// 
		__asm2__(	add		esp, 4							);  // Remove return address (we redirected a call)
		__asm1__(	push	_DHFIX_RETPOS2					);  // Push-ret jump to
		__asm1__(	ret										);  // the end of SV_BeginDownload_f
	}
	__JKG_EndHook;
}

static int JKG_CheckDownloadRequest(int clientNum, client_t *cl, const char *filename) {
	int illegal = 0;
	if (!filename) {
		illegal = 1;
	} else if (strlen(filename) < 4) {
		illegal = 1;
	} else if (Q_stricmpn(filename + strlen(filename) - 4, ".pk3", 4)) {
		illegal = 1;
	} else if (strstr(filename, "..")) {
		illegal = 1;
	}
	if (cl->state == CS_ACTIVE) {
		// These are 100% guaranteed to be fake
		if (illegal) {
			SV_SendServerCommand(cl, "print \"Download request for %s rejected: Illegal download request detected\n\"", filename);
			return 0;
		} else {
			SV_SendServerCommand(cl, "print \"Download request for %s rejected: Download requested while in-game\n\"", filename);
			return 0;
		}
	} else {
		if (illegal) {
			// Get a substitute file and send that instead
			// TODO: use a cvar for this
			Q_strncpyz(cl->downloadName, "baddownload.txt", 64);
			return 0;
		}
		// Legal download (or substituted one ;P)
		return 1;
	}
}

void JKG_PatchEngine() {
	//openJK crashes because of the printed messages
	//G_Printf(" ------- Installing Engine Patches -------- \n");

	///////////////////////////////
	// Hook 7: Download Hack Fix
	///////////////////////////////
	pDHFIX = JKG_PlacePatch(PATCH_CALL, _DHFIX_PATCHPOS, (unsigned int)_Hook_DownloadHackFix()); // We'll be overwriting a call here
	free(pDHFIX);

	//G_Printf("Finished\n");
}

void JKG_UnpatchEngine() {
	JKG_RemovePatch(&pDHFIX);
}
