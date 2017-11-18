// console.cpp: the console buffer, its display, and command line control

#include "engine.h"

#define MAXCONLINES 1000
struct cline { char *line; int type, outtime; };
reversequeue<cline, MAXCONLINES> conlines;

int commandmillis = -1;
cubestr commandbuf;
char *commandaction = NULL, *commandprompt = NULL;
enum { CF_COMPLETE = 1 << 0, CF_EXECUTE = 1 << 1 };
int commandflags = 0, commandpos = -1;

VARFP(maxcon, 10, 200, MAXCONLINES, { while (conlines.length() > maxcon) delete [] conlines.pop().line; });

#define CONSTRLEN 512

void conline(int type, const char *sf)        // add a line to the console buffer
{
	char *buf = conlines.length() >= maxcon ? conlines.remove().line : newstring("", CONSTRLEN - 1);
	cline &cl = conlines.add();
	cl.line = buf;
	cl.type = type;
	cl.outtime = totalmillis;                // for how long to keep line on screen
	copystring(cl.line, sf, CONSTRLEN);
}

void conoutfv(int type, const char *fmt, va_list args)
{
	static char buf[CONSTRLEN];
	vformatstring(buf, fmt, args, sizeof(buf));
	conline(type, buf);
	logoutf("%s", buf);
}

void conoutf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	conoutfv(CON_INFO, fmt, args);
	va_end(args);
}

void conoutf(int type, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	conoutfv(type, fmt, args);
	va_end(args);
}

ICOMMAND(fullconsole, "iN$", (int *val, int *numargs, ident *id),
{
	if (*numargs > 0) UI::holdui("fullconsole", *val != 0);
	else
	{
		int vis = UI::uivisible("fullconsole") ? 1 : 0;
		if (*numargs < 0) intret(vis);
		else printvar(id, vis);
	}
});
ICOMMAND(toggleconsole, "", (), UI::toggleui("fullconsole"));

float rendercommand(float x, float y, float w)
{
	if (commandmillis < 0) return 0;

	char buf[CONSTRLEN];
	const char *prompt = commandprompt ? commandprompt : ">";
	formatstring(buf, "%s %s", prompt, commandbuf);

	float width, height;
	text_boundsf(buf, width, height, w);
	y -= height;
	draw_text(buf, x, y, 0xFF, 0xFF, 0xFF, 0xFF, commandpos >= 0 ? commandpos + 1 + strlen(prompt) : strlen(buf), w);
	return height;
}

VARP(consize, 0, 5, 100);
VARP(miniconsize, 0, 5, 100);
VARP(miniconwidth, 0, 40, 100);
VARP(confade, 0, 30, 60);
VARP(miniconfade, 0, 30, 60);
VARP(fullconsize, 0, 75, 100);
HVARP(confilter, 0, 0xFFFFFF, 0xFFFFFF);
HVARP(fullconfilter, 0, 0xFFFFFF, 0xFFFFFF);
HVARP(miniconfilter, 0, 0, 0xFFFFFF);

int conskip = 0, miniconskip = 0;

void setconskip(int &skip, int filter, int n)
{
	int offset = abs(n), dir = n < 0 ? -1 : 1;
	skip = clamp(skip, 0, conlines.length() - 1);
	while (offset)
	{
		skip += dir;
		if (!conlines.inrange(skip))
		{
			skip = clamp(skip, 0, conlines.length() - 1);
			return;
		}
		if (conlines[skip].type&filter) --offset;
	}
}

ICOMMAND(conskip, "i", (int *n), setconskip(conskip, UI::uivisible("fullconsole") ? fullconfilter : confilter, *n));
ICOMMAND(miniconskip, "i", (int *n), setconskip(miniconskip, miniconfilter, *n));

ICOMMAND(clearconsole, "", (), { while (conlines.length()) delete [] conlines.pop().line; });

float drawconlines(int conskip, int confade, float conwidth, float conheight, float conoff, int filter, float y = 0, int dir = 1)
{
	int numl = conlines.length(), offset = min(conskip, numl);

	if (confade)
	{
		if (!conskip)
		{
			numl = 0;
			loopvrev(conlines) if (totalmillis - conlines[i].outtime < confade * 1000) { numl = i + 1; break; }
		}
		else offset--;
	}

	int totalheight = 0;
	loopi(numl) //determine visible height
	{
		// shuffle backwards to fill if necessary
		int idx = offset + i < numl ? offset + i : --offset;
		if (!(conlines[idx].type&filter)) continue;
		char *line = conlines[idx].line;
		float width, height;
		text_boundsf(line, width, height, conwidth);
		if (totalheight + height > conheight) { numl = i; if (offset == idx) ++offset; break; }
		totalheight += height;
	}
	if (dir > 0) y = conoff;
	loopi(numl)
	{
		int idx = offset + (dir > 0 ? numl - i - 1 : i);
		if (!(conlines[idx].type&filter)) continue;
		char *line = conlines[idx].line;
		float width, height;
		text_boundsf(line, width, height, conwidth);
		if (dir <= 0) y -= height;
		draw_text(line, conoff, y, 0xFF, 0xFF, 0xFF, 0xFF, -1, conwidth);
		if (dir > 0) y += height;
	}
	return y + conoff;
}

float renderfullconsole(float w, float h)
{
	float conpad = FONTH / 2,
		conheight = h - 2 * conpad,
		conwidth = w - 2 * conpad;
	drawconlines(conskip, 0, conwidth, conheight, conpad, fullconfilter);
	return conheight + 2 * conpad;
}

float renderconsole(float w, float h, float abovehud)
{
	float conpad = FONTH / 2,
		conheight = min(float(FONTH*consize), h - 2 * conpad),
		conwidth = w - 2 * conpad - game::clipconsole(w, h);
	float y = drawconlines(conskip, confade, conwidth, conheight, conpad, confilter);
	if (miniconsize && miniconwidth)
		drawconlines(miniconskip, miniconfade, (miniconwidth*(w - 2 * conpad)) / 100, min(float(FONTH*miniconsize), abovehud - y), conpad, miniconfilter, abovehud, -1);
	return y;
}

// keymap is defined externally in keymap.cfg

struct keym
{
	enum
	{
		ACTION_DEFAULT = 0,
		ACTION_SPECTATOR,
		ACTION_EDITING,
		NUMACTIONS
	};

	int code;
	char *name;
	char *actions[NUMACTIONS];
	bool pressed;

	keym() : code(-1), name(NULL), pressed(false) { loopi(NUMACTIONS) actions[i] = newstring(""); }
	~keym() { DELETEA(name); loopi(NUMACTIONS) DELETEA(actions[i]); }

	void clear(int type);
	void clear() { loopi(NUMACTIONS) clear(i); }
};

hashtable<int, keym> keyms(128);

void keymap(int *code, char *key)
{
	if (identflags&IDF_OVERRIDDEN) { conoutf(CON_ERROR, "cannot override keymap %d", *code); return; }
	keym &km = keyms[*code];
	km.code = *code;
	DELETEA(km.name);
	km.name = newstring(key);
}

COMMAND(keymap, "is");

keym *keypressed = NULL;
char *keyaction = NULL;

const char *getkeyname(int code)
{
	keym *km = keyms.access(code);
	return km ? km->name : NULL;
}

void searchbinds(char *action, int type)
{
	vector<char> names;
	enumerate(keyms, keym, km,
	{
		if (!strcmp(km.actions[type], action))
		{
			if (names.length()) names.add(' ');
			names.put(km.name, strlen(km.name));
		}
	});
	names.add('\0');
	result(names.getbuf());
}

keym *findbind(char *key)
{
	enumerate(keyms, keym, km,
	{
		if (!strcasecmp(km.name, key)) return &km;
	});
	return NULL;
}

void getbind(char *key, int type)
{
	keym *km = findbind(key);
	result(km ? km->actions[type] : "");
}

void bindkey(char *key, char *action, int state, const char *cmd)
{
	if (identflags&IDF_OVERRIDDEN) { conoutf(CON_ERROR, "cannot override %s \"%s\"", cmd, key); return; }
	keym *km = findbind(key);
	if (!km) { conoutf(CON_ERROR, "unknown key \"%s\"", key); return; }
	char *&binding = km->actions[state];
	if (!keypressed || keyaction != binding) delete [] binding;
	// trim white-space to make searchbinds more reliable
	while (iscubespace(*action)) action++;
	int len = strlen(action);
	while (len>0 && iscubespace(action[len - 1])) len--;
	binding = newstring(action, len);
}

ICOMMAND(bind, "ss", (char *key, char *action), bindkey(key, action, keym::ACTION_DEFAULT, "bind"));
ICOMMAND(specbind, "ss", (char *key, char *action), bindkey(key, action, keym::ACTION_SPECTATOR, "specbind"));
ICOMMAND(editbind, "ss", (char *key, char *action), bindkey(key, action, keym::ACTION_EDITING, "editbind"));
ICOMMAND(getbind, "s", (char *key), getbind(key, keym::ACTION_DEFAULT));
ICOMMAND(getspecbind, "s", (char *key), getbind(key, keym::ACTION_SPECTATOR));
ICOMMAND(geteditbind, "s", (char *key), getbind(key, keym::ACTION_EDITING));
ICOMMAND(searchbinds, "s", (char *action), searchbinds(action, keym::ACTION_DEFAULT));
ICOMMAND(searchspecbinds, "s", (char *action), searchbinds(action, keym::ACTION_SPECTATOR));
ICOMMAND(searcheditbinds, "s", (char *action), searchbinds(action, keym::ACTION_EDITING));

void keym::clear(int type)
{
	char *&binding = actions[type];
	if (binding[0])
	{
		if (!keypressed || keyaction != binding) delete [] binding;
		binding = newstring("");
	}
}

ICOMMAND(clearbinds, "", (), enumerate(keyms, keym, km, km.clear(keym::ACTION_DEFAULT)));
ICOMMAND(clearspecbinds, "", (), enumerate(keyms, keym, km, km.clear(keym::ACTION_SPECTATOR)));
ICOMMAND(cleareditbinds, "", (), enumerate(keyms, keym, km, km.clear(keym::ACTION_EDITING)));
ICOMMAND(clearallbinds, "", (), enumerate(keyms, keym, km, km.clear()));

void inputcommand(char *init, char *action = NULL, char *prompt = NULL, char *flags = NULL) // turns input to the command line on or off
{
	commandmillis = init ? totalmillis : -1;
	textinput(commandmillis >= 0, TI_CONSOLE);
	keyrepeat(commandmillis >= 0, KR_CONSOLE);
	copystring(commandbuf, init ? init : "");
	DELETEA(commandaction);
	DELETEA(commandprompt);
	commandpos = -1;
	if (action && action[0]) commandaction = newstring(action);
	if (prompt && prompt[0]) commandprompt = newstring(prompt);
	commandflags = 0;
	if (flags) while (*flags) switch (*flags++)
	{
	case 'c': commandflags |= CF_COMPLETE; break;
	case 'x': commandflags |= CF_EXECUTE; break;
	case 's': commandflags |= CF_COMPLETE | CF_EXECUTE; break;
	}
	else if (init) commandflags |= CF_COMPLETE | CF_EXECUTE;
}

ICOMMAND(saycommand, "C", (char *init), inputcommand(init));
COMMAND(inputcommand, "ssss");

void pasteconsole()
{
	if (!SDL_HasClipboardText()) return;
	char *cb = SDL_GetClipboardText();
	if (!cb) return;
	size_t cblen = strlen(cb),
		commandlen = strlen(commandbuf),
		decoded = decodeutf8((uchar *) &commandbuf[commandlen], sizeof(commandbuf) - 1 - commandlen, (const uchar *) cb, cblen);
	commandbuf[commandlen + decoded] = '\0';
	SDL_free(cb);
}

struct hline
{
	char *buf, *action, *prompt;
	int flags;

	hline() : buf(NULL), action(NULL), prompt(NULL), flags(0) {}
	~hline()
	{
		DELETEA(buf);
		DELETEA(action);
		DELETEA(prompt);
	}

	void restore()
	{
		copystring(commandbuf, buf);
		if (commandpos >= (int) strlen(commandbuf)) commandpos = -1;
		DELETEA(commandaction);
		DELETEA(commandprompt);
		if (action) commandaction = newstring(action);
		if (prompt) commandprompt = newstring(prompt);
		commandflags = flags;
	}

	bool shouldsave()
	{
		return strcmp(commandbuf, buf) ||
			(commandaction ? !action || strcmp(commandaction, action) : action != NULL) ||
			(commandprompt ? !prompt || strcmp(commandprompt, prompt) : prompt != NULL) ||
			commandflags != flags;
	}

	void save()
	{
		buf = newstring(commandbuf);
		if (commandaction) action = newstring(commandaction);
		if (commandprompt) prompt = newstring(commandprompt);
		flags = commandflags;
	}

	void run()
	{
		if (flags&CF_EXECUTE && buf[0] == '/') execute(buf + 1);
		else if (action)
		{
			alias("commandbuf", buf);
			execute(action);
		}
		else game::toserver(buf);
	}
};
vector<hline *> history;
int histpos = 0;

VARP(maxhistory, 0, 1000, 10000);

void history_(int *n)
{
	static bool inhistory = false;
	if (!inhistory && history.inrange(*n))
	{
		inhistory = true;
		history[history.length() - *n - 1]->run();
		inhistory = false;
	}
}

COMMANDN(history, history_, "i");

struct releaseaction
{
	keym *key;
	union
	{
		char *action;
		ident *id;
	};
	int numargs;
	tagval args[3];
};
vector<releaseaction> releaseactions;

const char *addreleaseaction(char *s)
{
	if (!keypressed) { delete [] s; return NULL; }
	releaseaction &ra = releaseactions.add();
	ra.key = keypressed;
	ra.action = s;
	ra.numargs = -1;
	return keypressed->name;
}

tagval *addreleaseaction(ident *id, int numargs)
{
	if (!keypressed || numargs > 3) return NULL;
	releaseaction &ra = releaseactions.add();
	ra.key = keypressed;
	ra.id = id;
	ra.numargs = numargs;
	return ra.args;
}

void onrelease(const char *s)
{
	addreleaseaction(newstring(s));
}

COMMAND(onrelease, "s");

void execbind(keym &k, bool isdown)
{
	loopv(releaseactions)
	{
		releaseaction &ra = releaseactions[i];
		if (ra.key == &k)
		{
			if (ra.numargs < 0)
			{
				if (!isdown) execute(ra.action);
				delete [] ra.action;
			}
			else execute(isdown ? NULL : ra.id, ra.args, ra.numargs);
			releaseactions.remove(i--);
		}
	}
	if (isdown)
	{
		int state = keym::ACTION_DEFAULT;
		if (!mainmenu)
		{
			if (editmode) state = keym::ACTION_EDITING;
			else if (player->state == CS_SPECTATOR) state = keym::ACTION_SPECTATOR;
		}
		char *&action = k.actions[state][0] ? k.actions[state] : k.actions[keym::ACTION_DEFAULT];
		keyaction = action;
		keypressed = &k;
		execute(keyaction);
		keypressed = NULL;
		if (keyaction != action) delete [] keyaction;
	}
	k.pressed = isdown;
}

bool consoleinput(const char *str, int len)
{
	if (commandmillis < 0) return false;

	resetcomplete();
	int cmdlen = (int) strlen(commandbuf), cmdspace = int(sizeof(commandbuf)) - (cmdlen + 1);
	len = min(len, cmdspace);
	if (commandpos<0)
	{
		memcpy(&commandbuf[cmdlen], str, len);
	}
	else
	{
		memmove(&commandbuf[commandpos + len], &commandbuf[commandpos], cmdlen - commandpos);
		memcpy(&commandbuf[commandpos], str, len);
		commandpos += len;
	}
	commandbuf[cmdlen + len] = '\0';

	return true;
}

bool consolekey(int code, bool isdown)
{
	if (commandmillis < 0) return false;

#ifdef __APPLE__
#define MOD_KEYS (KMOD_LGUI|KMOD_RGUI)
#else
#define MOD_KEYS (KMOD_LCTRL|KMOD_RCTRL)
#endif

	if (isdown)
	{
		switch (code)
		{
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			break;

		case SDLK_HOME:
			if (strlen(commandbuf)) commandpos = 0;
			break;

		case SDLK_END:
			commandpos = -1;
			break;

		case SDLK_DELETE:
		{
			int len = (int) strlen(commandbuf);
			if (commandpos<0) break;
			memmove(&commandbuf[commandpos], &commandbuf[commandpos + 1], len - commandpos);
			resetcomplete();
			if (commandpos >= len - 1) commandpos = -1;
			break;
		}

		case SDLK_BACKSPACE:
		{
			int len = (int) strlen(commandbuf), i = commandpos >= 0 ? commandpos : len;
			if (i<1) break;
			memmove(&commandbuf[i - 1], &commandbuf[i], len - i + 1);
			resetcomplete();
			if (commandpos>0) commandpos--;
			else if (!commandpos && len <= 1) commandpos = -1;
			break;
		}

		case SDLK_LEFT:
			if (commandpos>0) commandpos--;
			else if (commandpos<0) commandpos = (int) strlen(commandbuf) - 1;
			break;

		case SDLK_RIGHT:
			if (commandpos >= 0 && ++commandpos >= (int) strlen(commandbuf)) commandpos = -1;
			break;

		case SDLK_UP:
			if (histpos > history.length()) histpos = history.length();
			if (histpos > 0) history[--histpos]->restore();
			break;

		case SDLK_DOWN:
			if (histpos + 1 < history.length()) history[++histpos]->restore();
			break;

		case SDLK_TAB:
			if (commandflags&CF_COMPLETE)
			{
				complete(commandbuf, sizeof(commandbuf), commandflags&CF_EXECUTE ? "/" : NULL);
				if (commandpos >= 0 && commandpos >= (int) strlen(commandbuf)) commandpos = -1;
			}
			break;

		case SDLK_v:
			if (SDL_GetModState()&MOD_KEYS) pasteconsole();
			break;
		}
	}
	else
	{
		if (code == SDLK_RETURN || code == SDLK_KP_ENTER)
		{
			hline *h = NULL;
			if (commandbuf[0])
			{
				if (history.empty() || history.last()->shouldsave())
				{
					if (maxhistory && history.length() >= maxhistory)
					{
						loopi(history.length() - maxhistory + 1) delete history[i];
						history.remove(0, history.length() - maxhistory + 1);
					}
					history.add(h = new hline)->save();
				}
				else h = history.last();
			}
			histpos = history.length();
			inputcommand(NULL);
			if (h) h->run();
		}
		else if (code == SDLK_ESCAPE)
		{
			histpos = history.length();
			inputcommand(NULL);
		}
	}

	return true;
}

void processtextinput(const char *str, int len)
{
	if (!UI::textinput(str, len))
		consoleinput(str, len);
}

void processkey(int code, bool isdown)
{
	keym *haskey = keyms.access(code);
	if (haskey && haskey->pressed) execbind(*haskey, isdown); // allow pressed keys to release
	else if (!UI::keypress(code, isdown)) // UI key intercept
	{
		if (!consolekey(code, isdown))
		{
			if (haskey) execbind(*haskey, isdown);
		}
	}
}

void clear_console()
{
	keyms.clear();
}

void writebinds(stream *f)
{
	static const char * const cmds[3] = { "bind", "specbind", "editbind" };
	vector<keym *> binds;
	enumerate(keyms, keym, km, binds.add(&km));
	binds.sortname();
	loopj(3)
	{
		loopv(binds)
		{
			keym &km = *binds[i];
			if (*km.actions[j])
			{
				if (validateblock(km.actions[j])) f->printf("%s %s [%s]\n", cmds[j], escapestring(km.name), km.actions[j]);
				else f->printf("%s %s %s\n", cmds[j], escapestring(km.name), escapestring(km.actions[j]));
			}
		}
	}
}

// tab-completion of all idents and base maps

enum { FILES_DIR = 0, FILES_LIST };

struct fileskey
{
	int type;
	const char *dir, *ext;

	fileskey() {}
	fileskey(int type, const char *dir, const char *ext) : type(type), dir(dir), ext(ext) {}
};

struct filesval
{
	int type;
	char *dir, *ext;
	vector<char *> files;
	int millis;

	filesval(int type, const char *dir, const char *ext) : type(type), dir(newstring(dir)), ext(ext && ext[0] ? newstring(ext) : NULL), millis(-1) {}
	~filesval() { DELETEA(dir); DELETEA(ext); files.deletearrays(); }

	void update()
	{
		if (type != FILES_DIR || millis >= commandmillis) return;
		files.deletearrays();
		listfiles(dir, ext, files);
		files.sort();
		loopv(files) if (i && !strcmp(files[i], files[i - 1])) delete [] files.remove(i--);
		millis = totalmillis;
	}
};

static inline bool htcmp(const fileskey &x, const fileskey &y)
{
	return x.type == y.type && !strcmp(x.dir, y.dir) && (x.ext == y.ext || (x.ext && y.ext && !strcmp(x.ext, y.ext)));
}

static inline uint hthash(const fileskey &k)
{
	return hthash(k.dir);
}

static hashtable<fileskey, filesval *> completefiles;
static hashtable<char *, filesval *> completions;

int completesize = 0;
char *lastcomplete = NULL;

void resetcomplete() { completesize = 0; }

void addcomplete(char *command, int type, char *dir, char *ext)
{
	if (identflags&IDF_OVERRIDDEN)
	{
		conoutf(CON_ERROR, "cannot override complete %s", command);
		return;
	}
	if (!dir[0])
	{
		filesval **hasfiles = completions.access(command);
		if (hasfiles) *hasfiles = NULL;
		return;
	}
	if (type == FILES_DIR)
	{
		int dirlen = (int) strlen(dir);
		while (dirlen > 0 && (dir[dirlen - 1] == '/' || dir[dirlen - 1] == '\\'))
			dir[--dirlen] = '\0';
		if (ext)
		{
			if (strchr(ext, '*')) ext[0] = '\0';
			if (!ext[0]) ext = NULL;
		}
	}
	fileskey key(type, dir, ext);
	filesval **val = completefiles.access(key);
	if (!val)
	{
		filesval *f = new filesval(type, dir, ext);
		if (type == FILES_LIST) explodelist(dir, f->files);
		val = &completefiles[fileskey(type, f->dir, f->ext)];
		*val = f;
	}
	filesval **hasfiles = completions.access(command);
	if (hasfiles) *hasfiles = *val;
	else completions[newstring(command)] = *val;
}

void addfilecomplete(char *command, char *dir, char *ext)
{
	addcomplete(command, FILES_DIR, dir, ext);
}

void addlistcomplete(char *command, char *list)
{
	addcomplete(command, FILES_LIST, list, NULL);
}

COMMANDN(complete, addfilecomplete, "sss");
COMMANDN(listcomplete, addlistcomplete, "ss");

void complete(char *s, int maxlen, const char *cmdprefix)
{
	int cmdlen = 0;
	if (cmdprefix)
	{
		cmdlen = strlen(cmdprefix);
		if (strncmp(s, cmdprefix, cmdlen)) prependstring(s, cmdprefix, maxlen);
	}
	if (!s[cmdlen]) return;
	if (!completesize) { completesize = (int) strlen(&s[cmdlen]); DELETEA(lastcomplete); }

	filesval *f = NULL;
	if (completesize)
	{
		char *end = strchr(&s[cmdlen], ' ');
		if (end) f = completions.find(cubestrslice(&s[cmdlen], int(end - &s[cmdlen])), NULL);
	}

	const char *nextcomplete = NULL;
	if (f) // complete using filenames
	{
		int commandsize = strchr(&s[cmdlen], ' ') + 1 - s;
		f->update();
		loopv(f->files)
		{
			if (strncmp(f->files[i], &s[commandsize], completesize + cmdlen - commandsize) == 0 &&
				(!lastcomplete || strcmp(f->files[i], lastcomplete) > 0) && (!nextcomplete || strcmp(f->files[i], nextcomplete) < 0))
				nextcomplete = f->files[i];
		}
		cmdprefix = s;
		cmdlen = commandsize;
	}
	else // complete using command names
	{
		enumerate(idents, ident, id,
			if (strncmp(id.name, &s[cmdlen], completesize) == 0 &&
				(!lastcomplete || strcmp(id.name, lastcomplete) > 0) && (!nextcomplete || strcmp(id.name, nextcomplete) < 0))
				nextcomplete = id.name;
		);
	}
	DELETEA(lastcomplete);
	if (nextcomplete)
	{
		cmdlen = min(cmdlen, maxlen - 1);
		if (cmdlen) memmove(s, cmdprefix, cmdlen);
		copystring(&s[cmdlen], nextcomplete, maxlen - cmdlen);
		lastcomplete = newstring(nextcomplete);
	}
}

void writecompletions(stream *f)
{
	vector<char *> cmds;
	enumeratekt(completions, char *, k, filesval *, v, { if (v) cmds.add(k); });
	cmds.sort();
	loopv(cmds)
	{
		char *k = cmds[i];
		filesval *v = completions[k];
		if (v->type == FILES_LIST)
		{
			if (validateblock(v->dir)) f->printf("listcomplete %s [%s]\n", escapeid(k), v->dir);
			else f->printf("listcomplete %s %s\n", escapeid(k), escapestring(v->dir));
		}
		else f->printf("complete %s %s %s\n", escapeid(k), escapestring(v->dir), escapestring(v->ext ? v->ext : "*"));
	}
}

// bindmap is defined externally in bindmap.cfg
//bindmap, keep in mind the folloing terms, id is the actioncode that we are going to be using this is the index in vector<actionstate *> actions where 0 is global. Where as state is the state of this key (one of ACTION_STATES, key up down released pressed)
struct bindmap
{
	enum ACTION_STATES
	{
		ACTION_DOWN = 0, //while the key is pressed
		ACTION_RELEASE, //on release of key (fires once)
		ACTION_PRESS,	//on press of key (fires once;
		ACTION_UP,		//while the key isnt being pressed
		NUMACTION
	};
private:
	struct actionstate
	{
		char *code[NUMACTION]; //code that runs when an action is true

		actionstate() { loopi(NUMACTION)code[i] = newstring(""); } //fix bug and make sure all data is pointing to something if not already stated.
		actionstate(char *act[NUMACTION]) { loopi(NUMACTION) code[i] = newstring(act[i]); } //dont need to delete old ones cuz there is no info assigned yet
		void reassign(uchar id, char *act) //only assigns one action
		{
			if (id >= NUMACTION) return;
			DELETEA(code[id]);	code[id] = newstring(act); //rememeber to always delete old arrays before assigning new ones and assign using newstrings so we can point to new memory so not to get a seg fault
		}
		void reassign(char *act[NUMACTION]) //assign all actions states
		{
			loopi(NUMACTION)DELETEA(code[i]);  //delete old
			loopi(NUMACTION) code[i] = newstring(act[i]); //create new
		}
		void fire(uint action) { if (action < NUMACTION) compilecode(code[action]); }
		void clear() { loopi(NUMACTION) clear(i); }
		void clear(uchar state) { if (state >= NUMACTION) return; DELETEA(code[state]); code[state] = newstring("[ ]"); }
		~actionstate() { loopi(NUMACTION) DELETEA(code[i]); }
	};

public:
	bindmap() : charcode(-1), name(""), pressed(false), changed(false) { } //add one element to the actions for the global state;
	void initkeymap(char *name, int code) { this->name = name; this->charcode = code; } // starts up the key and sets up nessary info
	void setaction(int id, char * d_code, char *r_code, char *p_code, char *u_code) //find all actions for each of the ACTION_STATES
	{
		char *codes[NUMACTION] = { d_code, r_code, p_code,u_code };
		if (actions.length() > id) if (actions[id]) actions[id]->reassign(codes); else actions[id] = new actionstate(codes); // if we dont need to pad then we check to see if we have an assignable action state if so reassign else create new one with the data we have;
		else //we need to pad the array now
		{
			int len = actions.length(); //just in case we have wierd run time error when changing this value as we pad
			actions.pad(len - id); //pad one from where we want to be
			actions.add(new actionstate(codes));
		}
	}
	char *getaction(int id, uchar state) //gets code that is bound to an action
	{
		if (actions.length() > id && actions[id]) return actions[id]->code[state]; //if actionstate exeist return the actionstate of the state
		return newstring("");
	}

	char *getname() const { return newstring(name.c_str()); } //return name
	bool searchaction(int id, uchar state, char * action); //searches for a string that is bound to this key; (maybe unused);
	void clear() { actions.deletecontents(); }
	void clear(int id)  //removes a particluar action -- remember actions are order dependent so just empty them and add in black holders when they need cleared
	{
		if (actions.length() > id || !actions[id]) return;
		actions[id]->clear();
	}
	void clear(int id, uchar state)
	{
		if (id < 0 || state >= NUMACTION || !actions[id]) return;
		actions[id]->clear(state);
	}

	void update(bool press) //remember 0 is global and all other are user defined; this function will fire on all downs or ups but not while up (this is called from the man sdl push);
	{
		changed = !(pressed == press);
		pressed = press;
	}
	void fire(); //this is fired all in one loop, this just simply fires the appropriate call when asked;
	bool comparebindname(char *name)
	{
		conoutf("name: %s -> %s", name, this->name.c_str());
		return this->name == str(name);
	}
	~bindmap() { actions.deletecontents(); }
private:

	int charcode; //charactor code of elememt (so we know which id to tract in sdl)
	str name; //name of key set by user for referencing
	bool pressed; //is currently pressed;
	bool changed; //has changed from pressed to unpress or visa versa this frame
	vector<actionstate *> actions; //stores code in cubescript for binds of each actionstate;


};


//Profiles, for various users or styles, like joystick vs keyboard or some other styling. Each profile will use default as base when creating and then create varions. This will be writen back to a file
struct bindmapprofile
{
	static void init() //create default state and runs keymap.cfg or userdefined class;
	{
		newprofile("default", "", true);
		bindstates.add("gloabl");
		curentstate = 0;
	}
	bindmapprofile(const char *name) : name("") { this->name = str(name); }
	static void newprofile(str name, str copyprof = "c", bool setactive = true) // creates a new profile from a curent profile. or from default if not defined (default is default and will be saved to file);
	{
		bindmapprofile * bmp = getprofilebyname(name);
		if (bmp) { int id = profiles.find(bmp); if (id) delete profiles.remove(id); bmp = nullptr; } //removes any copy of an object that we have already;
		bmp = new bindmapprofile(name.c_str());
		profiles.add(bmp);
		if (setactive)curentprofile = bmp;

		if (copyprof == "")return; //if this is the first profile then we dont need to copy a profile
		else if (copyprof == "c") copyprof = curentprofile->getname(); //this allows the defult arg to just copy the curent profile so we can make a slight modification to the curent one, while still having the old one saved back. or so we can use one of these as a base to create new ones. 

																	   //else we need to make a copy of the curent porfile to stop from needing to redefine;
		bindmapprofile *cbmp = getprofilebyname(copyprof);
		if (!cbmp)return;
		////enumerate(cbmp->bindmaps, bindmap, bm,
		//for(std::pair<int, bindmap> element :cbmp->bindmaps)
		//{
		//	bindmap &bm = element.second;
		//	bmp->newbind(bm);
		//};
		return;
	}
	//keymap fucntion
	static void _newbindmap(int *code, char * name) { if (curentprofile)curentprofile->newbindmap(code, name); else conoutf(CON_ERROR, "no curent profile"); };
	void newbindmap(int *code, char * name) //adds new key to bindmap, this should be defined in keys.cfg and should hold all keys then profiles should change how these keys act. The keys are automimous so each profile can have various keys, but only if not defined in the default map
	{
		if (identflags&IDF_OVERRIDDEN) { conoutf(CON_ERROR, "cannot override keymap %d", *code); return; } //maybe remove ??
		bindmap bm = bindmaps[*code];
		//bm.charcode = *code; cant directly manipulate
		//DELETEA(bm.name); only use if using char *;
		//bm.name = name; cant directly manipulate
		bm.initkeymap(name, *code); //use this to init the keymap now;
		conoutf("%s %d inserted", name, *code);
	}
	static void processkey(int key, bool isdown) //wrapper that set the curent bindprofile and tells it to process the data
	{
		//	conoutf("key: %d state: %s", key, isdown ? "down" : "up");
		return;
		if (curentprofile)curentprofile->_processkey(key, isdown); else conoutf(CON_ERROR, "no curent profile");
	}
	void _processkey(int key, bool isdown) //finds then calls the appropriate bindmaps with the data it needs to know
	{
		bindmap &bm = bindmaps[key];
		bm.update(isdown);
	}
	static void firekeys()
	{
		if (curentprofile)curentprofile->_firekeys(); else conoutf(CON_ERROR, "no curent profile");
	}
	void _firekeys()
	{
		for (std::pair<int, bindmap> element : bindmaps)
			element.second.fire();
	}
	bindmap *findbindmap(char *name) //#expensive call -- simple command that takes the char name and finds the bindmap (this should only be used to init the keys and the key id should be used for all other mods because this is much more efficent
	{
		for (std::pair<int, bindmap> element : bindmaps)
		{
			bindmap &bm = element.second;
			if (bm.comparebindname(name)) return &bm;
		}
		return NULL;
	}

	char *getkeyname(int code) //This returns the name of the key this will also serve as a varification where a Null represents a non-defined key.
	{
		bindmap &bm = bindmaps[code];
		return bm.getname();
	}
	static int newbindstate(str name) //adds a new bindstate name, must add all names before using returns an id of the bind state so it can be easily used, if you over reach in newbind() you should make sure you have called this first
	{
		bindstates.add(name);
		return bindstates.length() - 1;
	}
	//same as newbind
	static void newbind(int *id, char *name, char * d_code, char *r_code, char *p_code, char *u_code)//Binds a new key in a given id (ids are the old actions and represent the different player states only change is that 0 is default and 1 is game etc) the *_code represents the cube script given for each action state. d= down r = release p = press u = up. See bindmap::actionstates::ACTION_STATES
	{
		//if we have a profile error just set it back to default and move on; if error prosist there is a larger problem so lets just forget it
		if (!curentprofile) { conoutf("error: bindmap profile error; does not exist: %s", curentprofile->getname().c_str()); curentprofile = getprofilebyname("default"); if (!curentprofile) return conoutf("Error: bindprofile \"default\" does not exist, this is an unfixable error, try reinstalling"); } //dont touch
		bindmap *b = curentprofile->findbindmap(name);
		if (!b) return conoutf("unknown key %s", name);
		if (bindstates.length() <= *id) return conoutf("unknow bind state %s: try defining newbindstate and using this id to create a new bind in this state. If you wish this to be a default bind use id 0.");
		//b->setaction(*id, d_code, r_code, p_code, u_code);
		conoutf("sucess");
		return;
	}
	//make me private
	void newbind(const bindmap &cbm) //creates a new bindmap for this profile from a bindmap of this profile or another profile -- used when copying a profile from one to another.
	{
		bindmap *bm = findbindmap(cbm.getname());
		if (!bm) return;
		*bm = cbm; //this should copy over the cbm (copy bindmap) to the bm. this may not work but i believe this is correct.
	}
	void findaction(char *code, int type = -1, char actionstate = -1) //finds an action used for bind menus. This will find if a bind contains an action, this can get convoluted for more complex binds, this should be used sparingly. Will look to improve this function with a posiblity to fix some problems this presents.
	{
		//cube script only
		vector<char> names;
		int st = type < 0 ? bindmapprofile::bindstates.length() : 1;
		int st2 = actionstate < 0 ? 4 : 1; //as long as there are only 4 action states
		int en = type < 0 ? 0 : type;
		int en2 = actionstate < 0 ? 0 : actionstate;
		//	enumerate(bindmaps, bindmap, bm, //loop bindmaps
		for (std::pair<int, bindmap > element : bindmaps)
		{
			bindmap &bm = element.second;
			//see loopmidi it starts at st and then move en times
			loopmidi(st, en) loopmidk(st2, en2) // loop from st to en for bindstate then st2 to en2 for actionstate as stated above
				if (bm.getaction(i, k))
				{
					if (names.length()) names.add(' ');
					char * a = bm.getname();
					names.put(a, strlen(a));
				}
		}
		names.add('\0');
		result(names.getbuf());
	}
	void getaction(char *key, int type, char actionstate = -1) //#expensive call due to calling findbindmap -- this will get the action of a key and return it back. This will either "[ d_act ] [ r_act ] [ p_act ] [ u_act ]" if actionstate = -1; else it will return just the actionstate requested. any number over 3 or under -1 will return "/0"
	{
		bindmap *bm = findbindmap(key);

		if (actionstate < 4) return result(bm ? bm->getaction(type, actionstate) : "");
		if (actionstate >= 4 || !bm) return result("");
		vector<char> names;
		loopi(4) //if < 0 then get all action stats so they look like [ $d_action ] [ $r_action ] [ $p_action ] [ $u_action ] this will allow you to use them like an array to find data easily
		{
			if (names.length()) names.add(' ');
			names.add('['); names.add(' ');
			bm->getaction(type, i);
			names.add(' ');	names.add(']');
		}
		names.add('\0');
		result(names.getbuf());
	}
	str getname() { return str(name.c_str()); }
	static int getbindstatefromname(char *name) //interfaces with like named vector for ease of use.
	{
		loopv(bindstates)if (str(name) == bindstates[i]) return i; return -1;
	}
	static str getbindstatefromid(uint id) { return bindstates.length() < id ? bindstates[id] : str(""); }  //interfaces with like named vector;
	static uchar getcurentstate() { return bindmapprofile::curentstate; }
	static  void setcurentstate(uchar i) { if (i < bindstates.length()) bindmapprofile::curentstate = i; }
	static bindmapprofile *getprofilebyname(str n) { loopv(profiles) if (profiles[i]->getname() == n) return profiles[i]; return NULL; }

private:
	str name; //name of the profile
	std::unordered_map<int, bindmap > bindmaps; //key map used for each profile includes sdl id and code to run on each actionstate
	static vector<bindmapprofile *> profiles;  //list of profiles and there names, keep all data loaded on game start, only load active one and any ones accessed during games life, only load the current one on restart.
	static bindmapprofile *curentprofile; //stores the current profile
	static uchar curentstate; //stores the curent action state we are in;
	static vector<str> bindstates; //makes different bind types for easy access so we know how to manipulate everything. This will more meta and for user ease of use.

};

bindmapprofile * bindmapprofile::curentprofile = NULL;
vector<bindmapprofile *> bindmapprofile::profiles;
vector<str> bindmapprofile::bindstates;
uchar bindmapprofile::curentstate = 0;


//wrapperfunction so we can call this in the main loop init
void initbindmap() { bindmapprofile::init(); }
void firekeymaps() { bindmapprofile::firekeys(); }
void processkeymap(int key, bool isdown) { bindmapprofile::processkey(key, isdown); }

void bindmap::fire() //uses above definition so we need to fix
{
	if (actions.length() == 0) return; //if we have no actions assigned then we dont need to do anything.
	uchar curact = bindmapprofile::getcurentstate();
	if (curact >= actions.length()) curact = 0; //check to see if we are padded that far if not then we run the gobal state
	else if (!actions[curact]) curact = 0; //check to see if there is code to run for this state. (if it was nv specified then run the global). (keep in mind if you create a bind that has code "" this will stop the global from running instead should use clear bind
	if (!actions[curact]) return; //if no code to run then dont try
	if (changed)actions[curact]->fire(ACTION_RELEASE + pressed); //use cheeky code to make this easy (this will handle release and press states.
	actions[curact]->fire(ACTION_DOWN + (pressed * 2)); //use cheeky code to hanle up and down cases;
	changed = false;
}

ICOMMAND(mapkey, "is", (int *id, char *name), bindmapprofile::_newbindmap(id, name););
ICOMMAND(mapbind, "isssss", (int *id, char *name, char * d_code, char *r_code, char *p_code, char *u_code), bindmapprofile::newbind(id, name, d_code, r_code, p_code, u_code););

