
/*****************************************************************************
 * name:		be_aas_file.h
 *
 * desc:		AAS
 *
 * $Archive: /source/code/botlib/be_aas_file.h $
 * $Author: erfg1 $ 
 * $Revision: 1.1 $
 * $Modtime: 10/05/99 3:32p $
 * $Date: 2006/11/29 17:02:40 $
 *
 *****************************************************************************/

#ifdef AASINTERN
//loads the AAS file with the given name
int AAS_LoadAASFile(char *filename);
//writes an AAS file with the given name
qboolean AAS_WriteAASFile(char *filename);
//dumps the loaded AAS data
void AAS_DumpAASData(void);
//print AAS file information
void AAS_FileInfo(void);
#endif //AASINTERN

