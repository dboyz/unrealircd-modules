/* noamsg.c: Disables AME and AMSG network-wide, opers and ulines are exempted.
 * Written by: Daniel Tan (DBoyz) <danieltan1901@gmail.com>
 * Idea is taken from InspIRCd's m_blockamsg.cpp
 */

#include "unrealircd.h"

#define DelOverride(cmd, ovr) if (ovr && CommandExists(cmd)) { CmdoverrideDel(ovr); ovr = NULL; }
#define ircfree(x) if (x) MyFree(x); x = NULL

static ModuleInfo *MyModInfo = NULL;
Cmdoverride *OvrP = NULL;
static int override_noamsg(Cmdoverride *, aClient *, aClient *, int, char *[]);

ModuleHeader MOD_HEADER(noamsg)
= {
	"noamsg",
	"v0.1",
	"Disables AMSG and AME command (DBoyz)",
	"3.2-b8-1",
	NULL
};

MOD_TEST(noamsg)
{
	return MOD_SUCCESS;
}

MOD_INIT(noamsg)
{
	ircd_log(LOG_ERROR, "debug: mod_init called from noamsg");

	sendto_realops("loading noamsg");

	MyModInfo = modinfo;

	return MOD_SUCCESS;
}

MOD_LOAD(noamsg)
{
	OvrP = CmdoverrideAdd(MyModInfo->handle, "PRIVMSG", override_noamsg);

	if (ModuleGetError(MyModInfo->handle) != MODERR_NOERROR || !OvrP)
	{
		config_error("Error replacing command PRIVMSG when loading module %s: %s",
			MOD_HEADER(noamsg).name, ModuleGetErrorStr(MyModInfo->handle));

		ircfree(OvrP);

		return MOD_FAILED;
	}

	return MOD_SUCCESS;
}

MOD_UNLOAD(noamsg)
{
	ircd_log(LOG_ERROR, "debug: mod_unload called from noamsg");

	sendto_realops("unloading noamsg");

	DelOverride("PRIVMSG", OvrP);

	ircfree(OvrP);

	return MOD_SUCCESS;
}

static int override_noamsg(Cmdoverride *ovr, aClient *cptr, aClient *sptr, int parc, char *parv[])
{
	char output_target[1024], temp_target[1024];
	memset(output_target, 0, strlen(output_target));
	memset(temp_target, 0, strlen(temp_target));
	char *nick = NULL, *p = NULL;
	unsigned short count_channel = 0;

	if (IsHandshake(sptr))
		return 0;

	if (IsOper(sptr) || IsULine(sptr))
		return CallCmdoverride(OvrP, cptr, sptr, parc, parv);

	if (parc < 2 || *parv[1] == '\0')
	{
		sendto_one(sptr, err_str(ERR_NORECIPIENT),
			me.name, sptr->name, "PRIVMSG");
		return -1;
	}

	if (parc < 3 || *parv[2] == '\0')
	{
		sendto_one(sptr, err_str(ERR_NOTEXTTOSEND), me.name, sptr->name);
		return -1;
	}

	if (MyConnect(sptr))
		parv[1] = (char *)canonize(parv[1]);

	for (p = NULL, nick = strtoken(&p, parv[1], ","); nick; nick = strtoken(&p, NULL, ","))
	{
		if (strchr(nick, '#'))
		{
			if (++count_channel > 1)
			{
				temp_target[0] = '\0';
				continue;
			}
			else
			{
				strncat(temp_target, nick, sizeof(temp_target)-strlen(temp_target)-1);
				strncat(temp_target, ",", sizeof(temp_target)-strlen(temp_target)-1);
			}
		}
		else
		{
			strncat(output_target, nick, sizeof(output_target)-strlen(output_target)-1);
			strncat(output_target, ",", sizeof(output_target)-strlen(output_target)-1);
		}
	}

	strncat(output_target,temp_target, sizeof(output_target)-strlen(output_target)-1);

	if (*(output_target + strlen(output_target) - 1) == ',')
		*(output_target + strlen(output_target) - 1) = '\0';

	if (count_channel > 1)
	{
		sendto_one(sptr, ":%s 404 %s %s :%s",me.name, sptr->name, "*",
			"AMSG and AME has been disabled");
	}

	parv[1] = output_target;

	if (*parv[1] == '\0')
	{
		return -1;
	}

	ircfree(nick);
	ircfree(p);

	return CallCmdoverride(OvrP, cptr, sptr, parc, parv);
}
