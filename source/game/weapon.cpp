// weapon.cpp: all shooting and effects code, projectile management
#include "game.h"

namespace game
{
    static const int OFFSETMILLIS = 500;
    vec rays[MAXRAYS];

    struct hitmsg
    {
        int target,
            lifesequence,
            info1,
            info2;
        //headshots;
        ivec dir;
    };
    vector<hitmsg> hits;

    #define MINDEBRIS 3
    VARP(maxdebris, MINDEBRIS, 10, 100);
    VARP(maxgibs, 0, 4, 100);

    ICOMMAND(getweapon, "", (), intret(player1->gunselect));

    void gunselect(int gun, gameent* d)
    {
        if (gun != d->gunselect)
        {
            addmsg(N_GUNSELECT, "rci", d, gun);
            playsound(S_WEAPLOAD, &d->o);
        }
        d->gunselect = gun;
    }

    void nextweapon(int dir, bool force = false)
    {
        if (player1->state != CS_ALIVE) return;
        dir = (dir < 0 ? NUMGUNS - 1 : 1);
        int gun = player1->gunselect;
        loopi(NUMGUNS)
        {
            gun = (gun + dir) % NUMGUNS;
            if (force || player1->hasammo(gun)) break;
        }
        if (gun != player1->gunselect) gunselect(gun, player1);
        else playsound(S_NOAMMO);
    }
    ICOMMAND(nextweapon, "ii", (int* dir, int* force), nextweapon(*dir, *force != 0));

    int getweapon(const char* name)
    {
        if (isdigit(name[0])) return parseint(name);
        else
        {
            int len = strlen(name);
            loopi(sizeof(guns) / sizeof(guns[0])) if (!strncasecmp(guns[i].name, name, len)) return i;
        }
        return -1;
    }

    void setweapon(const char* name, bool force = false)
    {
        int gun = getweapon(name);
        if (player1->state != CS_ALIVE || !validgun(gun)) return;
        /*if(force || player1->hasammo(gun))*/ { gunselect(gun, player1); return; }
        playsound(S_NOAMMO);
    }
    ICOMMAND(setweapon, "si", (char* name, int* force), setweapon(name, *force != 0));

    void cycleweapon(int numguns, int* guns, bool force = false)
    {
        if (numguns <= 0 || player1->state != CS_ALIVE) return;
        int offset = 0;
        loopi(numguns) if (guns[i] == player1->gunselect) { offset = i + 1; break; }
        loopi(numguns)
        {
            int gun = guns[(i + offset) % numguns];
            if (gun >= 0 && gun < NUMGUNS && (force || player1->hasammo(gun)))
            {
                gunselect(gun, player1);
                return;
            }
        }
        playsound(S_NOAMMO);
    }
    ICOMMAND(cycleweapon, "V", (tagval* args, int numargs),
        {
             int numguns = min(numargs, 3);
             int guns[3];
             loopi(numguns) guns[i] = getweapon(args[i].getstr());
             cycleweapon(numguns, guns);
        });

    void weaponswitch(gameent* d)
    {
        if (d->state != CS_ALIVE) return;
        int s = d->gunselect;
        if (s != GUN_SHOTGUN && d->hasammo(GUN_SHOTGUN))  s = GUN_SHOTGUN;
        else if (s != GUN_M4 && d->hasammo(GUN_M4))  s = GUN_M4;
        else if (s != GUN_PULSERIFLE && d->hasammo(GUN_PULSERIFLE))  s = GUN_PULSERIFLE;
        else if (s != GUN_357 && d->hasammo(GUN_357))  s = GUN_357;
        else if (s != GUN_PISTOL && d->hasammo(GUN_PISTOL))  s = GUN_PISTOL;
        gunselect(s, d);
    }

    ICOMMAND(weapon, "V", (tagval* args, int numargs),
        {
            if (player1->state != CS_ALIVE) return;
            loopi(3)
            {
                const char* name = i < numargs ? args[i].getstr() : "";
                if (name[0])
                {
                    int gun = getweapon(name);
                    if (validgun(gun) && gun != player1->gunselect && player1->hasammo(gun)) { gunselect(gun, player1); return; }
                }
     else { weaponswitch(player1); return; }
}
playsound(S_NOAMMO);
        });

    void offsetray(const vec& from, const vec& to, int spread, float range, vec& dest)
    {
        vec offset;
        do offset = vec(rndscale(1), rndscale(1), rndscale(1)).sub(0.5f);
        while (offset.squaredlen() > 0.5f * 0.5f);
        offset.mul((to.dist(from) / 1024) * spread);
        dest = vec(offset).add(to);
        if (dest != from)
        {
            vec dir = vec(dest).sub(from).normalize();
            raycubepos(from, dir, dest, range, RAY_CLIPMAT | RAY_ALPHAPOLY);
        }
    }

    void createrays(int atk, const vec& from, const vec& to)             // create random spread of rays
    {
        loopi(attacks[atk].rays) offsetray(from, to, attacks[atk].spread, attacks[atk].range, rays[i]);
    }

    enum { BNC_GIBS, BNC_DEBRIS };

    struct bouncer : physent
    {
        int lifetime, bounces;
        float lastyaw, roll;
        bool local;
        gameent* owner;
        int bouncetype, variant;
        vec offset;
        int offsetmillis;
        int id;

        bouncer() : bounces(0), roll(0), variant(0)
        {
            type = ENT_BOUNCE;
        }
    };

    vector<bouncer*> bouncers;

    void newbouncer(const vec& from, const vec& to, bool local, int id, gameent* owner, int type, int lifetime, int speed)
    {
        bouncer& bnc = *bouncers.add(new bouncer);
        bnc.o = from;
        bnc.radius = bnc.xradius = bnc.yradius = type == BNC_DEBRIS ? 0.5f : 1.5f;
        bnc.eyeheight = bnc.radius;
        bnc.aboveeye = bnc.radius;
        bnc.lifetime = lifetime;
        bnc.local = local;
        bnc.owner = owner;
        bnc.bouncetype = type;
        bnc.id = local ? lastmillis : id;

        switch (type)
        {
        case BNC_DEBRIS: bnc.variant = rnd(4); break;
        case BNC_GIBS: bnc.variant = rnd(1); break;
        }

        vec dir(to);
        dir.sub(from).safenormalize();
        bnc.vel = dir;
        bnc.vel.mul(speed);

        avoidcollision(&bnc, dir, owner, 0.1f);

        bnc.offset = from;
        bnc.offset.sub(bnc.o);
        bnc.offsetmillis = OFFSETMILLIS;

        bnc.resetinterp();
    }

    void bounced(physent* d, const vec& surface)
    {
        if (d->type != ENT_BOUNCE) return;
        bouncer* b = (bouncer*)d;
        if (b->bouncetype != BNC_GIBS || b->bounces >= 2) return;
        b->bounces++;
        addstain(STAIN_BLOOD, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 2.96f / b->bounces, bvec(0x60, 0xFF, 0xFF), rnd(4));
    }

    void updatebouncers(int time)
    {
        loopv(bouncers)
        {
            bouncer& bnc = *bouncers[i];
            vec old(bnc.o);
            bool stopped = false;
            // cheaper variable rate physics for debris, gibs, etc.
            for (int rtime = time; rtime > 0;)
            {
                int qtime = min(30, rtime);
                rtime -= qtime;
                if ((bnc.lifetime -= qtime) < 0 || bounce(&bnc, qtime / 1000.0f, 0.6f, 0.5f, 1)) { stopped = true; break; }
            }
            if (stopped)
            {
                delete bouncers.remove(i--);
            }
            else
            {
                bnc.roll += old.sub(bnc.o).magnitude() / (4 * RAD);
                bnc.offsetmillis = max(bnc.offsetmillis - time, 0);
            }
        }
    }

    void removebouncers(gameent* owner)
    {
        loopv(bouncers) if (bouncers[i]->owner == owner) { delete bouncers[i]; bouncers.remove(i--); }
    }

    void clearbouncers() { bouncers.deletecontents(); }

    struct projectile
    {
        vec dir, o, from, to, offset;
        float speed;
        gameent* owner;
        int atk;
        bool local;
        int offsetmillis;
        int id;
    };
    vector<projectile> projs;

    void clearprojectiles() { projs.shrink(0); }

    void newprojectile(const vec& from, const vec& to, float speed, bool local, int id, gameent* owner, int atk)
    {
        projectile& p = projs.add();
        p.dir = vec(to).sub(from).safenormalize();
        p.o = from;
        p.from = from;
        p.to = to;
        p.offset = hudgunorigin(attacks[atk].gun, from, to, owner);
        p.offset.sub(from);
        p.speed = speed;
        p.local = local;
        p.owner = owner;
        p.atk = atk;
        p.offsetmillis = OFFSETMILLIS;
        p.id = local ? lastmillis : id;
    }

    void removeprojectiles(gameent* owner)
    {
        // can't use loopv here due to strange GCC optimizer bug
        int len = projs.length();
        loopi(len) if (projs[i].owner == owner) { projs.remove(i--); len--; }
    }

    VARP(blood, 0, 1, 1);

    void damageeffect(int damage, gameent* d, bool thirdperson)
    {
        vec p = d->o;
        p.z += 0.6f * (d->eyeheight + d->aboveeye) - d->eyeheight;
#if 0
        if (thirdperson) particle_textcopy(d->abovehead(), tempformatstring("%d", damage), PART_TEXT, 2000, 0xFF4B19, 4.0f, -8);
#endif

        if (d == player1 && damage) {
            float droll = damage / 1.9f;
            player1->roll += player1->roll > 0 ? droll : (player1->roll < 0 ? -droll : (rnd(2) ? droll : -droll));
        }
    }

    void spawnbouncer(const vec& p, const vec& vel, gameent* d, int type)
    {
        vec to(rnd(100) - 50, rnd(100) - 50, rnd(100) - 50);
        if (to.iszero()) to.z += 1;
        to.normalize();
        to.add(p);
        newbouncer(p, to, true, 0, d, type, rnd(1000) + 1000, rnd(100) + 20);
    }

    void gibeffect(int damage, const vec& vel, gameent* d)
    {
        if (!blood || !maxgibs || damage < 0) return;
        vec from = d->abovehead();
        loopi(rnd(maxgibs) + 1) spawnbouncer(from, vel, d, BNC_GIBS);
    }

    bool isheadshot(dynent* d, vec to)
    {
        if ((to.z - (d->o.z - d->eyeheight)) / (d->eyeheight + d->aboveeye) > 0.9f) return true;
        return false;
    }

    void hit(int damage, dynent* d, gameent* at, const vec& vel, int atk, float info1, int info2 = 1)
    {
        if (at == player1 && d != at)
        {
            extern int hitsound;
            if (hitsound && lasthit != lastmillis) playsound(S_HIT);
            lasthit = lastmillis;
        }

        gameent* f = (gameent*)d;

        f->lastpain = lastmillis;
        if (at->type == ENT_PLAYER && !isteam(at->team, f->team)) at->totaldamage += damage;

        if (!m_mp(gamemode) || f == at) f->hitpush(damage, vel, at, atk);

        if (!m_mp(gamemode)) damaged(damage, f, at);
        else
        {
            hitmsg& h = hits.add();
            //h.headshots=headshots;
            h.target = f->clientnum;
            h.lifesequence = f->lifesequence;
            h.info1 = int(info1 * DMF);
            h.info2 = info2;
            h.dir = f == at ? ivec(0, 0, 0) : ivec(vec(vel).mul(DNF));
            if (at == player1)
            {
                damageeffect(damage, f);
                if (f == player1)
                {
                    damageblend(damage / 4);
                    damagecompass(damage, at ? at->o : f->o);
                    playsound(S_PAIN2);
                }
                else playsound(S_PAIN1, &f->o);
            }
        }
    }

    void hitpush(int damage, dynent* d, gameent* at, vec& from, vec& to, int atk, int rays)
    {
        hit(damage, d, at, vec(to).sub(from).safenormalize(), atk, from.dist(to), rays);
    }

    float projdist(dynent* o, vec& dir, const vec& v, const vec& vel)
    {
        vec middle = o->o;
        middle.z += (o->aboveeye - o->eyeheight) / 2;
        dir = vec(middle).sub(v).add(vec(vel).mul(5)).safenormalize();

        float low = min(o->o.z - o->eyeheight + o->radius, middle.z),
            high = max(o->o.z + o->aboveeye - o->radius, middle.z);
        vec closest(o->o.x, o->o.y, clamp(v.z, low, high));
        return max(closest.dist(v) - o->radius, 0.0f);
    }

    void radialeffect(dynent* o, const vec& v, const vec& vel, int qdam, gameent* at, int atk)
    {
        if (o->state != CS_ALIVE) return;
        vec dir;
        float dist = projdist(o, dir, v, vel);
        at->headshots = 0;
        if (dist < attacks[atk].exprad)
        {
            float damage = qdam * (1 - dist / EXP_DISTSCALE / attacks[atk].exprad);
            if (o == at) damage /= EXP_SELFDAMDIV;
            if (damage > 0) hit(max(int(damage), 1), o, at, dir, atk, 0, dist);
            if (blood && qdam < 800 && o->type == ENT_PLAYER && qdam) {
                addstain(STAIN_BLOOD, o->o, vec(v).sub(o->o).normalize(), rnd(5) + 5.f, bvec(0x99, 0xFF, 0xFF));
                particle_splash(PART_BLOOD, 1, 200, v, 0x99FFFF, 10.f, 50, 0);
                particle_splash(PART_WATER, 5, 500, v, 0xFF0000, 0.4f, 500);
                //gibeffect(qdam, v, (gameent*)o);
            }
            if (isheadshot(o, v)) {
                at->headshots++;
                at->lastdamage = qdam * EXPLOSION_HEADSHOTMUL;
            }
            else at->lastdamage = qdam;

            if (qdam && at == player1)conoutf(CON_GAMEINFO, "-%d", at->lastdamage);
        }
    }

    void explode(bool local, gameent* owner, const vec& v, const vec& vel, dynent* safe, int damage, int atk)
    {
        particle_splash(PART_SPARK, 200, 300, v, 0x50CFE5, 0.45f);
        playsound(S_PULSEEXPLODE, &v);
        particle_fireball(v, 1.15f * attacks[atk].exprad, PART_PULSE_BURST, int(attacks[atk].exprad * 20), 0x50CFE5, 4.0f);
        vec debrisorigin = vec(v).sub(vec(vel).mul(5));
        adddynlight(safe ? v : debrisorigin, 2 * attacks[atk].exprad, vec(1.0f, 3.0f, 4.0f), 350, 40, 0, attacks[atk].exprad / 2, vec(0.5f, 1.5f, 2.0f));
#if 0
        int numdebris = maxdebris > MINDEBRIS ? rnd(maxdebris - MINDEBRIS) + MINDEBRIS : min(maxdebris, MINDEBRIS);
        if (numdebris)
        {
            vec debrisvel = vec(vel).neg();
            loopi(numdebris)
                spawnbouncer(debrisorigin, owner, BNC_DEBRIS);
        }
#endif
        if (!local) return;
        int numdyn = numdynents();
        loopi(numdyn)
        {
            dynent* o = iterdynents(i);
            if (o->o.reject(v, o->radius + attacks[atk].exprad) || o == safe) continue;
            radialeffect(o, v, vel, damage, owner, atk);
        }
    }

    void pulsestain(const projectile& p, const vec& pos)
    {
        vec dir = vec(p.dir).neg();
        float rad = attacks[p.atk].exprad * 0.75f;
        addstain(STAIN_PULSE_SCORCH, pos, dir, rad);
        addstain(STAIN_PULSE_GLOW, pos, dir, rad, 0x50CFE5);
    }

    void projsplash(projectile& p, const vec& v, dynent* safe)
    {
        explode(p.local, p.owner, v, p.dir, safe, attacks[p.atk].damage, p.atk);
        pulsestain(p, v);
    }

    void explodeeffects(int atk, gameent* d, bool local, int id)
    {
        if (local) return;
        switch (atk)
        {
            //case ATK_PULSE_SHOOT:
//                loopv(projs)
//                {
//                    projectile &p = projs[i];
//                    if(p.atk == atk && p.owner == d && p.id == id && !p.local)
//                    {
//                        vec pos = vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)).add(p.o);
//                        explode(p.local, p.owner, pos, p.dir, NULL, 0, atk);
//                        pulsestain(p, pos);
//                        projs.remove(i);
//                        break;
//                    }
//                }
//                break;
        default:
            break;
        }
    }

    bool projdamage(dynent* o, projectile& p, const vec& v)
    {
        if (o->state != CS_ALIVE) return false;
        if (!intersect(o, p.o, v, attacks[p.atk].margin)) return false;
        projsplash(p, v, o);
        vec dir;
        projdist(o, dir, v, p.dir);
        hit(attacks[p.atk].damage, o, p.owner, dir, p.atk, 0, 0);
        return true;
    }

    void updateprojectiles(int time)
    {
        if (projs.empty()) return;
        gameent* noside = hudplayer();
        loopv(projs)
        {
            projectile& p = projs[i];
            p.offsetmillis = max(p.offsetmillis - time, 0);
            vec dv;
            float dist = p.to.dist(p.o, dv);
            dv.mul(time / max(dist * 1000 / p.speed, float(time)));
            vec v = vec(p.o).add(dv);
            bool exploded = false;
            hits.setsize(0);
            if (p.local)
            {
                vec halfdv = vec(dv).mul(0.5f), bo = vec(p.o).add(halfdv);
                float br = max(fabs(halfdv.x), fabs(halfdv.y)) + 1 + attacks[p.atk].margin;
                loopj(numdynents())
                {
                    dynent* o = iterdynents(j);
                    if (p.owner == o || o->o.reject(bo, o->radius + br)) continue;
                    if (projdamage(o, p, v)) { exploded = true; break; }
                }
            }
            if (!exploded)
            {
                if (dist < 4)
                {
                    if (p.o != p.to) // if original target was moving, reevaluate endpoint
                    {
                        if (raycubepos(p.o, p.dir, p.to, 0, RAY_CLIPMAT | RAY_ALPHAPOLY) >= 4) continue;
                    }
                    projsplash(p, v, NULL);
                    exploded = true;
                }
                else
                {
                    vec pos = vec(p.offset).mul(p.offsetmillis / float(OFFSETMILLIS)).add(v);
                    particle_splash(PART_PULSE_FRONT, 1, 1, pos, 0x50CFE5, 2.4f, 150, 20);
                    if (p.owner != noside)
                    {
                        float len = min(20.0f, vec(p.offset).add(p.from).dist(pos));
                        vec dir = vec(dv).normalize(),
                            tail = vec(dir).mul(-len).add(pos),
                            head = vec(dir).mul(2.4f).add(pos);
                        particle_flare(tail, head, 1, PART_PULSE_SIDE, 0x50CFE5, 2.5f);
                    }
                }
            }
            if (exploded)
            {
                if (p.local)
                    addmsg(N_EXPLODE, "rici3iv", p.owner->headshots, p.owner, lastmillis - maptime, p.atk, p.id - maptime,
                        hits.length(), hits.length() * sizeof(hitmsg) / sizeof(int), hits.getbuf());
                projs.remove(i--);
            }
            else p.o = v;
        }
    }

    void shorten(const vec& from, vec& target, float dist) {
        target.sub(from).mul(min(1.0f, dist)).add(from);
    }

    void railhit(const vec& from, const vec& to, bool stain = true)
    {
        vec dir = vec(from).sub(to).safenormalize();
        if (stain)
        {
            addstain(STAIN_RAIL_HOLE, to, dir, 2.0f);
            addstain(STAIN_RAIL_GLOW, to, dir, 2.5f, 0x50CFE5);
        }
        adddynlight(vec(to).madd(dir, 4), 10, vec(0.25f, 0.75f, 1.0f), 225, 75);
    }

    void raydamage(vec& from, vec& to, gameent* d, int atk)
    {
        dynent* o;
        d->lastdamage = 0;
        float dist;
        d->headshots = 0;
        int maxrays = attacks[atk].rays, margin = attacks[atk].margin;
        if (attacks[atk].rays > 1)
        {
            dynent* hits[MAXRAYS];
            loopi(maxrays)
            {
                //d->headshots[i]=0;
                if ((hits[i] = intersectclosest(from, rays[i], d, margin, dist)))
                {
                    //here we may be looping over MULTIPLE dynent objects, for each hit we make
                    //we need to keep headshots in an array of size MAXRAYS instead of keeping one global headshot variable (otherwise, all players hit by the shotgun will receive the combined headshots
                    //reading this on the server-side will be another thing....
                    vec blooddest = rays[i];
                    blooddest.z += rnd(2) + 2;
                    shorten(from, rays[i], dist);
                    if (blood && d->type == ENT_PLAYER) {
                        particle_splash(PART_BLOOD, 1, 200, rays[i], 0x99FFFF, 5.f, 50, 0);
                        particle_splash(PART_WATER, 5, 500, rays[i], 0xFF0000, 0.4f, 500);
                        if (i % 2 == 0)addstain(STAIN_BLOOD, blooddest, vec(from).sub(blooddest).normalize(), rnd(5) + 5.f, bvec(0x99, 0xFF, 0xFF));
                        //gibeffect(attacks[atk].damage, rays[i], NULL);
                    }
                    if (d == player1 || d->ai) {
                        if (isheadshot(hits[i], rays[i])) {
                            //d->headshots[i]++;
                            d->headshots++;
                            d->lastdamage += attacks[atk].damage * SHOTGUN_HEADSHOTMUL;
                        }
                        else d->lastdamage += attacks[atk].damage;
                    }
                }

            }
            loopi(maxrays) if (hits[i])
            {
                o = hits[i];
                hits[i] = NULL;
                int numhits = 1;
                for (int j = i + 1; j < maxrays; j++) if (hits[j] == o)
                {
                    hits[j] = NULL;
                    numhits++;
                }
                hitpush(numhits * attacks[atk].damage, o, d, from, to, atk, numhits);
            }
        }
        else if ((o = intersectclosest(from, to, d, margin, dist)))
        {
            vec blooddest = to;
            shorten(from, to, dist);
            particle_splash(PART_BLOOD, 1, 200, to, 0x99FFFF, 5.f, 50, 0);
            particle_splash(PART_WATER, 5, 500, to, 0xFF0000, 0.4f, 500);
            addstain(STAIN_BLOOD, blooddest, vec(from).sub(blooddest).normalize(), rnd(5) + 5.f, bvec(0x99, 0xFF, 0xFF));
            if (d == player1 || d->ai) {
                if (isheadshot(o, to)) {
                    d->headshots++;
                    d->lastdamage += attacks[atk].damage * HITSCAN_HEADSHOTMUL;
                }
                else d->lastdamage += attacks[atk].damage;
            }
            //gibeffect(attacks[atk].damage, to, d);
            hitpush(attacks[atk].damage, o, d, from, to, atk, 1);
        }
        if (d->lastdamage && d == player1)conoutf(CON_GAMEINFO, "-%d", d->lastdamage);
    }

    void shoteffects(int atk, const vec& from, const vec& to, gameent* d, bool local, int id, int prevaction)     // create visual effect from a shot
    {
        int gun = attacks[atk].gun;
        vec dest = to, orig = from;
        if (!attacks[atk].projspeed && d != player1) raydamage(orig, dest, d, atk);

        switch (atk)
        {
        case ATK_M4_SECONDARY:
        case ATK_PULSERIFLE_SECONDARY:
            if (d->muzzle.x >= 0)
                particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, 0x50CFE5, 3.50f, d);
            newprojectile(from, to, attacks[atk].projspeed, local, id, d, atk);
            break;

        case ATK_M4_PRIMARY:
        case ATK_357_PRIMARY:
        case ATK_357_SECONDARY:
        case ATK_PISTOL_PRIMARY:
        case ATK_PISTOL_SECONDARY:
        case ATK_PULSERIFLE_PRIMARY:
            particle_splash(PART_SPARK, 200, 250, to, 0x50FF50, 0.45f);
            particle_flare(hudgunorigin(gun, from, to, d), to, 200, PART_RAIL_TRAIL, 0x50FF50, 0.5f);
            railhit(from, to);
            if (d->muzzle.x >= 0)
                particle_flare(d->muzzle, d->muzzle, 140, PART_RAIL_MUZZLE_FLASH, 0x50FF50, 2.75f, d);
            adddynlight(hudgunorigin(gun, d->o, to, d), 35, vec(0.25f, 0.75f, 1.0f), 75, 75, DL_FLASH, 0, vec(0, 0, 0), d);
            break;

        case ATK_SHOTGUN_PRIMARY:
        case ATK_SHOTGUN_SECONDARY:
            particle_splash(PART_SPARK, 200, 250, to, 0x50CFE5, 0.45f);
            loopi(attacks[atk].rays) {
                particle_flare(hudgunorigin(gun, from, rays[i], d), rays[i], 200, PART_RAIL_TRAIL, 0x50CFE5, 0.5f);
                railhit(from, rays[i]);
            }
            if (d->muzzle.x >= 0)
                particle_flare(d->muzzle, d->muzzle, 140, PART_RAIL_MUZZLE_FLASH, 0x50CFE5, 2.75f, d);
            adddynlight(hudgunorigin(gun, d->o, to, d), 35, vec(0.25f, 0.75f, 1.0f), 75, 75, DL_FLASH, 0, vec(0, 0, 0), d);
            break;

        default:
            break;
        }

        //if(d==hudplayer()) playsound(attacks[atk].hudsound, NULL);
        if (atk == ATK_SHOTGUN_SECONDARY) playsound(attacks[ATK_SHOTGUN_PRIMARY].sound, d == hudplayer() ? NULL : &d->o); //hack to also play the shotgun primary sound when firing the secondary, because the licenses of the sounds we have don't allow modification
        playsound(attacks[atk].sound, d == hudplayer() ? NULL : &d->o);
    }

    void particletrack(physent* owner, vec& o, vec& d)
    {
        if (owner->type != ENT_PLAYER) return;
        gameent* pl = (gameent*)owner;
        if (pl->muzzle.x < 0 || pl->lastattack < 0 || attacks[pl->lastattack].gun != pl->gunselect) return;
        float dist = o.dist(d);
        o = pl->muzzle;
        if (dist <= 0) d = o;
        else
        {
            vecfromyawpitch(owner->yaw, owner->pitch, 1, 0, d);
            float newdist = raycube(owner->o, d, dist, RAY_CLIPMAT | RAY_ALPHAPOLY);
            d.mul(min(newdist, dist)).add(owner->o);
        }
    }

    void dynlighttrack(physent* owner, vec& o, vec& hud)
    {
        if (owner->type != ENT_PLAYER) return;
        gameent* pl = (gameent*)owner;
        if (pl->muzzle.x < 0 || pl->lastattack < 0 || attacks[pl->lastattack].gun != pl->gunselect) return;
        o = pl->muzzle;
        hud = owner == hudplayer() ? vec(pl->o).add(vec(0, 0, 2)) : pl->muzzle;
    }

    float intersectdist = 1e16f;

    bool intersect(dynent* d, const vec& from, const vec& to, float margin, float& dist)   // if lineseg hits entity bounding box
    {
        vec bottom(d->o), top(d->o);
        bottom.z -= d->eyeheight + margin;
        top.z += d->aboveeye + margin;
        return linecylinderintersect(from, to, bottom, top, d->radius + margin, dist);
    }

    dynent* intersectclosest(const vec& from, const vec& to, gameent* at, float margin, float& bestdist)
    {
        dynent* best = NULL;
        bestdist = 1e16f;
        loopi(numdynents())
        {
            dynent* o = iterdynents(i);
            if (o == at || o->state != CS_ALIVE) continue;
            float dist;
            if (!intersect(o, from, to, margin, dist)) continue;
            if (dist < bestdist)
            {
                best = o;
                bestdist = dist;
            }
        }
        return best;
    }

    void shoot(gameent* d, const vec& targ)
    {
        int prevaction = d->lastaction, attacktime = lastmillis - prevaction;
        if (attacktime < d->gunwait) return;
        d->gunwait = 0;
        if (!d->attacking) return;
        int gun = d->gunselect, act = d->attacking, atk = guns[gun].attacks[act];
        d->lastaction = lastmillis;
        d->lastattack = atk;
        if (!(d->ammo[ATKAMMOTYPE(atk)] - attacks[atk].use))
        {
            if (d == player1)
            {
                msgsound(S_NOAMMO, d);
                d->gunwait = 600;
                d->lastattack = -1;
                //weaponswitch(d);
            }
            return;
        }
        d->ammo[ATKAMMOTYPE(atk)] -= attacks[atk].use;

        vec from = d->o, to = targ, dir = vec(to).sub(from).safenormalize();
        float dist = to.dist(from);
        if (!(d->physstate >= PHYS_SLOPE && d->crouching && d->crouched()))
        {
            vec kickback = vec(dir).mul(attacks[atk].kickamount * -2.5f);
            d->vel.add(kickback);
        }
        float shorten = attacks[atk].range && dist > attacks[atk].range ? attacks[atk].range : 0,
            barrier = raycube(d->o, dir, dist, RAY_CLIPMAT | RAY_ALPHAPOLY);
        if (barrier > 0 && barrier < dist && (!shorten || barrier < shorten))
            shorten = barrier;
        if (shorten) to = vec(dir).mul(shorten).add(from);

        if (attacks[atk].rays > 1) createrays(atk, from, to);
        else if (attacks[atk].spread) offsetray(from, to, attacks[atk].spread, attacks[atk].range, to);

        hits.setsize(0);

        if (!attacks[atk].projspeed) raydamage(from, to, d, atk);

        shoteffects(atk, from, to, d, true, 0, prevaction);

        if (d == player1 || d->ai)
        {
            addmsg(N_SHOOT, "rici2i6iv", d->headshots, d, lastmillis - maptime, atk,
                (int)(from.x * DMF), (int)(from.y * DMF), (int)(from.z * DMF),
                (int)(to.x * DMF), (int)(to.y * DMF), (int)(to.z * DMF),
                hits.length(), hits.length() * sizeof(hitmsg) / sizeof(int), hits.getbuf());
        }

        if (!attacks[atk].automatic) d->attacking = ACT_IDLE;
        d->gunwait = attacks[atk].attackdelay;
        if (attacks[atk].action != ACT_SECONDARY && d->ai) d->gunwait += int(d->gunwait * (((101 - d->skill) + rnd(111 - d->skill)) / 100.f));
        d->totalshots += attacks[atk].damage * attacks[atk].rays;
    }

    void adddynlights()
    {
        loopv(projs)
        {
            projectile& p = projs[i];
            //            if(p.atk!=ATK_PULSE_SHOOT) continue;
            vec pos(p.o);
            pos.add(vec(p.offset).mul(p.offsetmillis / float(OFFSETMILLIS)));
            adddynlight(pos, 20, vec(0.25f, 0.75f, 1.0f));
        }
    }

    static const char* const gibnames[3] = { "gibs/gib01", "gibs/gib02", "gibs/gib03" };
#if 0
    static const char* const debrisnames[4] = { "debris/debris01", "debris/debris02", "debris/debris03", "debris/debris04" };
#endif

    void preloadbouncers()
    {
        loopi(sizeof(gibnames) / sizeof(gibnames[0])) preloadmodel(gibnames[i]);
#if 0
        loopi(sizeof(debrisnames) / sizeof(debrisnames[0])) preloadmodel(debrisnames[i]);
#endif
    }

    void renderbouncers()
    {
        float yaw, pitch;
        loopv(bouncers)
        {
            bouncer& bnc = *bouncers[i];
            vec pos(bnc.o);
            pos.add(vec(bnc.offset).mul(bnc.offsetmillis / float(OFFSETMILLIS)));
            vec vel(bnc.vel);
            if (vel.magnitude() <= 25.0f) yaw = bnc.lastyaw;
            else
            {
                vectoyawpitch(vel, yaw, pitch);
                yaw += 90;
                bnc.lastyaw = yaw;
            }
            pitch = -bnc.roll;
            const char* mdl = NULL;
            int cull = MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED;
            float fade = 1;
            if (bnc.lifetime < 250) fade = bnc.lifetime / 250.0f;
            switch (bnc.bouncetype)
            {
            case BNC_GIBS: mdl = gibnames[bnc.variant]; break;
#if 0
            case BNC_DEBRIS: mdl = debrisnames[bnc.variant]; break;
#endif
            default: continue;
            }
            rendermodel(mdl, ANIM_MAPMODEL | ANIM_LOOP, pos, yaw, pitch, 0, cull, NULL, NULL, 0, 0, fade);
        }
    }

    void renderprojectiles()
    {
    }

    void removeweapons(gameent* d)
    {
        removebouncers(d);
        removeprojectiles(d);
    }

    void updateweapons(int curtime)
    {
        updateprojectiles(curtime);
        if (player1->clientnum >= 0 && player1->state == CS_ALIVE) shoot(player1, worldpos); // only shoot when connected to server
        updatebouncers(curtime); // need to do this after the player shoots so bouncers don't end up inside player's BB next frame
    }

    void avoidweapons(ai::avoidset& obstacles, float radius)
    {
        loopv(projs)
        {
            projectile& p = projs[i];
            obstacles.avoidnear(NULL, p.o.z + attacks[p.atk].exprad + 1, p.o, radius + attacks[p.atk].exprad);
        }
    }
};

