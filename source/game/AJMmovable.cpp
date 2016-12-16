// This is a modified movable.cpp: implements physics for inanimate models
// it should be a good breakout for our bullet ents
#include "cube.h"

#include "game.h"
#include "engine.h"

extern int physsteps;

namespace game
{
    enum
    {
        BULLETWEIGHT = 25,
        BULLETHEALTH = 5,
        BARRELWEIGHT = 25,
        PLATFORMWEIGHT = 1000,
        PLATFORMSPEED = 8,
        EXPLODEDELAY = 200
    };
	
	struct bulletmovable {
		int etype, mapmodel, weight;
		float yaw, pitch, roll;
		vec o, radius;
		btRigidBody* body;
		bulletmovable(const entity &e) : 
			etype(e.type), mapmodel(e.attr1),
			yaw(e.attr2), pitch(e.attr3), roll(e.attr4), 
			o(e.o), weight(e.attr5)
		{
			const char *mdlname = mapmodelname(e.attr1);
			if (!mdlname) return;
			model *m = loadmodel(mdlname);
			if (!m) return;
			vec center, bbradius;
			m->boundbox(center, bbradius);
			radius = bbradius;
			
		}
	};
    //struct bulletmovable : dynent
    //{
    //    int etype, mapmodel, health, weight, exploding, tag, dir;
    //    physent *stacked;
    //    vec stackpos;

    //    bulletmovable(const entity &e) : 
    //        etype(e.type),
    //        mapmodel(e.attr2),
    //        health(e.type==BARREL ? (e.attr4 ? e.attr4 : BULLETHEALTH) : 0), 
    //        weight(e.type==PLATFORM || e.type==ELEVATOR ? PLATFORMWEIGHT : (e.attr3 ? e.attr3 : (e.type==BARREL ? BARRELWEIGHT : BULLETWEIGHT))), 
    //        exploding(0),
    //        tag(e.type==PLATFORM || e.type==ELEVATOR ? e.attr3 : 0),
    //        dir(e.type==PLATFORM || e.type==ELEVATOR ? (e.attr4 < 0 ? -1 : 1) : 0),
    //        stacked(NULL),
    //        stackpos(0, 0, 0)
    //    {
    //        state = CS_ALIVE;
    //        type = ENT_BULLETENT;
    //        yaw = e.attr1;
    //        //if(e.type==PLATFORM || e.type==ELEVATOR) 
    //        //{
    //            //maxspeed = e.attr4 ? fabs(float(e.attr4)) : PLATFORMSPEED;
    //            //if(tag) vel = vec(0, 0, 0);
    //            //else if(e.type==PLATFORM) { vecfromyawpitch(yaw, 0, 1, 0, vel); vel.mul(dir*maxspeed); } 
    //           // else vel = vec(0, 0, dir*maxspeed);
    //        //}

    //        const char *mdlname = mapmodelname(e.attr2);
    //        if(mdlname) setbbfrommodel(this, mdlname);
    //    }
    //   
    //    void hitpush(int damage, const vec &dir, gameent *actor, int gun)//concern gun
    //    {
    //        if(etype!=BULLETENT) return;
    //        vec push(dir);
    //        //push.mul(80*damage);///weight);
    //        push.mul((actor==actor && attacks[gun].exprad ? EXP_SELFPUSH : 1.0f)*attacks[gun].hitpush*damage/weight);
    //        vel.add(push);

    //    }

    //    void explode(dynent *at)
    //    {
    //        state = CS_DEAD;
    //        exploding = 0;
    //        //game::explode(true, (gameent *)at, o, this, guns[GUN_BARREL].damage, GUN_BARREL);
    //    }
 
    //    void damaged(int damage, gameent *at, int gun = -1)
    //    {
    //        if(etype!=BARREL || state!=CS_ALIVE || exploding) return;
    //        health -= damage*damage;
    //        if(health>0) return;
    //        //if(gun==GUN_BARREL) exploding = lastmillis + EXPLODEDELAY;
    //        else explode(at); 
    //    }

    //    void suicide()
    //    {
    //        state = CS_DEAD;
    //        if(etype==BARREL) explode(player1);
    //    }
    //};

    vector<bulletmovable *> bulletmovables;
   
    void clearbulletmovables()
    {
        if(bulletmovables.length())
        {
           // cleardynentcache();
            bulletmovables.deletecontents();
        }
        //if(!m_dmsp && !m_classicsp) return;
        loopv(entities::ents) 
        {
            entity &e = *entities::ents[i];
            if (e.type==MAPMODEL){
				const char *mdlname = mapmodelname(e.attr1);
                setcbfrommodel(e.o, mdlname, vec(e.attr2 + 90.f, e.attr3, e.attr4), 0.f);
			}
            if(e.type!=BULLETENT) continue;
            bulletmovable *m = new bulletmovable(e);
            bulletmovables.add(m);
            //m->o = e.o;
	    const char *mdlname = mapmodelname(m->mapmodel);
         //   entinmap(m);
	    m->body = setcbfrommodel(m->o, mdlname, vec(m->yaw, m->pitch, m->roll), m->weight);
          //  updatedynentcache(m);
        }
    }
	//void QuaternionToEuler(const btQuaternion &TQuat, vec &TEuler) {
	//	float W = TQuat.getW();
	//	float X = TQuat.getX();
	//	float Z = -TQuat.getY();
	//	float Y = TQuat.getZ();
	//	float WSquared = W * W;
	//	float XSquared = X * X;
	//	float YSquared = Y * Y;
	//	float ZSquared = Z * Z;
	//	float yaw = (-2.0f * ((X * Z) - (Y * W)));
	//	TEuler.x = (atan2f(2.0f * (Y * Z + X * W), -XSquared - YSquared + ZSquared + WSquared));
	//	TEuler.y = (asin(yaw >= 1.0f ? 1.00f : yaw <= -1.0f ? -1.00f : yaw));
	//	TEuler.z = (atan2f(2.0f * (X * Y + Z * W), XSquared - YSquared - ZSquared + WSquared));
	//	TEuler.mul(180.0f / PI);//90,180,270
	//	float factor = 10.f;
	//	TEuler = vec(int(TEuler.x*factor)*(1 / factor), int(TEuler.y*factor)*(1 / factor), int(TEuler.z*factor)*(1 / factor));
	//}
	void updatebulletmovables(vec tmpvec, vec tmprot, int j)
	{
		loopv(bulletmovables){
			bulletmovable *m = bulletmovables[i];
			if (!m->body || !m->body->getMotionState()) continue;
			vec tmpvec;
			vec tmprot;
			btTransform trans;
			m->body->getMotionState()->getWorldTransform(trans);
			//vec dir(m->body->getOrientation().getX(), m->body->getOrientation().getZ(), m->body->getOrientation().getY());
			vec EulerRotation;
			//QuaternionToEuler(m->body->getOrientation(), EulerRotation);
			//conoutf("world pos = %f,%f,%f\n",float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
			//tmpvec = vec(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
			//tmprot = vec(float(trans.getRotation().getX()),float(trans.getRotation().getY()),float(trans.getRotation().getZ()));
			tmpvec = vec(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());
			
			//tmprot = vec(trans.getRotation().getX(),trans.getRotation().getY(),trans.getRotation().getZ());
			tmprot = EulerRotation;
			//if (-INFINITY == m->pitch) conoutf("HELLO THERE ITS COOL IM NEGATIVE");
			m->o = vec(tmpvec.x * 17, tmpvec.z * 17, tmpvec.y * 17);
			m->yaw = tmprot.z - 90.f;
			m->pitch = tmprot.y; //correct
			m->roll = tmprot.x;
			//m->pitch = m->pitch;
			//if (m->pitch < -100.f) m->pitch = 90.f;
			//conoutf("rotation = %d,%d,%d", int(m->yaw), int(m->pitch), int(m->roll));
			//conoutf("pitch roll %f", m->pitch);
		}
		//return;
		//if (j == 0) return;
		////if(!curtime) return;
		//loopv(bulletmovables)
		//{
		//	//conoutf(");
		//	if (i == j - 1){

		//		bulletmovable *m = bulletmovables[i];
		//		//if(m->state!=CS_ALIVE) break;
		//		//0.058825
		//		m->o = vec(tmpvec.x * 17, tmpvec.z * 17, tmpvec.y * 17);
		//		m->yaw = tmprot.z - 90;
		//		m->pitch = tmprot.y; //correct
		//		m->roll = tmprot.x;

		//		//rendermodel(mdlname, ANIM_MAPMODEL | ANIM_LOOP, o, yaw, pitch, roll, MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED);

		//		conoutf("rotation = %f,%f,%f", m->yaw, m->pitch, m->roll);
		//	}
		//}
	}
    void renderbulletmovables()
    {
        loopv(bulletmovables)
        {
            bulletmovable &m = *bulletmovables[i];
           // if(m.state!=CS_ALIVE) continue;
            //vec o = m.feetpos();//o must be center of model
			//vec pos(0, 0, 1);
			//if(m.roll)pos.rotate_around_y(-m.roll*RAD);
			//if (m.pitch) pos.rotate_around_x(m.pitch*RAD);
			//if (m.yaw)pos.rotate_around_z(m.yaw*RAD);
			//pos.mul(-m.radius.z);
			//pos.add(m.o);
			//particle_flare(pos.add(m.o),m.o , 0, PART_STREAK, 0x0000FF);

			const char *mdlname = mapmodelname(m.mapmodel);
            if(!mdlname) continue;
			rendermodel(mdlname, ANIM_MAPMODEL|ANIM_LOOP, m.o, m.yaw, m.pitch, m.roll, MDL_CULL_VFC | MDL_CULL_DIST|MDL_CULL_OCCLUDED|MDL_CENTER_ALIGNED );

        }
    }
}
