// Copyright (C) 2003 - 2006 - Michael J. Nohai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of agreement written in the JAE Mod Source.doc.
// See JKA Game Source License.htm for legal information with Raven Software.
// Use this code at your own risk.

#ifndef __G_ADMINSHARED_H__
#define __G_ADMINSHARED_H__

void G_IgnoreClientChat ( int ignorer, int ignoree, qboolean ignore );

qboolean G_IsClientChatIgnored( int ignorer, int ingnoree );
void G_RemoveFromAllIgnoreLists( int ignorer );

#endif //__G_ADMINSHARED_H__
