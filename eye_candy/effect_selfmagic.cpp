#ifdef EYE_CANDY

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_selfmagic.h"

namespace ec
{

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

SelfMagicParticle::SelfMagicParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD, const SelfMagicEffect::SelfMagicType _type) : Particle(_effect, _mover, _pos, _velocity)
{
  type = _type;
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  texture = _texture;
  size = _size * (0.5 + randcoord()) * 13 / (_LOD + 3);
  alpha = _alpha;
  velocity /= size;
  flare_max = 5.0;
  flare_exp = 0.1;
  flare_frequency = 3.0;
  LOD = _LOD;
  state = 0;
}

bool SelfMagicParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;
  
  switch(type)
  {
    case SelfMagicEffect::HEAL:
    {
      const interval_t float_time = delta_t / 1000000.0;

      if (state == 0)
      {
        if ((get_time() - born > 500000) || (math_cache.powf_0_1_rough_close(randfloat(), float_time * 5.0)) < 0.5)
          state = 1;
      }
      else
      {
        if (alpha < 0.008)
          return false;

        const float scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 15.0);
        energy *= scalar;
        if (size < 10)
          size /= scalar;
        alpha *= scalar;
      }
      break;
    }
    case SelfMagicEffect::MAGIC_PROTECTION:
    {
      const interval_t float_time = delta_t / 1000000.0;

      if (state == 0)
      {
        if ((get_time() - born > 700000) || (math_cache.powf_0_1_rough_close(randfloat(), float_time * 3.0)) < 0.5)
          state = 1;
      }
      else
      {
        if (alpha < 0.02)
          return false;
          
        const float scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 7.0);
        energy *= scalar;
        alpha *= scalar;
      }
      break;
    }
    case SelfMagicEffect::SHIELD:
    {
      const interval_t float_time = delta_t / 1000000.0;

      if (state == 0)
      {
        if ((get_time() - born > 700000) || (math_cache.powf_0_1_rough_close(randfloat(), float_time * 3.0)) < 0.5)
          state = 1;
      }
      else
      {
        if (alpha < 0.02)
          return false;
        
        const float scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 4.5);
        if (size < 10)
          size /= scalar;
        alpha *= square(scalar);
      }
      break;
    }
    case SelfMagicEffect::RESTORATION:
    {
      const interval_t float_time = delta_t / 1000000.0;

      if (state == 0)
      {
        if ((get_time() - born > 700000) || (math_cache.powf_0_1_rough_close(randfloat(), float_time * 3.0)) < 0.5)
          state = 1;
      }
      else
      {
        if (alpha < 0.02)
          return false;

        const float scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 4.5);
        if (size < 10)
          size /= scalar;
        alpha *= square(scalar);
      }
      break;
    }
    case SelfMagicEffect::BONES_TO_GOLD:
    {
      const interval_t float_time = delta_t / 1000000.0;

      if (state == 0)
      {
        if ((get_time() - born > 500000) || (math_cache.powf_0_1_rough_close(randfloat(), float_time * 6.0)) < 0.5)
        {
          for (int i = 0; i < 5; i++)
          {
            Vec3 new_velocity;
            new_velocity.randomize(1.5);
            new_velocity += velocity;
            Particle * p = new SelfMagicParticle(effect, mover, pos, new_velocity, 2.0, 1.0, 1.0, 1.0, 0.5, &(base->TexShimmer), LOD, type);
            p->state = 1;
            if (!base->push_back_particle(p))
              break;
          }
          return false;
        }
      }
      else
      {
        if (alpha < 0.02)
          return false;

        const float scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 10.0);
        energy *= scalar;
        if (size < 10)
          size /= scalar;
        alpha *= scalar;
      }
      break;
    }
    case SelfMagicEffect::TELEPORT_TO_THE_PORTALS_ROOM:
    {
      if (alpha < 0.005)
        return false;

      const alpha_t scalar = math_cache.powf_05_close((float)delta_t / 500000);
      alpha *= scalar;
  
      break;
    }
    case SelfMagicEffect::MAGIC_IMMUNITY:
    {
      const interval_t float_time = delta_t / 1000000.0;

      if (state == 0)
      {
        if ((get_time() - born > 700000) || (math_cache.powf_0_1_rough_close(randfloat(), float_time * 3.0)) < 0.5)
          state = 1;
      }
      else
      {
        if (alpha < 0.02)
          return false;
          
        const float scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 7.0);
        alpha *= scalar;
      }
      break;
    }
    case SelfMagicEffect::ALERT:
    {
      alpha -= delta_t / 5000000.0;

      if (alpha < 0.01)
        return false;
  
      break;
    }
  }
  
  pos += ((SelfMagicEffect*)effect)->shift;
  
  return true;
}

GLuint SelfMagicParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

SelfMagicEffect::SelfMagicEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const SelfMagicType _type, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "SelfMagicEffect (" << this << ") created (" << _type << ")." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  effect_center = *pos;
  type = _type;
  LOD = base->last_forced_LOD;
  desired_LOD = _LOD;
  spawner = NULL;
  mover = NULL;
  spawner2 = NULL;
  mover2 = NULL;
  target_alpha = NULL;
  shift = Vec3(0.0, 0.0, 0.0);
  
  switch(type)
  {
    case HEAL:
    {
      effect_center.y += 0.2;
      spawner = new SierpinskiIFSParticleSpawner();
      mover = new GravityMover(this, &effect_center, 1e10);
      while ((int)particles.size() < LOD * 100)
      {
        Vec3 coords = spawner->get_new_coords() * 0.5;
        Vec3 velocity = -coords * 3;
        coords += effect_center;
        Particle * p = new SelfMagicParticle(this, mover, coords, velocity, 0.7, 0.5, 0.4, 0.7, 0.2, &(base->TexFlare), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case MAGIC_PROTECTION:
    {
      effect_center.y += 0.4;
      spawner = new HollowSphereSpawner(0.7);
      mover = new GravityMover(this, &effect_center, 3e10);
      while ((int)particles.size() < LOD * 100)
      {
        Vec3 coords = spawner->get_new_coords() + effect_center;
        Vec3 velocity;
        velocity.randomize(0.4);
        Particle * p = new SelfMagicParticle(this, mover, coords, velocity, 1.1, 1.0, 0.7, 0.2, 0.4, &(base->TexVoid), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case SHIELD:
    {
      effect_center.y += 0.2;
      spawner = new HollowDiscSpawner(0.45);
      mover = new SpiralMover(this, &effect_center, 15.0, 14.0);

      while ((int)particles.size() < LOD * 100)
      {
        Vec3 coords = spawner->get_new_coords() + effect_center;
        Vec3 velocity;
        velocity.randomize(0.3);
        velocity.y *= 5;
        velocity.y += 0.7;
        Particle * p = new SelfMagicParticle(this, mover, coords, velocity, 2.0, 1.0, 0.5, 0.5, 0.6, &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case RESTORATION:
    {
      effect_center.y += 0.2;
      spawner = new FilledSphereSpawner(0.8);
      mover = new GravityMover(this, &effect_center, 3e10);
      spawner2 = new HollowDiscSpawner(0.45);
      mover2 = new SpiralMover(this, &effect_center, 10.0, 11.0);
      while ((int)particles.size() < LOD * 60)
      {
        Vec3 coords = spawner->get_new_coords() * 2.5;
        Vec3 velocity = -coords * 3;
        coords += effect_center;
        Particle * p = new SelfMagicParticle(this, mover, coords, velocity, 1.9 + randcoord(1.5), 0.85 + randalpha(0.15), 0.25 + randcolor(0.3), 0.7 + randcolor(0.2), 0.3, &(base->TexFlare), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      while ((int)particles.size() < LOD * 100)
      {
        Vec3 coords = spawner2->get_new_coords() + effect_center;
        coords.y += 1.0;
        Vec3 velocity;
        velocity.randomize(0.3);
        velocity.y *= 5;
        velocity.y -= 0.7;
        Particle * p = new SelfMagicParticle(this, mover2, coords, velocity, 1.0 + randcoord(1.5), 0.75 + randalpha(0.25), 0.6 + randcolor(0.3), 0.35 + randcolor(0.45), 0.3, &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case BONES_TO_GOLD:
    {
      count = 0;
      count_scalar = 100000 / _LOD;
      spawner = new FilledDiscSpawner(0.45);
      mover = new SpiralMover(this, &effect_center, 13.0, 12.0);
      Vec3 coords = spawner->get_new_coords() + effect_center;
      Vec3 velocity;
      velocity.randomize(0.4);
      velocity.y += 2.8;
      Particle * p = new SelfMagicParticle(this, mover, coords, velocity, 2.0, 1.0, 0.6, 0.5, 0.5, &(base->TexInverse), LOD, type);
      base->push_back_particle(p);
      break;
    }
    case TELEPORT_TO_THE_PORTALS_ROOM:
    {
      mover = new ParticleMover(this);
      spawner = new FilledDiscSpawner(0.2);
      const float sqrt_LOD = fastsqrt(LOD);
      size_scalar = 1.0;
      for (int i = 0; i < LOD * 100; i++)
      {
        const Vec3 coords = spawner->get_new_coords() + effect_center + Vec3(0.0, randcoord() * randcoord() * 8.0 * sqrt_LOD, 0.0);
        Vec3 velocity(0.0, randcoord(0.1), 0.0);
        velocity.randomize(0.2);
        const coord_t size = size_scalar * (0.5 + 1.5 * randcoord());
        velocity /= size;
        Particle* p = new SelfMagicParticle(this, mover, coords, velocity, size, 1.0, 0.8 + randcolor(0.2),  0.8 + randcolor(0.2), 0.8 + randcolor(0.2), &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      
//      const float radius = 0.5 * powf(2, 0.18) / 1.5;
      const float radius = 0.377628;
      for (int i = 0; i < LOD * 4; i++)
      {
        const percent_t percent = ((percent_t)i + 1) / (LOD * 4);
        capless_cylinders.push_back(new CaplessCylinder(base, effect_center, effect_center + Vec3(0.0, 10.0 / percent, 0.0), Vec3(1.0, 1.0, 1.0), (0.1 + (1.0 - percent) * 0.05) / (LOD + 2), radius * percent, (int)(25 * (percent + 0.2))));
      }

      break;
    }
    case MAGIC_IMMUNITY:
    {
      effect_center.y += 0.2;
      spawner = new FilledSphereSpawner(1.0);
      mover = new GravityMover(this, &effect_center, 4e10);
      while ((int)particles.size() < LOD * 20)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity;
        velocity.randomize(2.0);
        coords += effect_center;
        coords.y += 0.3;
        Particle * p = new SelfMagicParticle(this, mover, coords, velocity, randcoord(7.0) + 0.3, 1.0, randcolor(), randcolor(), randcolor(), &(base->TexVoid), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case ALERT:
    {
      effect_center.y += 0.5;
      spawner = new HollowEllipsoidSpawner(Vec3(0.2, 0.5, 0.2));
      mover = new ParticleMover(this);
      while ((int)particles.size() < LOD * 20)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity = coords * 16;
        coords += effect_center;
        Particle * p = new SelfMagicParticle(this, mover, coords, velocity, 7.0, 0.12, 1.0, 1.0, 1.0, &(base->TexVoid), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
  }
}

SelfMagicEffect::~SelfMagicEffect()
{
  if (spawner)
    delete spawner;
  if (mover)
    delete mover;
  if (spawner2)
    delete spawner2;
  if (mover2)
    delete mover2;
  for (size_t i = 0; i < capless_cylinders.size(); i++)
    delete capless_cylinders[i];
  capless_cylinders.clear();
  if (EC_DEBUG)
    std::cout << "SelfMagicEffect (" << this << ") destroyed." << std::endl;
}

bool SelfMagicEffect::idle(const Uint64 usec)
{
  if (particles.size() == 0)
  {
    if (capless_cylinders.size())
    {
      if ((*capless_cylinders.rbegin())->alpha < 0.01)
        return false;
    }
    else
      return false;
  }
  else if (recall)
    return true;
    
  const Vec3 last_effect_center = effect_center;
    
  effect_center.x = pos->x;
  effect_center.z = pos->z;
  
  shift = effect_center - last_effect_center;

  effect_center.y += usec / 1500000.0;
  
  const Uint64 cur_time = get_time();
  const Uint64 age = cur_time - born;
  switch(type)
  {
    case HEAL:
    {
      break;
    }
    case MAGIC_PROTECTION:
    {
      break;
    }
    case SHIELD:
    {
      break;
    }
    case RESTORATION:
    {
      break;
    }
    case BONES_TO_GOLD:
    {
      if (age < 500000)
      {
        count += usec;
    
        while (count > 0)
        {
          Vec3 coords = spawner->get_new_coords() + effect_center;
          Vec3 velocity;
          velocity.randomize(0.4);
          velocity.y += 2.8;
          Particle * p = new SelfMagicParticle(this, mover, coords, velocity, 2.0, 1.0, 0.6, 0.5, 0.5, &(base->TexInverse), LOD, type);
          if (!base->push_back_particle(p))
          {
            count = 0;
            break;
          }
          count -= count_scalar;
        }
      }
      break;
    }
    case TELEPORT_TO_THE_PORTALS_ROOM:
    {
      if (age > 500000)
      {
        const alpha_t scalar = math_cache.powf_05_close((interval_t)usec / 200000.0);
        for (int i = 0; i < (int)capless_cylinders.size(); i++)
        {
          float percent = float(i) / float(capless_cylinders.size()) * 0.8; 
          std::vector<Shape*>::const_iterator iter = capless_cylinders.begin() + i;
          (*iter)->alpha = (*iter)->alpha * percent + (*iter)->alpha * scalar * (1.0 - percent);
        }
/*
        for (std::vector<Shape*>::iterator iter = capless_cylinders.begin(); iter != capless_cylinders.end(); iter++)
        {
          (*iter)->alpha *= scalar;
//          std::coutk << (*iter)->alpha << scalar << ((*iter)->alpha * scalar) << std::endl;
          if ((*iter)->alpha > 0.005)
            break;
        }
*/
      }
      
      if (target_alpha)
      {
        if (age < 500000)
        {
          *target_alpha = 1.0 - (age / 500000.0);
        }
        else if (age < 1000000)
        {
          *target_alpha = (age - 500000) / 500000.0;
        }
        else
        {
          *target_alpha = 1.0;
        }
      }
      
      break;
    }
    case MAGIC_IMMUNITY:
    {
      break;
    }
    case ALERT:
    {
      break;
    }
  }
  
  return true;
}

void SelfMagicEffect::draw(const Uint64 usec)
{
  for (std::vector<Shape*>::iterator iter = capless_cylinders.begin(); iter != capless_cylinders.end(); iter++)
    (*iter)->draw();
}


///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef EYE_CANDY
