
// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_glow.h"

namespace ec
{

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

GlowParticle::GlowParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD, const GlowEffect::GlowType _type) : Particle(_effect, _mover, _pos, _velocity)
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

bool GlowParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;
    
  const interval_t float_time = delta_t / 1000000.0;
  const Uint64 age = get_time() - born;
  switch(type)
  {
    case GlowEffect::REMOTE_HEAL_GLOW:
    {
      color[0] = 0.4 + randcolor(0.15) * sin(age / 530000.0);
      color[1] = 0.7;
      color[2] = 0.2 + 0.15 * sin(age / 780000.0 + 1.9);
    
      if (age < 200000)
      {
        if (state == 0)
          size = age / 45000.0;
      }
      else
      {
        const percent_t scalar = math_cache.powf_05_close(float_time * 0.5);
        size *= scalar;
        alpha *= scalar;
    
        if (alpha < 0.01)
          return false;
      }
      break;
    }
    case GlowEffect::HARM_GLOW:
    {
      color[0] = 1.0;
      color[1] = 0.4 + randcolor(0.15) * sin(age / 530000.0);
      color[2] = 0.2 + 0.15 * sin(age / 780000.0 + 1.9);
    
      if (age < 200000)
      {
        if (state == 0)
          size = age / 45000.0;
      }
      else
      {
        const percent_t scalar = math_cache.powf_05_close(float_time * 0.5);
        size *= scalar;
        alpha *= scalar;
    
        if (alpha < 0.01)
          return false;
      }
      break;
    }
    case GlowEffect::POISON_GLOW:
    {
      color[0] = randcolor(0.3);
      color[1] = 0.5 + randcolor(0.3);
      color[2] = randcolor(0.5);
    
      if (age < 200000)
      {
        if (state == 0)
          size = age / 45000.0;
      }
      else
      {
        const percent_t scalar = math_cache.powf_05_close(float_time * 0.5);
        size *= scalar;
        alpha *= scalar;
    
        if (alpha < 0.01)
          return false;
      }
      break;
    }
    case GlowEffect::LEVEL_UP_OA_GLOW:
    case GlowEffect::LEVEL_UP_ATT_GLOW:
    case GlowEffect::LEVEL_UP_DEF_GLOW:
    {
      const Uint64 age = get_time() - born;
      if (age < 950000)
        break;
    
      if (alpha < 0.01)
        return false;

      const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 1.0); // orig: 6, smaller numbers -> longer effect
      alpha *= scalar;
	  break;
	}
  }

  // if the effect moved, shift the particle
  pos += ((GlowEffect*)effect)->shift;
  
  // rotate particle around effect's y achsis
  switch(type)
  {
    case GlowEffect::LEVEL_UP_OA_GLOW:
    {
	  // relative position of the particle to the effect center
	  Vec3 relpos;
	  relpos.y = 0;
	  relpos.x = pos.x - ((GlowEffect*)effect)->pos->x;
	  relpos.z = pos.z - ((GlowEffect*)effect)->pos->z;
	  
	  // relative position to rotate
	  Vec3 rotrelpos = relpos;
	  const double angle = M_PI / 64.0 * (relpos.x*relpos.x+relpos.z*relpos.z);
	  // rotate it around y achsis
	  rotrelpos.x = relpos.x * cos(angle) + relpos.z * sin(angle);
	  rotrelpos.z = -relpos.x * sin(angle) + relpos.z * cos(angle);

	  // move particle
	  pos += (relpos - rotrelpos);
	}
	default:
	{
	  break;
	}
  }
	  
  return true;
}

GLuint GlowParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

light_t GlowParticle::get_light_level()
{
  return alpha * size / 1500;
};

GlowEffect::GlowEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const GlowType _type, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "GlowEffect (" << this << ") created (" << type << ")." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  effect_center = *pos;
  type = _type;
  LOD = base->last_forced_LOD;
  desired_LOD = _LOD;
  spawner = NULL;
  spawner2 = NULL;
  spawner3 = NULL;
  bounds = NULL;
  mover = NULL;
  mover2 = NULL;
  mover3 = NULL;
  shift = Vec3(0.0, 0.0, 0.0);
  
  switch(type)
  {
    case GlowEffect::REMOTE_HEAL_GLOW:
    {
      mover = new ParticleMover(this);
	  spawner = new FilledEllipsoidSpawner(ec::Vec3(2.0, 1.0, 0.1));
      for (int i = 0; i < LOD * 64; i++)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity = coords / 14.0;
        coords += effect_center;
        Particle* p = new GlowParticle(this, mover, coords, velocity, 0.65, 0.05, 0.4 + randcolor(0.3), 0.7, 0.2, &(base->TexShimmer), LOD, type);
        p->state = 1;
        if (!base->push_back_particle(p))
          break;
      }
      Particle* p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
      if (!base->push_back_particle(p))
        break;
      p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
      base->push_back_particle(p);
      break;
    }
    case GlowEffect::HARM_GLOW:
    {
      mover = new ParticleMover(this);
	  spawner = new FilledEllipsoidSpawner(ec::Vec3(2.0, 1.0, 0.1));
      for (int i = 0; i < LOD * 64; i++)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity = coords / 14.0;
        coords += effect_center;
        Particle* p = new GlowParticle(this, mover, coords, velocity, 0.95, 0.05, 1.0, 0.4 + randcolor(0.3), 0.2, &(base->TexFlare), LOD, type);
        p->state = 1;
        if (!base->push_back_particle(p))
          break;
      }
      Particle* p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
      if (!base->push_back_particle(p))
        break;
      p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
      base->push_back_particle(p);
      break;
    }
    case GlowEffect::POISON_GLOW:
    {
      mover = new ParticleMover(this);
	  spawner = new FilledEllipsoidSpawner(ec::Vec3(2.0, 1.0, 0.1));
      for (int i = 0; i < LOD * 64; i++)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity = coords / 14.0;
        coords += effect_center;
        Particle* p = new GlowParticle(this, mover, coords, velocity, 1.95, 0.05, randcolor(0.3), 0.5 + randcolor(0.3), randcolor(0.5), &(base->TexInverse), LOD, type);
        p->state = 1;
        if (!base->push_back_particle(p))
          break;
      }
      Particle* p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
      if (!base->push_back_particle(p))
        break;
      p = new GlowParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexCrystal), LOD, type);
      base->push_back_particle(p);
      break;
    }
    case GlowEffect::LEVEL_UP_OA_GLOW:
    {
      spawner = new FilledSphereSpawner(0.75);
      spawner2 = new FilledDiscSpawner(0.55);
      spawner3 = new FilledSphereSpawner(0.9);
      mover = new GravityMover(this, &effect_center, 4e9);
      mover2 = new GravityMover(this, &effect_center, 4e9);
      mover3 = new GravityMover(this, &effect_center, 9e3);
      for (int i = 0; i < LOD * 32; i++)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.75);
        coords += effect_center;
        Particle * p = new GlowParticle(this, mover, coords, velocity, 3.0 + randcolor(0.75), 0.7 + randcolor(0.3), 254.0/255.0, 254.0/255.0 - 0.1 + randcolor(0.1), 0.0, &(base->TexInverse), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      for (int i = 0; i < LOD * 32; i++)
      {
        Vec3 coords = spawner2->get_new_coords();
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.9);
        coords += effect_center;
        Particle * p = new GlowParticle(this, mover2, coords, velocity, 2.75, 0.7 + randcolor(0.3), 254.0/255.0 - 0.2 + randcolor(0.2), randcolor(1.0), randcolor(0.33), &(base->TexTwinflare), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      for (int i = 0; i < LOD * 64; i++)
      {
        Vec3 coords = spawner3->get_new_coords();
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.9);
        coords += effect_center;
        Particle * p = new GlowParticle(this, mover3, coords, velocity, 2.25, 0.5 + randcolor(0.5), randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexCrystal), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
	  break;
	}
    case LEVEL_UP_ATT_GLOW:
    {
      spawner = new FilledEllipsoidSpawner(ec::Vec3(2.0, 1.0, 0.1));
      spawner2 = new HollowDiscSpawner(0.65);
      spawner3 = new FilledSphereSpawner(0.9);
      mover = new SpiralMover(this, &effect_center, 5.0, 80.0);
      mover2 = new SpiralMover(this, &effect_center, 15.0, 14.0);
      mover3 = new GravityMover(this, &effect_center, 9e3);
      for (int i = 0; i < LOD * 32; i++)
      {
        Vec3 coords = spawner->get_new_coords() + effect_center;
        Vec3 velocity;
        velocity.randomize(0.36);
        velocity.y *= 2.5;
        velocity.y -= 0.7;
        Particle * p = new GlowParticle(this, mover, coords, velocity, 8.0, 0.5, 1.0, 0.0, 0.0, &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      for (int i = 0; i < LOD * 32; i++)
      {
        Vec3 coords = spawner2->get_new_coords() + effect_center;
        Vec3 velocity;
        velocity.randomize(0.36);
        velocity.y *= 2.5;
        velocity.y -= 0.7;
        Particle * p = new GlowParticle(this, mover2, coords, velocity, 2.0, 1.0, 1.0, 0.2, 0.2, &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      for (int i = 0; i < LOD * 64; i++)
      {
        Vec3 coords = spawner3->get_new_coords();
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.9);
        coords += effect_center;
        Particle * p = new GlowParticle(this, mover3, coords, velocity, 2.25, 0.5 + randcolor(0.5), randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexCrystal), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case LEVEL_UP_DEF_GLOW:
    {
      spawner = new HollowSphereSpawner(1.25);
      spawner3 = new FilledSphereSpawner(0.9);
      mover = new GravityMover(this, &effect_center, 3e10);
      mover3 = new GravityMover(this, &effect_center, 9e3);
      for (int i = 0; i < LOD * 192; i++)
      {
        Vec3 coords = spawner->get_new_coords() + effect_center;
        Vec3 velocity;
        velocity.randomize(0.1);
        velocity.normalize(0.1);
        Particle * p = new GlowParticle(this, mover, coords, velocity, 2.0, 0.5, 0.0, 0.0, 1.0, &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      for (int i = 0; i < LOD * 64; i++)
      {
        Vec3 coords = spawner3->get_new_coords();
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.9);
        coords += effect_center;
        Particle * p = new GlowParticle(this, mover3, coords, velocity, 2.25, 0.5 + randcolor(0.5), randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexCrystal), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
  }
}

GlowEffect::~GlowEffect()
{
  if (spawner)
    delete spawner;
  if (spawner2)
    delete spawner2;
  if (spawner3)
    delete spawner3;
  if (mover)
    delete mover;
  if (mover2)
    delete mover2;
  if (mover3)
    delete mover3;
  if (EC_DEBUG)
    std::cout << "GlowEffect (" << this << ") destroyed." << std::endl;
}

bool GlowEffect::idle(const Uint64 usec)
{
  if (particles.size() == 0)
    return false;
    
  if (recall)
    return true;

  const Vec3 last_effect_center = effect_center;

  effect_center.x = pos->x;
  effect_center.z = pos->z;
  
  switch(type)
  {
    case GlowEffect::LEVEL_UP_OA_GLOW:
	{
      effect_center.y += usec / 2500000.0;
	  break;
	}
	default:
	{
      effect_center.y = pos->y;
	  break;
	}
  }
  
  shift = effect_center - last_effect_center;

  return true;
}

void GlowParticle::draw(const Uint64 usec)
{
  Particle::draw(usec);
}

///////////////////////////////////////////////////////////////////////////////

};

