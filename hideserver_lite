/*
 * =================================================================
 * Filename:             hideserver_lite.c
 * Description:          Limits MAP & LINKS to opers only.
 * Written by:           AngryWolf <angrywolf@flashmail.com>
 *                       Daniel Tan (DBoyz) <danieltan1901 at gmail dot com>
 * This module is closely based on AngryWolf's hideserver.c
 * =================================================================
 */

#include "unrealircd.h"

#define DelOverride(cmd, ovr) if (ovr && CommandExists(cmd)) CmdoverrideDel(ovr); ovr = NULL

Cmdoverride *AddOverride(char *msg, iFP cb);
static int override_map(Cmdoverride *, aClient *, aClient *, int, char *[]);
static int override_links(Cmdoverride *, aClient *, aClient *, int, char *[]);

Cmdoverride *OvrMap, *OvrLinks;

static ModuleInfo *MyModInfo;
#define SAVE_MODINFO MyModInfo = modinfo

ModuleHeader MOD_HEADER(hideserver)
= {
	"hideserver_lite",
	"v0.1",
	"Limits MAP & LINKS to opers only (AngryWolf, DBoyz)",
	"3.2-b8-1",
	NULL
};

MOD_TEST(hideserver)
{
	SAVE_MODINFO;
	return MOD_SUCCESS;
}

MOD_INIT(hideserver)
{
	SAVE_MODINFO;
	return MOD_SUCCESS;
}

MOD_LOAD(hideserver)
{
	OvrMap = AddOverride("map", override_map);
	OvrLinks = AddOverride("links", override_links);

	if (!OvrMap || !OvrLinks)
		return MOD_FAILED;

	return MOD_SUCCESS;
}

MOD_UNLOAD(hideserver)
{
	DelOverride("map", OvrMap);
	DelOverride("links", OvrLinks);

	return MOD_SUCCESS;
}

Cmdoverride *AddOverride(char *msg, iFP cb)
{
	Cmdoverride *ovr = CmdoverrideAdd(MyModInfo->handle, msg, cb);

	if (!ovr)
	{
		config_error("Error replacing command %s when loading module %s: %s",
			msg, MOD_HEADER(hideserver).name, ModuleGetErrorStr(MyModInfo->handle));
		return NULL;
	}

	return ovr;
}

static int override_map(Cmdoverride *ovr, aClient *cptr, aClient *sptr, int parc, char *parv[])
{
	if (!IsClient(sptr) || IsOper(sptr))
		return CallCmdoverride(ovr, cptr, sptr, parc, parv);

	sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, sptr->name);
	return 0;
}

static int override_links(Cmdoverride *ovr, aClient *cptr, aClient *sptr, int parc, char *parv[])
{
	if (!IsClient(sptr) || IsOper(sptr))
		return CallCmdoverride(ovr, cptr, sptr, parc, parv);

	sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, sptr->name);
	return 0;
}
