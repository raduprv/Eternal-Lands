#ifdef SFX

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_harvesting.h"

namespace ec
{

extern MathCache_Lorange math_cache;

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

HarvestingParticle::HarvestingParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD, const HarvestingEffect::HarvestingType _type) : Particle(_effect, _mover, _pos, _velocity)
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

bool HarvestingParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;
    
  const interval_t float_time = delta_t / 1000000.0;
  const Uint64 age = get_time() - born;
  switch(type)
  {
    case HarvestingEffect::RADON_POUCH:
    {
      if (alpha < 0.01)
        return false;

      const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 3);
      alpha *= scalar;

      break;
    }
    case HarvestingEffect::CAVERN_WALL:
    {
      if (pos.y < -2.0)
        return false;

      break;
    }
    case HarvestingEffect::MOTHER_NATURE:
    {
      if (alpha < 0.01)
        return false;

      const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 5);
      if (size < 10)
        size /= scalar;
      alpha *= scalar;

      break;
    }
    case HarvestingEffect::QUEEN_OF_NATURE:
    {
      if (alpha < 0.01)
        return false;

      const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time);
      alpha *= scalar;

      break;
    }
    case HarvestingEffect::BEES:
    {
      const Uint64 age = get_time() - born;
      if (age < 650000)
        break;
    
      if (alpha < 0.03)
        return false;

      const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 6);
      alpha *= scalar;

      break;
    }
    case HarvestingEffect::BAG_OF_GOLD:
    {
      if (age > 220000)
      {
        const alpha_t alpha_scalar = math_cache.powf_05_close(float_time * 6.0);
        alpha *= alpha_scalar;
    
        if (alpha < 0.02)
          return false;
      }
      
      break;
    }
    case HarvestingEffect::RARE_STONE:
    {
      color[0] = 0.7 + 0.3 * sin(age / 530000);
      color[1] = 0.7 + 0.3 * sin(age / 970000 + 1.3);
      color[2] = 0.7 + 0.3 * sin(age / 780000 + 1.9);
    
      if (age < 700000)
      {
        if (state == 0)
          size = age / 45000.0;
      }
      else
      {
        const percent_t scalar = math_cache.powf_05_close(float_time * 1);
        size *= scalar;
        alpha *= scalar;
    
        if (alpha < 0.02)
          return false;
      }
      break;
    }
  }
  
  return true;
}

void HarvestingParticle::draw(const Uint64 usec)
{
  if (((type == HarvestingEffect::RADON_POUCH) && (state == 0)) || (type == HarvestingEffect::MOTHER_NATURE) || (type == HarvestingEffect::QUEEN_OF_NATURE) || ((type == HarvestingEffect::BAG_OF_GOLD) && (state == 0)) || (type == HarvestingEffect::RARE_STONE))
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

GLuint HarvestingParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

light_t HarvestingParticle::get_light_level()
{
  if (((type == HarvestingEffect::RADON_POUCH) && (state == 0)) || (type == HarvestingEffect::MOTHER_NATURE) || (type == HarvestingEffect::QUEEN_OF_NATURE) || ((type == HarvestingEffect::BAG_OF_GOLD) && (state == 0)) || (type == HarvestingEffect::RARE_STONE))
    return alpha * size / 1500;
  else
    return 0.0;
};

HarvestingEffect::HarvestingEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const HarvestingType _type, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "HarvestingEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  effect_center = *pos;
  type = _type;
  LOD = _LOD;
  desired_LOD = _LOD;
  spawner = NULL;
  mover = NULL;
  spawner2 = NULL;
  mover2 = NULL;
  
  switch(type)
  {
    case RADON_POUCH:
    {
      effect_center.y += 0.5;
      spawner = new FilledSphereSpawner(0.9);
      mover = new GravityMover(this, &effect_center, 8e9);
      while ((int)particles.size() < LOD * 50)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.8);
        coords += effect_center;
        Particle * p = new HarvestingParticle(this, mover, coords, velocity, 5.25, 0.5, 0.6, 0.7, 0.2, &(base->TexFlare), LOD, type);
        p->state = 0;
        if (!base->push_back_particle(p))
          break;
      }
      while ((int)particles.size() < LOD * 100)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(1.5);
        coords += effect_center;
        Particle * p = new HarvestingParticle(this, mover, coords, velocity, 4.5, 0.5 + randalpha(0.4), 0.7, 0.6, 0.5, &(base->TexWater), LOD, type);
        p->state = 1;
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case CAVERN_WALL:
    {
      effect_center.y += 15.0;
      spawner = new FilledSphereSpawner(1.0);
      mover = new ParticleMover(this);
      while ((int)particles.size() < LOD * 50)
      {
        Vec3 coords = spawner->get_new_coords();
        coords.y *= 8.0;
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.2);
        velocity.y *= 3.0;
        velocity.y -= 9.0;
        coords += effect_center;
        const color_t scalar = randcolor(0.4);
        Particle * p = new HarvestingParticle(this, mover, coords, velocity, 8.0 + randcoord(12.0), 1.0, scalar + randcolor(0.1), scalar + randcolor(0.1), scalar + randcolor(0.1), &(base->TexSimple), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      while ((int)particles.size() < LOD * 100)
      {
        Vec3 coords = spawner->get_new_coords();
        coords.y *= 8.0;
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.2);
        velocity.y *= 3.0;
        velocity.y -= 9.0;
        coords += effect_center;
        Particle * p = new HarvestingParticle(this, mover, coords, velocity, 3.0 + randcoord(6.0), 0.4 + randalpha(0.4), 0.2 + randcolor(0.2), 0.2 + randcolor(0.2), 0.2 + randcolor(0.2), &(base->TexWater), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case MOTHER_NATURE:
    {
      effect_center.y += 0.2;
      spawner = new HollowDiscSpawner(0.1);
      mover = new SpiralMover(this, &effect_center, 30.0, 18.0);
      while ((int)particles.size() < LOD * 100)
      {
        Vec3 coords = spawner->get_new_coords() + effect_center;
        Vec3 velocity;
        velocity.randomize(0.5);
        velocity.y *= 3;
        velocity.y += 2.0;
        Particle * p = new HarvestingParticle(this, mover, coords, velocity, 3.75, 0.3, 1.0, 0.5 + randcolor(0.5), 0.5, &(base->TexTwinflare), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case QUEEN_OF_NATURE:
    {
      effect_center.y += 0.2;
      spawner = new FilledDiscSpawner(0.5);
      mover = new ParticleMover(this);
      while ((int)particles.size() < LOD * 100)
      {
        Vec3 coords = spawner->get_new_coords() + effect_center;
        coords.y += (coord_t)(randfloat(2.0) * randfloat(2.0) * randfloat(2.0));
        Vec3 velocity(0.0, 0.0, 0.0);
        Particle * p = new HarvestingParticle(this, mover, coords, velocity, 2.0 + randcoord(1.0), 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case BEES:
    {
      effect_center.y += 0.5;
      spawner = new FilledSphereSpawner(0.5);
      mover = new GravityMover(this, &effect_center, 8e9);
      while ((int)particles.size() < LOD * 50)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity;
        velocity.randomize();
        velocity.normalize(0.9);
        coords += effect_center;
        Particle * p = new HarvestingParticle(this, mover, coords, velocity, 0.75, 1.0, 0.8, 0.7, 0.3, &(base->TexTwinflare), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case BAG_OF_GOLD:
    {
      effect_center.y += 0.5;
      mover = new GravityMover(this, &effect_center, 2e10);
      spawner = new HollowSphereSpawner(0.3);
  
      for (int i = 0; i < LOD * 60; i++)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity = coords / 10.0;
        coords += effect_center;
        Particle* p = new HarvestingParticle(this, mover, coords, velocity, 1.05, 0.75, randcolor(0.3) + 0.7, randcolor(0.3) + 0.5, randcolor(0.3) + 0.3, &(base->TexFlare), LOD, type);
        p->state = 1;
        if (!base->push_back_particle(p))
          break;
      }
      
      Particle* p = new HarvestingParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 8.0, 1.0, 0.8, 0.7, 0.3, &(base->TexShimmer), LOD, type);
      base->push_back_particle(p);
      break;
    }
    case RARE_STONE:
    {
      effect_center.y += 0.5;
      mover = new ParticleMover(this);
      spawner = new HollowSphereSpawner(0.3);
  
      for (int i = 0; i < LOD * 60; i++)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity = coords / 10.0;
        coords += effect_center;
        Particle* p = new HarvestingParticle(this, mover, coords, velocity, 0.75, 0.05, randcolor(0.3) + 0.7, randcolor(0.3) + 0.5, randcolor(0.3) + 0.3, &(base->TexFlare), LOD, type);
        p->state = 1;
        if (!base->push_back_particle(p))
          break;
      }
      
      Particle* p = new HarvestingParticle(this, mover, effect_center, Vec3(0.0, 0.0, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexVoid), LOD, type);
      if (!base->push_back_particle(p))
        break;
      p = new HarvestingParticle(this, mover, effect_center, Vec3(0.0, 0.01, 0.0), 7.5, 1.0, 1.0, 1.0, 1.0, &(base->TexVoid), LOD, type);
      base->push_back_particle(p);
      break;
    }
  }
}

HarvestingEffect::~HarvestingEffect()
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
    std::cout << "HarvestingEffect (" << this << ") destroyed." << std::endl;
}

bool HarvestingEffect::idle(const Uint64 usec)
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

#endif	// #ifdef SFX
