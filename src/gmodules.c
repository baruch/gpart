/*
 * gmodules.c -- gpart module functions
 *
 * gpart (c) 1999-2001 Michail Brzitwa <mb@ichabod.han.de>
 * Guess PC-type hard disk partitions.
 *
 * gpart is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * Created:   04.01.1999 <mb@ichabod.han.de>
 * Modified:  29.01.2001 <mb@ichabod.han.de>
 *            New modules: qnx & beos.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gpart.h"


static g_module		*g_head;
static int		g_count;



g_module *g_mod_head()
{
	return (g_head);
}



int g_mod_count()
{
	return (g_count);
}



void g_mod_list()
{
	g_module	*m;

	pr(MSG,"Module\tWeight\n");
	for (m = g_head; m; m = m->m_next)
		pr(MSG,"%s\t(%3.1f)\n",m->m_name,m->m_weight);
	pr(MSG,"\n");
}



void g_mod_delete(g_module *m)
{
	if (m)
	{
		if (m->m_name) free((void *)m->m_name);
		free(m);
		g_count--;
	}
}



void g_mod_deleteall()
{
	g_module	*m;

	while (g_head)
	{
		m = g_head->m_next; g_mod_delete(g_head); g_head = m;
	}
}



/*
 * set weight of module and re-insert as head. 
 */

g_module *g_mod_setweight(char *name,float weight)
{
	g_module	*m, *prev = 0;

	for (m = g_head; m; m = m->m_next)
		if (strcmp(m->m_name,name) == 0)
			break;
		else
			prev = m;
	if (m == 0)
		return (0);
	if (prev)
	{
		prev->m_next = m->m_next;
		m->m_next = g_head;
		g_head = m;
	}
	g_head->m_weight = weight;
	return (g_head);
}



g_module *g_mod_lookup(int how,char *name)
{
	g_module	*m;

	if (g_head == 0)
	{
		if (how == GM_LOOKUP)
			return (0);
		g_head = (g_module *)alloc(sizeof(g_module));
		m = g_head;
	}
	else
	{
		for (m = g_head; m->m_next; m = m->m_next)
			if (strcmp(m->m_name,name) == 0)
				return (m);
		if (how == GM_LOOKUP)
			return (0);
		m->m_next = (g_module *)alloc(sizeof(g_module));
		m = m->m_next;
	}
	if ((m->m_name = strdup(name)) == 0)
		pr(FATAL,"out of memory in strdup");
	m->m_weight = 1.0; g_count++;
	return (m);
}

/*
 * preloaded modules
 */

void g_mod_addinternals()
{
	g_module	*m;

#define GMODINS(mod)	\
	do { \
		m = g_mod_lookup(GM_INSERT,#mod); \
		if (!m) {		\
			m->m_init=mod##_init; \
			m->m_term=mod##_term; \
			m->m_gfun=mod##_gfun; \
		} \
	} while (0);

	/*
	 * If no weights are given on the command line, the order
	 * is somehow important.
	 */

	GMODINS(bsddl);
	GMODINS(lswap);
	GMODINS(qnx4);
	GMODINS(reiserfs);
	GMODINS(ntfs);
	GMODINS(hpfs);
	GMODINS(minix);
	GMODINS(beos);
	GMODINS(ext2);
	GMODINS(fat);
	GMODINS(s86dl);
	GMODINS(hmlvm);
	GMODINS(xfs);
}
