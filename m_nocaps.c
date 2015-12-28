/* m_nocaps: a very simple module originally written
 * by Grunt. Heavily based on m_nocodes, by Syzop
 * and m_blocknonalpha, by Certus. Currently maintained
 * by dboyz.
 *
 * This module will block messages in all caps to a channel
 * that has mode +x set. Does not take into consideration
 * non-alpha characters like: '!' '?' '^' and etc. Thus:
 * "A!A!A!???" is considered to be in caps!
 * 
 * Changes:
 *
 * 1.5 (DBoyz) [26th December 2015]
 * ---- Update to UnrealIRCd-4.0 codebase.
 * 1.4.1 (DBoyz) [25th December 2015]
 * ---- Code tidying and fixing silly mistakes.
 * 1.4 (DBoyz) [22nd April 2015]
 * ---- Reindentation, introduce missing braces and remove whitespaces.
 * ---- Switch '&&' to '&' boolean operator as pointed out by Jobe in 04 August 2013.
 * ---- Check for errors during module loading, report it and discontinue loading.
 * ---- Switch to channel mode +x from previous +d (now occupied in unreal 3.4)
 * 1.3 (DBoyz) [22nd October 2012]
 * ---- Fixed a bug by adding line 151.
 * 1.2
 * ---- Added option to convert the message to lowercase
 * 1.1
 * ---- Code cleanup
 * ---- Optimisation (stop searching when a lowercase letter is found)
 * 1.0
 * ---- Initial release
 */

#include "unrealircd.h"

/*
 * NOCAPS_ACTION - what action to take?
 * 1 = BLOCK
 * 0 = CONVERT the message to lowercase
 * any other number = DO NOTHING
 */

#define NOCAPS_ACTION 1

Cmode_t NOCAPS_BLOCK = 0L;
Cmode *ModeBlock = NULL;
static Hook *CheckMsg;
char *nocaps_checkmsg(aClient *, aChannel *, char *, int);

ModuleHeader MOD_HEADER(m_nocaps)
= {
	"m_nocaps",
	"v1.5",
	"chmode +x - Blocks all caps messages sent to channels (Grunt, DBoyz)",
	"3.2-b8-1",
	NULL
};

MOD_INIT(m_nocaps)
{
	CmodeInfo req;
	ircd_log(LOG_ERROR, "debug: mod_init called from m_nocaps");
	sendto_realops("loading m_nocaps");
	memset(&req, 0, sizeof(req));
	req.paracount = 0;
	req.is_ok = extcmode_default_requirehalfop;
	req.flag = 'x';
	if (!(ModeBlock = CmodeAdd(modinfo->handle, req, &NOCAPS_BLOCK)))
	{
		config_error("nocaps: Could not add channel mode +x : %s",
		    ModuleGetErrorStr(modinfo->handle));
		return MOD_FAILED;
	}
	CheckMsg = HookAddPChar(modinfo->handle, HOOKTYPE_PRE_CHANMSG, 0, nocaps_checkmsg);
	return MOD_SUCCESS;
}

MOD_LOAD(m_nocaps)
{
	return MOD_SUCCESS;
}

MOD_UNLOAD(m_nocaps)
{
	HookDel(CheckMsg);
	CmodeDel(ModeBlock);
	ircd_log(LOG_ERROR, "debug: mod_unload called from m_nocaps");
	sendto_realops("unloading m_nocaps");
	return MOD_SUCCESS;
}

char *nocaps_checkmsg(aClient *sptr, aChannel *chptr, char *text, int notice)
{
	int contor;
	unsigned short int rezultat=0;
	if (chptr->mode.extmode & NOCAPS_BLOCK)
	{
		for (contor=0 ; contor<=strlen(text) && (!rezultat) ; contor++)
		{
			if (islower(text[contor]))
			rezultat=1;
		}
		if (rezultat==1)
		{
			return text;
		}
		else
		{
			switch(NOCAPS_ACTION)
			{
			case 1 :
				sendto_one(sptr, err_str(ERR_CANNOTSENDTOCHAN), me.name, sptr->name, chptr->chname,
				    "Messages with all letters in caps are not permitted in this channel",
				    chptr->chname);
				return NULL;
				break;
			case 0 :
				for (contor=0 ; contor<=strlen(text) ; contor++) {
					if (isupper(text[contor]))
					{
						text[contor]=tolower(text[contor]);
					}
				}
				sendto_one(sptr, err_str(ERR_CANNOTSENDTOCHAN), me.name, sptr->name, chptr->chname,
				    "Your message to this channel was converted to lowercase",
				    chptr->chname);
				return text;
				break;
			}
		}
	}
	else
	{
		return text;
	}
}
