/**************************************************************************
*         S M A Technologie AG, 34266 Niestetal
***************************************************************************
*
*  Include SMADEF.H defines basic data types used by SMA software
*
***************************************************************************
*  PREFIX : -
***************************************************************************
*  STATUS : rfc
***************************************************************************
*  REMARKS: MUST be used for all new files
***************************************************************************
*
*  HISTORY:
*
*  XXX DD.MM.JJ DESCRIPTION
*  ------------------------------------------------------------------------
*  CA  09.04.98 Created
*  CA  27.04.98 change define from _SMADEFINES to _SMADEFINE_98 to seperate
*               from old smadefs.h
*
***************************************************************************
*
*  TODO: -
*
***************************************************************************/


#ifndef _SMADEF_H
#define	_SMADEF_H

#ifdef	__cplusplus
extern "C" {
#endif



/**************************************************************************
* C O N S T A N T S / D E F I N E S
***************************************************************************/

/* Globale Konstanten ****************************************************/

#ifndef TRUE
#define TRUE      (1 == 1)
#define FALSE     (1 == 0)
#endif

#ifndef false
#define false FALSE
#endif

#ifndef true
#define true TRUE
#endif

/* Typen *****************************************************************/

typedef unsigned int       BIT;        /* Prefix bit */
typedef char               CHAR;       /* Prefix c */
typedef unsigned char      BYTE;       /* Prefix b */
typedef short              SHORT;      /* Prefix s */
typedef unsigned short     WORD;       /* Prefix w */
typedef long               LONG;       /* Prefix l */
typedef unsigned long      DWORD;      /* Prefix d */
typedef float              FLOAT;      /* Prefix f */
typedef double             DOUBLE;     /* Prefix dbl */

/*typedef void*              XPOINT;*/

#ifndef NO_SMA_BOOL
#ifndef BOOL
typedef int BOOL;
#endif
#endif

/**************************************************************************
* M A C R O  D E F I N I T I O N S
***************************************************************************/

/**************************************************************************
* Mark not used variables -> eliminates compiler warning
***************************************************************************/
#define UNUSED_VAR(var)  (var) = (var)


#ifdef	__cplusplus
}
#endif

#endif	/* _SMADEF_H */

