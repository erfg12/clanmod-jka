
/*****************************************************************************
 * name:		l_log.h
 *
 * desc:		log file
 *
 * $Archive: /source/code/botlib/l_log.h $
 * $Author: erfg1 $ 
 * $Revision: 1.1 $
 * $Modtime: 10/05/99 3:32p $
 * $Date: 2006/11/29 17:02:44 $
 *
 *****************************************************************************/

//open a log file
void Log_Open(char *filename);
//close the current log file
void Log_Close(void);
//close log file if present
void Log_Shutdown(void);
//write to the current opened log file
void QDECL Log_Write(char *fmt, ...);
//write to the current opened log file with a time stamp
void QDECL Log_WriteTimeStamped(char *fmt, ...);
//returns a pointer to the log file
FILE *Log_FilePointer(void);
//flush log file
void Log_Flush(void);

