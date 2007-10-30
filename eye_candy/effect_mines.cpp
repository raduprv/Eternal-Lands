#ifdef MINES

#ifdef EYE_CANDY

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_mines.h"
#include "orbital_mover.h"

namespace ec
{

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

MineParticle::MineParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD, const MineEffect::MineType _type) : Particle(_effect, _mover, _pos, _velocity)
{
  type = _type;
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  texture = _texture;
  size = _size * (0.5 + randcoord()) * 10 / _LOD;
  alpha = _alpha;
  flare_max = 5.0;
  flare_exp = 0.1;
  flare_frequency = 3.0;
  LOD = _LOD;
  state = 0;
}

bool MineParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;
    
  const interval_t float_time = delta_t / 1000000.0;
  const Uint64 age = get_time() - born;
  switch(type)
  {
    case MineEffect::DETONATE_MAGIC_IMMUNITY_REMOVAL:
    {
		if (pos.y > 20)
			return false;
		
		velocity.y *= 1.15;
		velocity.x *= 0.82;
		velocity.z *= 0.82;
		
		const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 1);
		alpha *= scalar;
		
		break;
    }
    case MineEffect::DETONATE_UNINVIZIBILIZER:
    {
		if (alpha < 0.01)
			return false;
		
		const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 1);
		alpha *= scalar;
		size *= 0.95;
		
		break;
    }
    case MineEffect::DETONATE_MANA_DRAINER:
    {
		if (pos.y < -2.0)
			return false;
		
		break;
    }
    case MineEffect::DETONATE_MANA_BURNER:
    {
      if (age < 650000)
        break;
    
      if (alpha < 0.03)
        return false;

      const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 6);
      alpha *= scalar;

      break;
    }
    case MineEffect::DETONATE_CALTROP:
    case MineEffect::DETONATE_CALTROP_POISON:
    {
      if (age < 700000)
      {
		  velocity *= 0.85;
      }
      else
      {
        const percent_t scalar = math_cache.powf_05_close(float_time * 1);
        alpha *= scalar;
    
        if (alpha < 0.01)
          return false;
      }
      break;
    }
    case MineEffect::DETONATE_TRAP:
    {
      color[0] = 0.7 + 0.3 * sin(age / 530000.0);
      color[1] = 0.7 + 0.3 * sin(age / 970000.0 + 1.3);
      color[2] = 0.7 + 0.3 * sin(age / 780000.0 + 1.9);
    
	  pos.y += (0.4 - pos.y) * 0.2;
	
      if (age > 4700000)
          alpha *= 0.5;

      if (alpha < 0.01)
          return false;
	  
      break;
    }
    case MineEffect::DETONATE_TYPE1_SMALL:
    case MineEffect::DETONATE_TYPE1_MEDIUM:
    case MineEffect::DETONATE_TYPE1_LARGE:
    {
		if (alpha < 0.01)
			return false;
		
		velocity *= 0.5;
		
//		const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 2);
		if (age > 500000)
			alpha *= 0.8;	// scalar
		
		break;
    }
  }
  
  return true;
}

void MineParticle::draw(const Uint64 usec)
{
  if ((type == MineEffect::DETONATE_CALTROP) || (type == MineEffect::DETONATE_CALTROP_POISON) || (type == MineEffect::DETONATE_TRAP) || 
	  ((type == MineEffect::DETONATE_TYPE1_SMALL || type == MineEffect::DETONATE_TYPE1_MEDIUM || 
  		type == MineEffect::DETONATE_TYPE1_LARGE) && (state == 0))
     )
    Particle::draw(usec);
  else
  {
    glEnable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
    glNormal3f(0.0, 1.0, 0.0);
    Particle::draw(usec);
  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_LIGHTING);
  }
}

GLuint MineParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

light_t MineParticle::get_light_level()
{
  if ((type == MineEffect::DETONATE_CALTROP) || (type == MineEffect::DETONATE_CALTROP_POISON) || (type == MineEffect::DETONATE_TRAP))
    return alpha * size / 1500;
  else if (type == MineEffect::DETONATE_UNINVIZIBILIZER)
    return alpha * size / 1000;
  else
    return 0.0;
};

MineEffect::MineEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const MineType _type, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "MineEffect (" << this << ") created (" << type << ")." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  effect_center = *pos;
  type = _type;
  LOD = base->last_forced_LOD;
  desired_LOD = _LOD;
  spawner = NULL;
  bounds = NULL;
  mover = NULL;
  spawner2 = NULL;
  mover2 = NULL;
  
  switch(type)
  {
    case DETONATE_MAGIC_IMMUNITY_REMOVAL:
    {
      spawner = new FilledSphereSpawner(0.1);
      mover = new ParticleMover(this);
      while ((int)particles.size() < LOD * 150)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity;
        velocity.randomize(0.5);
        velocity.y = 0.2;
        coords += effect_center;
        Particle * p = new MineParticle(this, mover, coords, velocity, 0.2, 1.0, 3.0, 3.0, 3.0, &(base->TexSimple), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
	  break;
    }
    case DETONATE_UNINVIZIBILIZER:
    {
      effect_center.y += 1.8;
      spawner = new HollowDiscSpawner(0.3);
      mover = new ParticleMover(this);
      while ((int)particles.size() < LOD * 200)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.2);
        coords += effect_center;
        coords.y = randfloat(1.8);
        Particle * p = new MineParticle(this, mover, coords, velocity, 1.2, 1.0, 3.0, 3.0, 3.0, &(base->TexSimple), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
	  break;
    }
    case DETONATE_MANA_DRAINER:
    {
      effect_center.y += 1.6;
      spawner = new HollowDiscSpawner(0.3);
      mover = new SimpleGravityMover(this);
      while ((int)particles.size() < LOD * 50)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.2);
        coords += effect_center;
        Particle * p = new MineParticle(this, mover, coords, velocity, 0.3, 1.0, 0.8, 0.35, 0.7, &(base->TexSimple), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
	  break;
    }
    case DETONATE_MANA_BURNER:
    {
      effect_center.y += 1.0;
      spawner = new FilledSphereSpawner(0.5);
      mover = new GravityMover(this, &effect_center, 8e9);
      while ((int)particles.size() < LOD * 100)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.9);
        coords += effect_center;
        Particle * p = new MineParticle(this, mover, coords, velocity, 0.5, 0.5, 0.8, 0.35, 0.7, &(base->TexTwinflare), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case DETONATE_CALTROP:
    case DETONATE_CALTROP_POISON:
    {
      effect_center.y += 0.05;
      mover = new SimpleGravityMover(this);
      spawner = new HollowSphereSpawner(0.1);

      for (int i = 0; i < LOD * 10; i++)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity(0.0, 5.0, 0.0);
        coords += effect_center;
        Particle* p = new MineParticle(this, mover, coords, velocity, 0.75, 0.6, 0.4, (type == DETONATE_CALTROP ? 0.3 : 0.5), 0.3, &(base->TexTwinflare), LOD, type);
        p->state = 1;
        if (!base->push_back_particle(p))
          break;
      }
      
      break;
    }
    case DETONATE_TRAP:
    {
      effect_center.y += 0.4;
      mover = new OrbitalMover(this, effect_center);
      spawner = new HollowSphereSpawner(0.3);
      Particle* p;

      for (int i = 0; i < LOD * 10; i++)
      {
          Vec3 c = effect_center;
          c.y = -0.2 - (i * 0.2);
          Vec3 vel;
          vel.randomize();
          vel.normalize(2.0);
          vel *= randfloat() * 4.0;
          p = new MineParticle(this, mover, c, vel, 0.2, 1.0, 1.0, 1.0, 1.0, &(base->TexVoid), LOD, type);
          if (!base->push_back_particle(p))
              break;

          dynamic_cast<OrbitalMover*>(mover)->setParticleData( p, OrbitalParticleData( i, 10, 0.45, 7 ) );
      }

      break;
    }
    case DETONATE_TYPE1_SMALL:
    case DETONATE_TYPE1_MEDIUM:
    case DETONATE_TYPE1_LARGE:
    {
      float i;
      spawner = new HollowSphereSpawner(0.1);
      mover = new ParticleMover(this);
      i = (type == DETONATE_TYPE1_SMALL ? 0.5 : type == DETONATE_TYPE1_MEDIUM ? 0.8 : 3.0);
      while ((int)particles.size() < LOD * 100)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity = coords * 150;
        velocity.y *= 2;
        coords += effect_center;
		coords.y -= 0.1;
        Particle * p = new MineParticle(this, mover, coords, velocity, i, 1, 0.9, 0.5, 0.3, &(base->TexFlare), LOD, type);
        p->state = 1;
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
  }
}

MineEffect::~MineEffect()
{
  if (spawner)
    delete spawner;
  if (mover)
    delete mover;
  if (spawner2)
    delete spawner2;
  if (mover2)
    delete mover2;
  if (EC_DEBUG)
    std::cout << "MineEffect (" << this << ") destroyed." << std::endl;
}

bool MineEffect::idle(const Uint64 usec)
{
  if (particles.size() == 0)
    return false;
    
  effect_center.x = pos->x;
  effect_center.y += usec / 3000000.0;
  effect_center.z = pos->z;
  
  gravity_center.y += usec / 10000000.0;
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef EYE_CANDY

#endif // MINES
