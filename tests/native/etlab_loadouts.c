/* ETLab regression tests using the production shared rules and spawn function. */
#include "../src/game/g_local.h"


static int checks;

static void Check(qboolean condition, const char *message)
{
	checks++;
	if (!condition)
	{
		fprintf(stderr, "FAIL: %s\n", message);
		exit(1);
	}
}

static void CheckSpawn(int team, int cls, weapon_t primary, int skillLevel)
{
	gclient_t client;
	bg_playerclass_t *info = BG_GetPlayerClassInfo(team, cls);
	weapon_t akimbo = cls == PC_COVERTOPS
		? (team == TEAM_AXIS ? WP_AKIMBO_SILENCEDLUGER : WP_AKIMBO_SILENCEDCOLT)
		: (team == TEAM_AXIS ? WP_AKIMBO_LUGER : WP_AKIMBO_COLT);
	qboolean bonus = cls == PC_SOLDIER && (primary == WP_PANZERFAUST || primary == WP_BAZOOKA || primary == WP_FLAMETHROWER);
	int i;

	Com_Memset(&client, 0, sizeof(client));
	client.sess.sessionTeam = team;
	client.sess.playerType = cls;
	client.sess.playerWeapon = primary;
	client.sess.playerWeapon2 = info->classSecondaryWeapons[0].weapon;
	client.sess.skill[SK_LIGHT_WEAPONS] = skillLevel;
	if (skillLevel == 4)
	{
		client.sess.playerWeapon2 = akimbo;
	}
	SetWolfSpawnWeapons(&client);
	Check(COM_BitCheck(client.ps.weapons, primary), "selected primary retained");
	Check(COM_BitCheck(client.ps.weapons, WP_PLIERS), "pliers on every class/team");
	Check(client.ps.stats[STAT_KEYS] & (1 << INV_BINOCS), "binoculars at zero Battle Sense XP");
	Check(COM_BitCheck(client.ps.weapons, akimbo) == (bonus || skillLevel == 4), "conditional bonus and normal XP unlock");
	Check(client.ps.weapon == primary, "primary initially selected");
	Check(client.sess.skill[SK_LIGHT_WEAPONS] == skillLevel, "no artificial XP promotion");
	if (cls != PC_ENGINEER)
	{
		Check(!COM_BitCheck(client.ps.weapons, WP_DYNAMITE), "no extra dynamite");
		Check(!COM_BitCheck(client.ps.weapons, WP_LANDMINE), "no extra mines");
	}
	if (bonus || skillLevel == 4)
	{
		Check(client.ps.ammoclip[GetWeaponTableData(akimbo)->clipIndex] == 8, "first Akimbo clip loaded");
		Check(client.ps.ammoclip[GetWeaponTableData(GetWeaponTableData(akimbo)->akimboSideArm)->clipIndex] == 8, "second Akimbo clip loaded");
		Check(client.ps.ammo[GetWeaponTableData(akimbo)->ammoIndex] >= 48, "Akimbo reserve ammo");
	}
	for (i = 0; i < MAX_WEAPS_PER_CLASS && info->classSecondaryWeapons[i].weapon; i++)
	{
		if (info->classSecondaryWeapons[i].weapon == akimbo)
		{
			Check(BG_IsSecondaryWeaponAvailable(info, team, primary, i, client.sess.skill) == (bonus || skillLevel == 4), "menu and server agree");
		}
	}
	// Reuse the same client state: changing class/loadout must clear the bonus.
	client.sess.playerType = PC_MEDIC;
	client.sess.playerWeapon = team == TEAM_AXIS ? WP_MP40 : WP_THOMPSON;
	client.sess.skill[SK_LIGHT_WEAPONS] = 0;
	SetWolfSpawnWeapons(&client);
	Check(!COM_BitCheck(client.ps.weapons, WP_AKIMBO_LUGER) && !COM_BitCheck(client.ps.weapons, WP_AKIMBO_COLT), "no stale bonus after class change");
	Check(COM_BitCheck(client.ps.weapons, WP_PLIERS), "pliers retained after class change");
}

/* Engine geometry is controlled here; all gameplay functions are the real module. */
extern gclient_t g_clients[MAX_CLIENTS];
extern void dllEntry(intptr_t(QDECL *syscallptr)(intptr_t arg, ...));
extern gentity_t *Weapon_Engineer(gentity_t *ent);
extern void G_CheckSpottedLandMines(void);
extern void G_ArmLandmine(gentity_t *mine, gentity_t *player);
static int traceTarget = ENTITYNUM_NONE;
static char savedModifier[64];
static qboolean radiusQuery, blocked;
static char endgameStats[BIG_INFO_STRING], lastAward[MAX_STRING_CHARS];
static int chatLines, awardCommands;

static intptr_t QDECL TestSyscall(intptr_t arg, ...)
{
	va_list ap;
	intptr_t result = 0;
	va_start(ap, arg);
	switch (arg)
	{
	case G_PRINT: case G_LOCATE_GAME_DATA: case G_FS_WRITE:
		break;
	case G_SEND_SERVER_COMMAND:
	{
		const char *text;
		(void)va_arg(ap, int);
		text = va_arg(ap, const char *);
		if (!Q_strncmp(text, "chat ", 5)) { chatLines++; }
		if (!Q_strncmp(text, "etlabaward", 10)) { awardCommands++; Q_strncpyz(lastAward, text, sizeof(lastAward)); }
		break;
	}
	case G_SET_CONFIGSTRING:
	{
		int num = va_arg(ap, int);
		const char *text = va_arg(ap, const char *);
		if (num == CS_ENDGAME_STATS) { Q_strncpyz(endgameStats, text, sizeof(endgameStats)); }
		break;
	}
	case G_CVAR_VARIABLE_INTEGER_VALUE: case G_REAL_TIME:
		break;
	case G_CVAR_SET:
	{
		const char *name = va_arg(ap, const char *), *value = va_arg(ap, const char *);
		if (!Q_stricmp(name, "etlab_modifier")) { Q_strncpyz(savedModifier, value, sizeof(savedModifier)); }
		break;
	}
	case G_CVAR_VARIABLE_STRING_BUFFER:
	{
		const char *name = va_arg(ap, const char *);
		char *buffer = va_arg(ap, char *);
		int size = va_arg(ap, int);
		Q_strncpyz(buffer, Q_stricmp(name, "etlab_modifier") ? "" : savedModifier, size);
		break;
	}
	case G_LINKENTITY: case G_UNLINKENTITY:
	{
		gentity_t *e = va_arg(ap, gentity_t *);
		e->r.linked = arg == G_LINKENTITY;
		VectorAdd(e->r.currentOrigin, e->r.mins, e->r.absmin);
		VectorAdd(e->r.currentOrigin, e->r.maxs, e->r.absmax);
		break;
	}
	case G_IN_PVS: result = 1; break;
	case G_ENTITIES_IN_BOX:
		(void)va_arg(ap, const float *); (void)va_arg(ap, const float *);
		if (radiusQuery) { int *list = va_arg(ap, int *); list[0] = MAX_CLIENTS; result = 1; }
		break;
	case G_TRACE:
	{
		trace_t *tr = va_arg(ap, trace_t *);
		const float *end;
		(void)va_arg(ap, const float *); (void)va_arg(ap, const float *); (void)va_arg(ap, const float *);
		end = va_arg(ap, const float *);
		Com_Memset(tr, 0, sizeof(*tr));
		tr->fraction = blocked ? 0.5f : (traceTarget == ENTITYNUM_NONE ? 1.f : 0.01f);
		tr->entityNum = blocked ? ENTITYNUM_WORLD : traceTarget;
		if (traceTarget == ENTITYNUM_NONE) { VectorCopy(end, tr->endpos); }
		break;
	}
	default:
		fprintf(stderr, "Unexpected engine syscall %d\n", (int)arg);
		exit(1);
	}
	va_end(ap);
	return result;
}

static void ResetScene(void)
{
	Com_Memset(g_entities, 0, sizeof(gentity_t) * MAX_GENTITIES);
	Com_Memset(&level, 0, sizeof(level));
	level.num_entities = MAX_CLIENTS + 2;
	level.time = 60000;
	level.clients = g_clients;
	g_corpses.integer = 1;
	g_gamestate.integer = GS_WARMUP;
	G_InitMapEntityData(&mapEntityData[0]);
	G_InitMapEntityData(&mapEntityData[1]);
	traceTarget = ENTITYNUM_NONE;
	radiusQuery = blocked = qfalse;
}

static void CheckPliersActions(int team, int cls)
{
	gclient_t client;
	gentity_t *target = &g_entities[MAX_CLIENTS];
	gentity_t *player = &g_entities[0];
	int kind;
	ResetScene();
	Com_Memset(&client, 0, sizeof(client));
	player->client = &client;
	client.sess.sessionTeam = team;
	client.sess.playerType = cls;
	client.ps.stats[STAT_PLAYER_CLASS] = cls;
	client.ps.weapon = WP_PLIERS;
	level.engineerChargeTime[team - 1] = 30000;
	traceTarget = MAX_CLIENTS;
	for (kind = 0; kind < 6; kind++)
	{
		Com_Memset(target, 0, sizeof(*target));
		target->s.number = MAX_CLIENTS;
		target->s.eType = kind < 5 ? ET_MISSILE : ET_CONSTRUCTIBLE;
		target->methodOfDeath = kind < 2 ? MOD_DYNAMITE : (kind < 4 ? MOD_LANDMINE : MOD_SATCHEL);
		target->s.effect1Time = kind == 0 || kind == 2 || kind == 4;
		target->classname = "test_target";
		Check(G_CanUsePliersOn(player, target) == (cls == PC_ENGINEER || kind == 0 || kind == 2), "pliers eligibility: only armed dynamite/mines outside Engineer");
		if (cls != PC_ENGINEER || kind == 0 || kind == 2)
		{
			Weapon_Engineer(player);
			Check(target->health == ((kind == 0 || kind == 2) ? 3 : 0), "real pliers dispatch blocks arming, satchel and construction");
		}
	}
	target->classname = "misc_mg42";
	target->s.eType = ET_GENERAL;
	target->s.frame = 1;
	target->health = 0;
	Weapon_Engineer(player);
	Check(target->health == (cls == PC_ENGINEER ? 3 : 0), "only Engineer repairs emplaced guns");
	// Completing a defuse must remove the mine without giving other classes mine ammo.
	Com_Memset(target, 0, sizeof(*target));
	target->inuse = qtrue;
	target->s.number = MAX_CLIENTS;
	target->s.eType = ET_MISSILE;
	target->s.weapon = WP_LANDMINE;
	target->methodOfDeath = MOD_LANDMINE;
	target->s.effect1Time = 1;
	target->s.teamNum = team;
	target->health = 248;
	target->classname = "landmine";
	Weapon_Engineer(player);
	Check(!target->inuse, "completed mine defuse removes target for all classes");
	if (cls != PC_ENGINEER)
	{
		Check(client.ps.ammo[WP_LANDMINE] == 0 && client.ps.ammoclip[WP_LANDMINE] == 0, "defusing does not supply mines to other classes");
	}
	player->client = NULL;
}

static void CheckMineDamage(void)
{
	const meansOfDeath_t mods[] = { MOD_GRENADE_LAUNCHER, MOD_GRENADE_PINEAPPLE, MOD_LUGER, MOD_COLT, MOD_MP40, MOD_PANZERFAUST, MOD_KNIFE };
	int spotted, i;
	for (spotted = 0; spotted < 2; spotted++)
	{
		for (i = 0; i < ARRAY_LEN(mods); i++)
		{
			gentity_t *mine = &g_entities[MAX_CLIENTS];
			qboolean expected = i < 2 || (spotted && i < 6);
			ResetScene();
			mine->s.number = MAX_CLIENTS;
			mine->s.eType = ET_MISSILE;
			mine->s.weapon = WP_LANDMINE;
			mine->methodOfDeath = MOD_LANDMINE;
			mine->s.effect1Time = 1;
			mine->s.modelindex2 = spotted;
			mine->takedamage = qtrue;
			G_Damage(mine, NULL, NULL, NULL, NULL, 30, 0, mods[i]);
			Check(mine->freeAfterEvent == expected, "mine detonation: hidden hand grenades only, spotted bullets/explosives");
		}
	}
	for (i = 0; i < 3; i++)
	{
		gentity_t *mine = &g_entities[MAX_CLIENTS];
		vec3_t origin = { 0, 0, 0 };
		ResetScene();
		mine->s.number = MAX_CLIENTS;
		mine->s.eType = ET_MISSILE;
		mine->s.weapon = WP_LANDMINE;
		mine->methodOfDeath = MOD_LANDMINE;
		mine->s.effect1Time = 1;
		mine->takedamage = qtrue;
		VectorSet(mine->r.currentOrigin, i == 2 ? 500 : 100, 0, 0);
		VectorCopy(mine->r.currentOrigin, mine->r.absmin);
		VectorCopy(mine->r.currentOrigin, mine->r.absmax);
		radiusQuery = qtrue;
		blocked = i == 1;
		G_RadiusDamage(origin, &g_entities[ENTITYNUM_WORLD], &g_entities[ENTITYNUM_WORLD], 250, 250, NULL, MOD_GRENADE_PINEAPPLE);
		Check(mine->freeAfterEvent == (i == 0), "real grenade radius: nearby hidden mine, cover and distance respected");
	}
}

static void CheckSpotting(int team, int cls)
{
	gclient_t client;
	gentity_t *player = &g_entities[0], *mine = &g_entities[MAX_CLIENTS];
	int i;
	ResetScene();
	Com_Memset(&client, 0, sizeof(client));
	player->client = &client;
	player->inuse = qtrue;
	player->health = 100;
	client.sess.sessionTeam = team;
	client.sess.playerType = cls;
	client.ps.stats[STAT_KEYS] = 1 << INV_BINOCS;
	level.numConnectedClients = 1;
	mine->s.number = MAX_CLIENTS;
	mine->inuse = qtrue;
	mine->s.eType = ET_MISSILE;
	mine->methodOfDeath = MOD_LANDMINE;
	mine->s.weapon = WP_LANDMINE;
	mine->s.effect1Time = 1;
	mine->s.teamNum = team == TEAM_AXIS ? TEAM_ALLIES : TEAM_AXIS;
	VectorSet(mine->r.currentOrigin, 500, 0, 0);
	G_CheckSpottedLandMines();
	Check(!mine->s.modelindex2 && !mine->count2, "no passive binocular spotting");
	client.ps.eFlags |= EF_ZOOMING;
	for (i = 0; i < 5; i++)
	{
		level.time += 500;
		G_CheckSpottedLandMines();
	}
	Check(mine->s.modelindex2 && mine->count2 == 250, "normal binocular dwell spots enemy mine for every class");
	Check((mine->r.contents & MASK_SHOT) && mine->r.maxs[2] == 40, "flag has bullet hitbox");
	Check(!(mine->r.contents & MASK_PLAYERSOLID), "flag does not block walking");
	player->client = NULL;
}

static void SetPool(const char *pool, int mode)
{
	Q_strncpyz(g_etlabModifierPool.string, pool, sizeof(g_etlabModifierPool.string));
	g_etlabModifierMode.integer = mode;
}

static void SpawnWith(gclient_t *client, int team, int cls, weapon_t primary, int modifier)
{
	Com_Memset(client, 0, sizeof(*client));
	client->sess.sessionTeam  = team;
	client->sess.playerType   = cls;
	client->sess.playerWeapon = primary;
	level.etlabModifier       = modifier;
	SetWolfSpawnWeapons(client);
}

static void CheckModifiers(void)
{
	gclient_t client, plain;
	gentity_t *player = &g_entities[0], *other = &g_entities[1];
	int i, pool[ETLAB_MOD_COUNT];

	// Shared table and pool parsing
	Check(ETLAB_MOD_COUNT == 17 && !Q_stricmp(etlabModifiers[ETLAB_MOD_NONE].name, "none"), "modifier table starts with none");
	Check(BG_ETLabModifierByName("doublexp") == ETLAB_MOD_DOUBLEXP && BG_ETLabModifierByName("VAMPIRE") == ETLAB_MOD_VAMPIRE, "names resolve case-insensitively");
	Check(BG_ETLabModifierByName("none") == ETLAB_MOD_NONE && BG_ETLabModifierByName("bogus") == -1 && BG_ETLabModifierByName("") == -1, "unknown names rejected");
	Check(BG_ETLabModifierInPool("all", ETLAB_MOD_HEADSHOTS) && BG_ETLabModifierInPool("doublexp, vampire;lowammo", ETLAB_MOD_VAMPIRE), "pool keywords and separators");
	Check(!BG_ETLabModifierInPool("doublexp vampire", ETLAB_MOD_LOWAMMO) && !BG_ETLabModifierInPool("", ETLAB_MOD_LOWAMMO) && !BG_ETLabModifierInPool("all", ETLAB_MOD_NONE), "pool excludes others and none");
	Check(BG_ETLabModifierPoolList("all", pool) == ETLAB_MOD_COUNT - 1 && pool[0] == ETLAB_MOD_DOUBLEXP, "full pool lists every modifier once");
	Check(BG_ETLabModifierPoolList("vampire doublexp", pool) == 2 && pool[0] == ETLAB_MOD_DOUBLEXP && pool[1] == ETLAB_MOD_VAMPIRE, "pool list keeps table order");
	for (i = ETLAB_MOD_NONE + 1; i < ETLAB_MOD_COUNT; i++)
	{
		Check(etlabModifiers[i].title[0] && etlabModifiers[i].description[0] && BG_ETLabModifierByName(etlabModifiers[i].name) == i, "every modifier has a unique name, title and description");
	}

	// Selection: random draws only from the pool, vote-only mode starts empty, off ignores the pool
	ResetScene();
	SetPool("lowammo", ETLAB_MODE_RANDOM);
	G_ETLabModifierInit(qfalse);
	Check(level.etlabModifier == ETLAB_MOD_LOWAMMO && !Q_stricmp(savedModifier, "lowammo"), "random mode draws from the pool and saves the choice");
	SetPool("all", ETLAB_MODE_RANDOM_VOTE);
	for (i = 0; i < 50; i++)
	{
		G_ETLabModifierInit(qfalse);
		Check(level.etlabModifier > ETLAB_MOD_NONE && level.etlabModifier < ETLAB_MOD_COUNT, "random draw stays inside the table");
	}
	SetPool("all", ETLAB_MODE_VOTE);
	G_ETLabModifierInit(qfalse);
	Check(level.etlabModifier == ETLAB_MOD_NONE, "vote mode starts without a modifier");
	SetPool("all", ETLAB_MODE_OFF);
	G_ETLabModifierInit(qfalse);
	Check(level.etlabModifier == ETLAB_MOD_NONE, "mode 0 never activates a modifier");
	SetPool("", ETLAB_MODE_RANDOM);
	G_ETLabModifierInit(qfalse);
	Check(level.etlabModifier == ETLAB_MOD_NONE, "empty pool means no modifier");
	// map_restart keeps the saved modifier, but only while it is still in the pool
	Q_strncpyz(savedModifier, "sidearms", sizeof(savedModifier));
	SetPool("all", ETLAB_MODE_RANDOM);
	G_ETLabModifierInit(qtrue);
	Check(level.etlabModifier == ETLAB_MOD_SIDEARMS, "restart keeps the saved modifier");
	Q_strncpyz(savedModifier, "sidearms", sizeof(savedModifier));
	SetPool("doublexp", ETLAB_MODE_RANDOM);
	G_ETLabModifierInit(qtrue);
	Check(level.etlabModifier == ETLAB_MOD_NONE, "restart drops a modifier removed from the pool");

	// Vote: refused unless the mode allows votes, validated against the pool, applied on success
	ResetScene();
	SetPool("doublexp vampire", ETLAB_MODE_RANDOM);
	{
		char arg[16], arg2[VOTE_MAXSTRING];
		Q_strncpyz(arg, "modifier", sizeof(arg));
		Q_strncpyz(arg2, "doublexp", sizeof(arg2));
		Check(G_ETLabModifier_v(NULL, 0, arg, arg2, qfalse) == G_INVALID, "random-only mode refuses modifier votes");
		SetPool("doublexp vampire", ETLAB_MODE_VOTE);
		Q_strncpyz(arg2, "lowammo", sizeof(arg2));
		Check(G_ETLabModifier_v(NULL, 0, arg, arg2, qfalse) == G_INVALID, "vote outside the pool refused");
		Q_strncpyz(arg2, "bogus", sizeof(arg2));
		Check(G_ETLabModifier_v(NULL, 0, arg, arg2, qfalse) == G_INVALID, "unknown vote name refused");
		Q_strncpyz(arg2, "vampire", sizeof(arg2));
		Check(G_ETLabModifier_v(NULL, 0, arg, arg2, qfalse) == G_OK && !Q_stricmp(level.voteInfo.vote_value, "vampire") && !Q_stricmp(arg2, "Vampire"), "valid vote stores value and display title");
		Check(G_ETLabModifier_v(NULL, 0, NULL, NULL, qfalse) == G_OK && level.etlabModifier == ETLAB_MOD_VAMPIRE, "passed vote activates the modifier");
		Q_strncpyz(arg2, "vampire", sizeof(arg2));
		Check(G_ETLabModifier_v(NULL, 0, arg, arg2, qfalse) == G_INVALID, "voting the active modifier again is refused");
		Q_strncpyz(arg2, "none", sizeof(arg2));
		Check(G_ETLabModifier_v(NULL, 0, arg, arg2, qfalse) == G_OK, "none is always a valid vote");
		G_ETLabModifier_v(NULL, 0, NULL, NULL, qfalse);
		Check(level.etlabModifier == ETLAB_MOD_NONE, "none vote clears the modifier");
	}

	// Rule helpers
	ResetScene();
	team_maxLandmines.integer = 10;
	level.etlabModifier = ETLAB_MOD_NONE;
	Check(G_ETLabLimboTime(30000) == 30000 && G_ETLabDynamiteFuse() == 30000 && G_ETLabTeamMaxLandmines() == 10, "no modifier keeps stock values");
	level.etlabModifier = ETLAB_MOD_FASTSPAWN;
	Check(G_ETLabLimboTime(30000) == 15000 && G_ETLabLimboTime(1500) == 1500, "Fast Reinforcements halves usable limbo times only");
	level.etlabModifier = ETLAB_MOD_LONGDYNO;
	Check(G_ETLabDynamiteFuse() == 45000, "Long Fuse: 45 seconds");
	level.etlabModifier = ETLAB_MOD_ENGINEERFEST;
	Check(G_ETLabTeamMaxLandmines() == 20, "Engineer Fest doubles the landmine limit");
	team_maxLandmines.integer = -1;
	Check(G_ETLabTeamMaxLandmines() == -1, "unlimited landmine setting passes through");
	team_maxLandmines.integer = 10;
	// G_InitGame sets the map-script factors to 1 before the modifier is initialised
	level.medicChargeTimeModifier[0] = level.medicChargeTimeModifier[1] = 1.f;
	level.engineerChargeTimeModifier[0] = level.engineerChargeTimeModifier[1] = 1.f;
	g_medicChargeTime.integer = 40000;
	g_engineerChargeTime.integer = 30000;
	G_ETLabApplyChargeTimes();
	Check(level.engineerChargeTime[0] == 15000 && level.engineerChargeTime[1] == 15000 && level.medicChargeTime[0] == 40000, "Engineer Fest halves engineer charge only");
	level.etlabModifier = ETLAB_MOD_MEDICSHORTAGE;
	G_ETLabApplyChargeTimes();
	Check(level.medicChargeTime[1] == 60000 && level.engineerChargeTime[1] == 30000, "Medic Shortage slows medic charge by half");
	level.etlabModifier = ETLAB_MOD_NONE;
	G_ETLabApplyChargeTimes();
	Check(level.medicChargeTime[0] == 40000 && level.engineerChargeTime[0] == 30000, "charge times return to normal");

	// Sudden Death blocks reinforcements inside the final two minutes of a timelimit
	g_gamestate.integer = GS_PLAYING;
	g_timelimit.value = 10;
	level.startTime = 0;
	level.timeCurrent = 7 * 60000;
	level.etlabModifier = ETLAB_MOD_SUDDENDEATH;
	Check(!G_ETLabRespawnBlocked(), "Sudden Death allows reinforcements before the window");
	level.timeCurrent = 8 * 60000 + 1;
	Check(G_ETLabRespawnBlocked(), "Sudden Death blocks reinforcements in the final two minutes");
	g_timelimit.value = 0;
	Check(!G_ETLabRespawnBlocked(), "Sudden Death needs a timelimit");
	g_timelimit.value = 10;
	level.etlabModifier = ETLAB_MOD_NONE;
	Check(!G_ETLabRespawnBlocked(), "no modifier never blocks reinforcements");

	// Loadouts: Sidearms Only strips primaries and grenades, keeps class tools; Low Ammo halves reserves
	for (i = TEAM_AXIS; i <= TEAM_ALLIES; i++)
	{
		weapon_t smg = i == TEAM_AXIS ? WP_MP40 : WP_THOMPSON;
		int w;
		SpawnWith(&plain, i, PC_SOLDIER, smg, ETLAB_MOD_NONE);
		SpawnWith(&client, i, PC_SOLDIER, smg, ETLAB_MOD_SIDEARMS);
		Check(!COM_BitCheck(client.ps.weapons, smg) && COM_BitCheck(plain.ps.weapons, smg), "Sidearms Only removes the primary weapon");
		Check(!COM_BitCheck(client.ps.weapons, WP_GRENADE_LAUNCHER) && !COM_BitCheck(client.ps.weapons, WP_GRENADE_PINEAPPLE), "Sidearms Only removes hand grenades");
		Check(COM_BitCheck(client.ps.weapons, BG_GetPlayerClassInfo(i, PC_SOLDIER)->classKnifeWeapon.weapon) && COM_BitCheck(client.ps.weapons, WP_PLIERS), "Sidearms Only keeps knife and pliers");
		Check(COM_BitCheck(client.ps.weapons, client.ps.weapon) && (GetWeaponTableData(client.ps.weapon)->type & WEAPON_TYPE_PISTOL), "Sidearms Only selects an owned pistol");
		for (w = WP_NONE + 1; w < WP_NUM_WEAPONS; w++)
		{
			Check(!(COM_BitCheck(client.ps.weapons, w) && (GetWeaponTableData(w)->type & (WEAPON_TYPE_SMG | WEAPON_TYPE_RIFLE | WEAPON_TYPE_MG | WEAPON_TYPE_PANZER | WEAPON_TYPE_MORTAR | WEAPON_TYPE_BEAM))), "Sidearms Only leaves no heavy or long weapon");
		}
		SpawnWith(&client, i, PC_SOLDIER, i == TEAM_AXIS ? WP_PANZERFAUST : WP_BAZOOKA, ETLAB_MOD_SIDEARMS);
		Check(!COM_BitCheck(client.ps.weapons, WP_PANZERFAUST) && !COM_BitCheck(client.ps.weapons, WP_BAZOOKA) && COM_BitCheck(client.ps.weapons, i == TEAM_AXIS ? WP_AKIMBO_LUGER : WP_AKIMBO_COLT), "Sidearms Only keeps the Akimbo bonus and drops the launcher");
		SpawnWith(&client, i, PC_ENGINEER, smg, ETLAB_MOD_SIDEARMS);
		Check(COM_BitCheck(client.ps.weapons, WP_DYNAMITE) && COM_BitCheck(client.ps.weapons, WP_LANDMINE) && !COM_BitCheck(client.ps.weapons, smg), "Sidearms Only keeps Engineer objectives tools");
		SpawnWith(&client, i, PC_MEDIC, smg, ETLAB_MOD_SIDEARMS);
		Check(COM_BitCheck(client.ps.weapons, WP_MEDIC_SYRINGE) && COM_BitCheck(client.ps.weapons, WP_MEDKIT), "Sidearms Only keeps Medic tools");
		SpawnWith(&client, i, PC_SOLDIER, smg, ETLAB_MOD_LOWAMMO);
		Check(client.ps.ammo[GetWeaponTableData(smg)->ammoIndex] == plain.ps.ammo[GetWeaponTableData(smg)->ammoIndex] / 2 && plain.ps.ammo[GetWeaponTableData(smg)->ammoIndex] > 0, "Low Ammo halves the primary reserve");
		Check(client.ps.ammoclip[GetWeaponTableData(smg)->clipIndex] == plain.ps.ammoclip[GetWeaponTableData(smg)->clipIndex], "Low Ammo keeps the loaded clip");
		Check(client.ps.ammo[WP_GRENADE_LAUNCHER] + client.ps.ammo[WP_GRENADE_PINEAPPLE] == plain.ps.ammo[WP_GRENADE_LAUNCHER] + plain.ps.ammo[WP_GRENADE_PINEAPPLE], "Low Ammo keeps hand grenades");
		Check(COM_BitCheck(client.ps.weapons, smg), "Low Ammo keeps the loadout itself");
	}

	// Class Rotation switches a repeated class to the next one the team limits allow and tracks the spawn class
	Com_Memset(&client, 0, sizeof(client));
	player->client = &client;
	team_maxSoldiers.integer = team_maxMedics.integer = team_maxEngineers.integer = team_maxFieldops.integer = team_maxCovertops.integer = -1;
	client.sess.sessionTeam = TEAM_ALLIES;
	client.sess.playerType = client.sess.latchPlayerType = PC_SOLDIER;
	level.etlabModifier = ETLAB_MOD_NONE;
	Check(!G_ETLabRotateClass(player) && client.pers.etlabLastSpawnClass == PC_SOLDIER + 1, "no rotation without the modifier, spawn class remembered");
	level.etlabModifier = ETLAB_MOD_CLASSROTATION;
	Check(G_ETLabRotateClass(player) && client.sess.playerType == PC_MEDIC && client.sess.latchPlayerType == PC_MEDIC, "repeated class rotates to the next class");
	Check(client.sess.playerWeapon == BG_GetPlayerClassInfo(TEAM_ALLIES, PC_MEDIC)->classPrimaryWeapons[0].weapon && client.sess.latchPlayerWeapon == client.sess.playerWeapon, "rotation assigns the class default primary");
	client.sess.playerType = client.sess.latchPlayerType = PC_COVERTOPS;
	Check(!G_ETLabRotateClass(player) && client.sess.playerType == PC_COVERTOPS, "a freely chosen new class is kept");
	Check(G_ETLabRotateClass(player) && client.sess.playerType == PC_SOLDIER, "rotation wraps around after Covert Ops");
	Q_strncpyz(team_maxMedics.string, "0", sizeof(team_maxMedics.string));
	team_maxMedics.integer = 0;
	Check(G_ETLabRotateClass(player) && client.sess.playerType == PC_ENGINEER, "rotation skips a class closed by the team limit");
	team_maxSoldiers.integer = team_maxFieldops.integer = team_maxCovertops.integer = 0;
	Check(!G_ETLabRotateClass(player) && client.sess.playerType == PC_ENGINEER, "rotation keeps the class when every other class is full");
	team_maxSoldiers.integer = team_maxMedics.integer = team_maxEngineers.integer = team_maxFieldops.integer = team_maxCovertops.integer = 0;
	team_maxMedics.string[0] = '\0';
	player->client = NULL;

	// Vampire heals the enemy killer up to max health only
	ResetScene();
	Com_Memset(&client, 0, sizeof(client));
	Com_Memset(&plain, 0, sizeof(plain));
	player->client = &client; other->client = &plain;
	client.sess.sessionTeam = TEAM_AXIS; plain.sess.sessionTeam = TEAM_ALLIES;
	client.ps.stats[STAT_MAX_HEALTH] = 100;
	player->health = 50;
	level.etlabModifier = ETLAB_MOD_NONE;
	G_ETLabModifierOnKill(other, player);
	Check(player->health == 50, "no healing without Vampire");
	level.etlabModifier = ETLAB_MOD_VAMPIRE;
	G_ETLabModifierOnKill(other, player);
	Check(player->health == 70 && client.ps.stats[STAT_HEALTH] == 70, "Vampire restores 20 health");
	player->health = 95;
	G_ETLabModifierOnKill(other, player);
	Check(player->health == 100, "Vampire caps at max health");
	plain.sess.sessionTeam = TEAM_AXIS;
	player->health = 50;
	G_ETLabModifierOnKill(other, player);
	Check(player->health == 50, "Vampire ignores team kills");
	G_ETLabModifierOnKill(player, player);
	Check(player->health == 50, "Vampire ignores suicides");
	player->client = other->client = NULL;

	// Timed modifiers: Grenade Rain refills, starts with the match and ends after the configured minutes
	ResetScene();
	Com_Memset(&client, 0, sizeof(client));
	player->client = &client; player->inuse = qtrue; player->health = 100;
	client.sess.sessionTeam = TEAM_AXIS; client.sess.playerType = PC_SOLDIER; client.sess.playerWeapon = WP_MP40;
	SetWolfSpawnWeapons(&client);
	client.ps.ammo[WP_GRENADE_LAUNCHER] = 0; client.ps.ammoclip[WP_GRENADE_LAUNCHER] = 0;
	level.numConnectedClients = 1; level.sortedClients[0] = 0;
	level.etlabModifier = ETLAB_MOD_GRENADERAIN;
	g_gamestate.integer = GS_WARMUP;
	G_ETLabModifierFrame();
	Check(level.etlabModifierEnd == 0 && client.ps.ammo[WP_GRENADE_LAUNCHER] == 0, "timed modifier waits for the match start");
	g_gamestate.integer = GS_PLAYING;
	G_ETLabModifierFrame();
	Check(level.etlabModifierEnd == level.time + 5 * 60000, "timed modifier runs five minutes by default");
	Check(client.ps.ammo[WP_GRENADE_LAUNCHER] == BG_MaxAmmoForWeapon(WP_GRENADE_LAUNCHER, client.sess.skill, PC_SOLDIER), "Grenade Rain refills to the class maximum");
	client.ps.ammo[WP_GRENADE_LAUNCHER] = 0;
	level.time += 10000;
	G_ETLabModifierFrame();
	Check(client.ps.ammo[WP_GRENADE_LAUNCHER] == 0, "Grenade Rain refills every 20 seconds, not every frame");
	level.time += 10000;
	G_ETLabModifierFrame();
	Check(client.ps.ammo[WP_GRENADE_LAUNCHER] > 0, "Grenade Rain second refill");
	level.time = level.etlabModifierEnd;
	G_ETLabModifierFrame();
	Check(level.etlabModifier == ETLAB_MOD_NONE && level.etlabModifierEnd == 0, "timed modifier ends and regular rules return");
	player->client = NULL;
	level.numConnectedClients = 0;
	g_medicChargeTime.integer = 0;
	g_gamestate.integer = GS_WARMUP;
	level.etlabModifier = ETLAB_MOD_NONE;
}

static void CheckQuickSpotting(void)
{
	gclient_t client;
	gentity_t *player = &g_entities[0], *mine = &g_entities[MAX_CLIENTS];
	int i, modifier;

	for (modifier = 0; modifier < 2; modifier++)
	{
		ResetScene();
		Com_Memset(&client, 0, sizeof(client));
		player->client = &client;
		player->inuse = qtrue;
		player->health = 100;
		client.sess.sessionTeam = TEAM_AXIS;
		client.sess.playerType = PC_SOLDIER;
		client.ps.stats[STAT_KEYS] = 1 << INV_BINOCS;
		client.ps.eFlags |= EF_ZOOMING;
		level.numConnectedClients = 1;
		level.etlabModifier = modifier ? ETLAB_MOD_QUICKSPOT : ETLAB_MOD_NONE;
		mine->s.number = MAX_CLIENTS;
		mine->inuse = qtrue;
		mine->s.eType = ET_MISSILE;
		mine->methodOfDeath = MOD_LANDMINE;
		mine->s.weapon = WP_LANDMINE;
		mine->s.effect1Time = 1;
		mine->s.teamNum = TEAM_ALLIES;
		VectorSet(mine->r.currentOrigin, 500, 0, 0);
		for (i = 0; i < 3; i++)
		{
			level.time += 500;
			G_CheckSpottedLandMines();
		}
		Check((mine->s.modelindex2 != 0) == (modifier != 0), "Quick Spotting halves the binocular dwell, normal dwell unchanged");
		player->client = NULL;
		level.etlabModifier = ETLAB_MOD_NONE;
	}
}

static int EndgameStatValues(int *values, int max)
{
	char *data = endgameStats, *token;
	int n = 0;
	while (n < max && *(token = COM_Parse(&data)))
	{
		values[n++] = Q_atoi(token);
	}
	return n;
}

static void CheckAwardTriplet(const int *values, int original, int award, int clientNum, int value, int team, const char *message)
{
	const int *v = values + (original + award) * 3;
	Check(v[0] == clientNum && v[1] == value && v[2] == team, message);
}

static void SetupMine(gentity_t *mine, int team, qboolean spotted)
{
	Com_Memset(mine, 0, sizeof(*mine));
	mine->inuse = qtrue;
	mine->s.number = MAX_CLIENTS;
	mine->s.eType = ET_MISSILE;
	mine->s.weapon = WP_LANDMINE;
	mine->methodOfDeath = MOD_LANDMINE;
	mine->s.effect1Time = 1;
	mine->s.teamNum = team;
	mine->s.modelindex2 = spotted;
	mine->takedamage = qtrue;
	mine->classname = "landmine";
}

/* Every award text needs a German msgstr in the mod catalog that ships in the PK3. */
static void CheckGermanCatalog(void)
{
	FILE *file = fopen(ETLAB_LOCALE_DE, "rb");
	char *data, *in, *out, *entry;
	long size;
	int i;

	Check(file != NULL, "German mod catalog readable");
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	data = calloc(size + 1, 1);
	Check(data && fread(data, 1, size, file) == (size_t)size, "German mod catalog loaded");
	fclose(file);
	for (in = out = data; *in; in++)
	{
		if (*in != '\r')
		{
			*out++ = *in;
		}
	}
	*out = '\0';
	for (i = -1; i < ETLAB_AWARD_COUNT * 2; i++)
	{
		const char *msgid = i < 0 ? "*** Christmas Awards ***" : (i % 2 ? etlabAwards[i / 2].description : etlabAwards[i / 2].title);
		const char *key = va("\nmsgid \"%s\"\nmsgstr \"", msgid);
		entry = strstr(data, key);
		Check(entry && entry[strlen(key)] != '"', "German translation for every award title, description and the chat header");
	}
	free(data);
}

static void CheckAwards(void)
{
	gclient_t *axis = &g_clients[0], *allies = &g_clients[1], *spectator = &g_clients[2];
	gentity_t *player = &g_entities[0], *target = &g_entities[MAX_CLIENTS];
	int values[256], original, i;
#ifdef FEATURE_RATING
	original = 22;
#else
	original = 21;
#endif

	// Shared table and session keys
	Check(ETLAB_AWARD_COUNT == 10 && ETLAB_STAT_COUNT == 7, "ten Christmas awards, seven own counters");
	for (i = 0; i < ETLAB_AWARD_COUNT; i++)
	{
		Check(etlabAwards[i].title[0] && etlabAwards[i].description[0] && !strchr(etlabAwards[i].title, '"') && !strchr(etlabAwards[i].description, '"'), "every award has a chat-safe title and description");
	}
	for (i = 0; i < ETLAB_STAT_COUNT; i++)
	{
		Check(!Q_strncmp(etlabStatNames[i], "etlab_", 6) && (!i || Q_stricmp(etlabStatNames[i], etlabStatNames[i - 1])), "session keys are prefixed and distinct");
	}

	ResetScene();
	Com_Memset(g_clients, 0, sizeof(gclient_t) * 3);
	for (i = 0; i < 3; i++)
	{
		g_entities[i].client = &g_clients[i];
		g_entities[i].inuse = qtrue;
		g_entities[i].s.number = i;
		g_clients[i].pers.enterTime = 0;
		Q_strncpyz(g_clients[i].pers.netname, va("Player%i", i), sizeof(g_clients[i].pers.netname));
		level.sortedClients[i] = i;
	}
	level.numConnectedClients = 3;
	axis->sess.sessionTeam = TEAM_AXIS;
	allies->sess.sessionTeam = TEAM_ALLIES;
	spectator->sess.sessionTeam = TEAM_SPECTATOR;

	// Counting window: like the original stats only while the match runs
	G_ETLabAwardCount(player, ETLAB_STAT_REVIVES);
	Check(axis->sess.etlabStats[ETLAB_STAT_REVIVES] == 0, "no award counting during warmup");
	g_gamestate.integer = GS_PLAYING;
	G_ETLabAwardCount(player, ETLAB_STAT_REVIVES);
	G_ETLabAwardCount(NULL, ETLAB_STAT_REVIVES);
	G_ETLabAwardCount(target, ETLAB_STAT_REVIVES);
	Check(axis->sess.etlabStats[ETLAB_STAT_REVIVES] == 1, "award events count while playing, entities without client ignored");

	// New Year's Firecracker: real damage path, hidden enemy mines and hand grenades only
	SetupMine(target, TEAM_ALLIES, qfalse);
	G_Damage(target, NULL, player, NULL, NULL, 30, 0, MOD_GRENADE_LAUNCHER);
	Check(target->freeAfterEvent && axis->sess.etlabStats[ETLAB_STAT_GRENADE_MINES] == 1, "hidden enemy mine blown up by grenade counts");
	SetupMine(target, TEAM_AXIS, qfalse);
	G_Damage(target, NULL, player, NULL, NULL, 30, 0, MOD_GRENADE_LAUNCHER);
	Check(target->freeAfterEvent && axis->sess.etlabStats[ETLAB_STAT_GRENADE_MINES] == 1, "own hidden mine explodes but does not count");
	SetupMine(target, TEAM_ALLIES, qtrue);
	G_Damage(target, NULL, player, NULL, NULL, 30, 0, MOD_GRENADE_LAUNCHER);
	Check(target->freeAfterEvent && axis->sess.etlabStats[ETLAB_STAT_GRENADE_MINES] == 1, "spotted mine is no hidden-mine award");
	SetupMine(target, TEAM_ALLIES, qfalse);
	G_Damage(target, NULL, player, NULL, NULL, 30, 0, MOD_LUGER);
	Check(!target->freeAfterEvent && axis->sess.etlabStats[ETLAB_STAT_GRENADE_MINES] == 1, "bullets neither detonate nor count hidden mines");

	// Santa's Little Helper: real pliers dispatch, enemy mines and dynamite for every class
	axis->sess.playerType = PC_SOLDIER;
	axis->ps.stats[STAT_PLAYER_CLASS] = PC_SOLDIER;
	axis->ps.weapon = WP_PLIERS;
	level.engineerChargeTime[0] = 30000;
	traceTarget = MAX_CLIENTS;
	SetupMine(target, TEAM_ALLIES, qfalse);
	target->health = 248;
	Weapon_Engineer(player);
	Check(!target->inuse && axis->sess.etlabStats[ETLAB_STAT_DEFUSES] == 1, "enemy landmine defused by Soldier counts");
	SetupMine(target, TEAM_AXIS, qfalse);
	target->health = 248;
	Weapon_Engineer(player);
	Check(!target->inuse && axis->sess.etlabStats[ETLAB_STAT_DEFUSES] == 1, "own landmine defuse does not count");
	for (i = 0; i < 2; i++)
	{
		Com_Memset(target, 0, sizeof(*target));
		target->inuse = qtrue;
		target->s.number = MAX_CLIENTS;
		target->s.eType = ET_MISSILE;
		target->s.weapon = WP_DYNAMITE;
		target->methodOfDeath = MOD_DYNAMITE;
		target->s.effect1Time = 1;
		target->s.teamNum = i ? TEAM_AXIS : TEAM_ALLIES;
		target->health = 246;
		target->classname = "dynamite";
		Weapon_Engineer(player);
		Check(target->health == 255 && axis->sess.etlabStats[ETLAB_STAT_DEFUSES] == 2, "enemy dynamite defuse counts, own dynamite does not");
	}

	// Workshop Elf: completed MG 42 repair by the Engineer
	axis->sess.playerType = PC_ENGINEER;
	axis->ps.stats[STAT_PLAYER_CLASS] = PC_ENGINEER;
	Com_Memset(target, 0, sizeof(*target));
	target->inuse = qtrue;
	target->s.number = MAX_CLIENTS;
	target->s.eType = ET_GENERAL;
	target->s.frame = 1;
	target->classname = "misc_mg42";
	target->health = 252;
	Weapon_Engineer(player);
	Check(target->s.frame == 0 && axis->sess.etlabStats[ETLAB_STAT_CONSTRUCTIONS] == 1, "completed MG 42 repair counts");
	target->health = 100;
	target->s.frame = 1;
	axis->ps.classWeaponTime = 0;
	Weapon_Engineer(player);
	Check(axis->sess.etlabStats[ETLAB_STAT_CONSTRUCTIONS] == 1, "unfinished repair does not count");
	traceTarget = ENTITYNUM_NONE;

	// Star of Bethlehem: real binocular spotting
	axis->ps.stats[STAT_KEYS] = 1 << INV_BINOCS;
	axis->ps.eFlags |= EF_ZOOMING;
	player->health = 100;
	level.numConnectedClients = 1;
	SetupMine(target, TEAM_ALLIES, qfalse);
	VectorSet(target->r.currentOrigin, 500, 0, 0);
	for (i = 0; i < 5; i++)
	{
		level.time += 500;
		G_CheckSpottedLandMines();
	}
	Check(target->s.modelindex2 && axis->sess.etlabStats[ETLAB_STAT_MINES_SPOTTED] == 1, "spotted enemy landmine counts");
	axis->ps.eFlags &= ~EF_ZOOMING;

	// Grinch: the shared destruction XP path; plain breakables without destruction XP do not count
	{
		g_constructible_stats_t structure;
		Com_Memset(&structure, 0, sizeof(structure));
		structure.destructxpbonus = 5.f;
		G_AddKillSkillPointsForDestruction(player, MOD_DYNAMITE, &structure);
		structure.destructxpbonus = 0.f;
		G_AddKillSkillPointsForDestruction(player, MOD_GRENADE_LAUNCHER, &structure);
		Check(axis->sess.etlabStats[ETLAB_STAT_DESTRUCTIONS] == 1, "destroyed objective counts, plain breakable does not");
	}

	// XP paths rerun CalculateRanks; restore the ranked scene for the award selection
	level.numConnectedClients = 3;
	for (i = 0; i < 3; i++)
	{
		level.sortedClients[i] = i;
	}

	// Stats reset with the original session stats
	for (i = 0; i < 3; i++)
	{
		G_deleteStats(i);
		Check(!g_clients[i].sess.etlabStats[ETLAB_STAT_REVIVES] && !g_clients[i].sess.etlabStats[ETLAB_STAT_DEFUSES] && !g_clients[i].sess.etlabStats[ETLAB_STAT_CONSTRUCTIONS], "stats reset clears award counters");
	}

	// Winners: Axis/Allies only, ties to the higher-ranked player, Krampus tie-break by team damage
	axis->sess.etlabStats[ETLAB_STAT_REVIVES] = 3;
	axis->sess.etlabStats[ETLAB_STAT_MINES_SPOTTED] = 1;
	axis->sess.team_kills = 2;
	axis->sess.team_damage_given = 100;
	axis->sess.aWeaponStats[WS_LUGER].headshots = 2;
	axis->sess.aWeaponStats[WS_MP40].headshots = 1;
	axis->sess.aWeaponStats[WS_KNIFE].kills = 1;
	axis->sess.etlabStats[ETLAB_STAT_DESTRUCTIONS] = 2;
	allies->sess.etlabStats[ETLAB_STAT_REVIVES] = 3;
	allies->sess.etlabStats[ETLAB_STAT_DEFUSES] = 2;
	allies->sess.etlabStats[ETLAB_STAT_GRENADE_MINES] = 1;
	allies->sess.etlabStats[ETLAB_STAT_PACKS] = 4;
	allies->sess.team_kills = 2;
	allies->sess.team_damage_given = 200;
	allies->sess.aWeaponStats[WS_KNIFE_KBAR].kills = 1;
	spectator->sess.etlabStats[ETLAB_STAT_REVIVES] = 10;
	spectator->sess.etlabStats[ETLAB_STAT_CONSTRUCTIONS] = 5;
	Check(G_ETLabAwardValue(axis, ETLAB_AWARD_NUTCRACKER) == 3 && G_ETLabAwardValue(allies, ETLAB_AWARD_SILENTNIGHT) == 1, "headshots and knife kills come from the weapon stats");
	level.time = 600000;
	level.intermissiontime = 0;

	g_etlabAwards.integer = 0;
	chatLines = awardCommands = 0;
	G_BuildEndgameStats();
	Check(EndgameStatValues(values, ARRAY_LEN(values)) == original * 3 && !awardCommands, "awards off: original endgame configstring, no announcement");

	g_etlabAwards.integer = 1;
	G_BuildEndgameStats();
	Check(EndgameStatValues(values, ARRAY_LEN(values)) == (original + ETLAB_AWARD_COUNT) * 3, "awards on: one triplet per award appended");
	CheckAwardTriplet(values, original, ETLAB_AWARD_ANGEL, 0, 3, TEAM_AXIS, "Christmas Angel: tie goes to the higher-ranked player, spectator excluded");
	CheckAwardTriplet(values, original, ETLAB_AWARD_HELPER, 1, 2, TEAM_ALLIES, "Santa's Little Helper");
	CheckAwardTriplet(values, original, ETLAB_AWARD_STAR, 0, 1, TEAM_AXIS, "Star of Bethlehem");
	CheckAwardTriplet(values, original, ETLAB_AWARD_KRAMPUS, 1, 2, TEAM_ALLIES, "Krampus: tie broken by team damage");
	CheckAwardTriplet(values, original, ETLAB_AWARD_GRINCH, 0, 2, TEAM_AXIS, "Grinch");
	CheckAwardTriplet(values, original, ETLAB_AWARD_FIRECRACKER, 1, 1, TEAM_ALLIES, "New Year's Firecracker");
	CheckAwardTriplet(values, original, ETLAB_AWARD_NUTCRACKER, 0, 3, TEAM_AXIS, "Nutcracker sums headshots of all weapons");
	CheckAwardTriplet(values, original, ETLAB_AWARD_SILENTNIGHT, 0, 1, TEAM_AXIS, "Silent Night counts both knives");
	CheckAwardTriplet(values, original, ETLAB_AWARD_WORKSHOPELF, -1, 0, TEAM_FREE, "Workshop Elf: nobody qualifies");
	CheckAwardTriplet(values, original, ETLAB_AWARD_SECRETSANTA, 1, 4, TEAM_ALLIES, "Secret Santa");
	Check(awardCommands == 10 && !chatLines && !Q_stricmp(lastAward, va("etlabaward %i 1 4 \"Player1\"", ETLAB_AWARD_SECRETSANTA)), "announcement: header and one client-localized line per given award");

	for (i = 0; i < 3; i++)
	{
		G_deleteStats(i);
		g_entities[i].client = NULL;
	}
	level.numConnectedClients = 0;
	g_etlabAwards.integer = 0;
	g_gamestate.integer = GS_WARMUP;
}

int main(void)
{
	int team, cls, i, xp;
	team_riflegrenades.integer = 1;
	dllEntry(TestSyscall);
	CheckMineDamage();
	CheckModifiers();
	CheckQuickSpotting();
	CheckGermanCatalog();
	CheckAwards();
	for (team = TEAM_AXIS; team <= TEAM_ALLIES; team++)
	{
		for (cls = PC_SOLDIER; cls < NUM_PLAYER_CLASSES; cls++)
		{
			bg_playerclass_t *info = BG_GetPlayerClassInfo(team, cls);
			CheckPliersActions(team, cls);
			CheckSpotting(team, cls);
			for (i = 0; i < MAX_WEAPS_PER_CLASS && info->classPrimaryWeapons[i].weapon; i++)
			{
				for (xp = 0; xp <= 4; xp += 4)
				{
					CheckSpawn(team, cls, info->classPrimaryWeapons[i].weapon, xp);
				}
			}
		}
	}
	printf("PASS: %d production spawn/menu/pliers/mines/modifier/award checks, both teams, all classes and primary loadouts, XP 0/4.\n", checks);
	return 0;
}
