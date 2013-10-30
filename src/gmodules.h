/*
 * gmodules.h -- gpart module header file
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

#ifndef _GMODULES_H
#define _GMODULES_H


#define GM_NO		(0.0)		/* predefined probabilities */
#define GM_PERHAPS	(0.5)
#define GM_YES		(0.8)
#define GM_UNDOUBTEDLY	(1.0)

typedef struct g_mod
{
	char		*m_name;	/* name of module */
	char		*m_desc;	/* readable description */
	int		(*m_init)(disk_desc *,struct g_mod *);
	int		(*m_term)(disk_desc *);
	int		(*m_gfun)(disk_desc *,struct g_mod *);
	float		m_guess;
	float		m_weight;	/* probability weight */
	void		*m_hd;		/* dlopen() descriptor */
	dos_part_entry	m_part;		/* a guessed partition entry */
	long		m_align;	/* alignment of partition */
	struct g_mod	*m_next;
	unsigned int	m_hasptbl : 1;	/* has a ptbl like entry in sec 0 */
	unsigned int	m_notinext : 1;	/* cannot exist in an ext part. */
	unsigned int	m_skip : 1;	/* skip this module this time */
} g_module;

#define GM_LOOKUP	0
#define GM_INSERT	1

void g_mod_list(), g_mod_delete(g_module *), g_mod_deleteall();
g_module *g_mod_head(), *g_mod_lookup(int,char *);
void g_mod_addinternals();
int g_mod_count(), g_mod_addexternal(char *);
g_module *g_mod_setweight(char *,float);



/*
 * preloaded guessing modules
 */

#define GMODDECL(mod)	int mod##_init(disk_desc *,g_module *),	\
			mod##_term(disk_desc *),		\
			mod##_gfun(disk_desc *,g_module *)

GMODDECL(bsddl); GMODDECL(ext2); GMODDECL(fat);
GMODDECL(hpfs); GMODDECL(lswap); GMODDECL(ntfs);
GMODDECL(s86dl); GMODDECL(minix); GMODDECL(rfs);
GMODDECL(hmlvm); GMODDECL(qnx4); GMODDECL(beos);
GMODDECL(xfs);


#endif /* _GMODULES_H */
