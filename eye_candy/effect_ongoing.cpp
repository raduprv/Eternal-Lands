#ifdef EYE_CANDY

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_ongoing.h"

namespace ec
{

extern MathCache_Lorange math_cache;

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

OngoingParticle::OngoingParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD, const OngoingEffect::OngoingType _type) : Particle(_effect, _mover, _pos, _velocity)
{
  type = _type;
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  texture = _texture;
  size = _size * (0.3 + randcoord()) * 15 / _LOD;
  alpha = _alpha;
  velocity /= size;
  flare_max = 1.0;
  flare_exp = 0.1;
  flare_frequency = 50.0;
  LOD = _LOD;
  state = 0;
}

bool OngoingParticle::idle(const Uint64 delta_t)
{
  const interval_t float_time = delta_t / 1000000.0;
  switch(type)
  {
    case OngoingEffect::MAGIC_PROTECTION:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 1.0);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
    case OngoingEffect::SHIELD:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 1.0);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
    case OngoingEffect::MAGIC_IMMUNITY:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 0.5);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
    case OngoingEffect::POISON:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 0.5);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
  }
  
  return true;
}

void OngoingParticle::draw(const Uint64 usec)
{
  if ((type == OngoingEffect::POISON) && (state == 1))
  {
    glEnable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Vec3 normal;
    normal.randomize();
    normal.normalize();
    glNormal3f(normal.x, normal.y, normal.z);
  }
  Particle::draw(usec);

  if ((type == OngoingEffect::POISON) && (state == 1))
  {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_LIGHTING);
  }
}

GLuint OngoingParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

OngoingEffect::OngoingEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const OngoingType _type, const Uint16 _LOD, const float _strength)
{
  if (EC_DEBUG)
    std::cout << "OngoingEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  effect_center = *pos;
  effect_center.y += 0.5;
  type = _type;
  LOD = _LOD;
  desired_LOD = _LOD;
  spawner = NULL;
  mover = NULL;
  strength = _strength;
  
  switch(type)
  {
    case MAGIC_PROTECTION:
    {
      spawner = new HollowEllipsoidSpawner(Vec3(0.45, 0.9, 0.45));
      mover = new ParticleMover(this);
      break;
    }
    case SHIELD:
    {
      spawner = new HollowDiscSpawner(0.6);
      mover = new SpiralMover(this, &effect_center, 3.0, 2.9);
      break;
    }
    case MAGIC_IMMUNITY:
    {
      spawner = new HollowEllipsoidSpawner(Vec3(0.6, 1.1, 0.6));
      mover = new ParticleMover(this);
      break;
    }
    case POISON:
    {
      spawner = new HollowDiscSpawner(0.13);
      mover = new SpiralMover(this, &effect_center, -0.9, 0.8);
      break;
    }
  }
}

OngoingEffect::~OngoingEffect()
{
  if (spawner)
    delete spawner;
  if (mover)
    delete mover;
  if (EC_DEBUG)
    std::cout << "OngoingEffect (" << this << ") destroyed." << std::endl;
}

bool OngoingEffect::idle(const Uint64 usec)
{
  if ((recall) && (particles.size() == 0))
    return false;
    
  if (recall)
    return true;
  
  effect_center = *pos;
  effect_center.y += 0.5;
    
  const interval_t float_time = usec / 1000000.0;
  switch(type)
  {
    case MAGIC_PROTECTION:
    {
      while (math_cache.powf_0_1_rough_close(randfloat(), float_time * 6.0 * LOD * strength) < 0.5)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity = coords / 12.0;
        coords += effect_center;
        Particle * p = new OngoingParticle(this, mover, coords, velocity, 0.5, 1.0, 0.7, 0.2, 0.4, &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case SHIELD:
    {
      while (math_cache.powf_0_1_rough_close(randfloat(), float_time * 6.0 * LOD * strength) < 0.5)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity(0.0, 0.0, 0.0);
        coords += effect_center;
        coords.y = -0.2 + randcoord(1.5);
        Particle * p = new OngoingParticle(this, mover, coords, velocity, 0.5, 1.0, 0.9, 0.9, 0.9, &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case MAGIC_IMMUNITY:
    {
      while (math_cache.powf_0_1_rough_close(randfloat(), float_time * 6.0 * LOD * strength) < 0.5)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity = coords / 18.0;
        coords += effect_center;
        Particle * p = new OngoingParticle(this, mover, coords, velocity, 0.6, 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexVoid), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case POISON :	//The odd one out.  ;)
    {
      while (math_cache.powf_0_1_rough_close(randfloat(), float_time * 4.0 * LOD * strength) < 0.5)
      {
        Vec3 coords = spawner->get_new_coords();
        Vec3 velocity;
        velocity.randomize(0.13);
        velocity.y += 0.08;
        coords += effect_center;
        coords.y = 0.3 + randcoord(0.8);
        Particle* p;
        if (randfloat() < 0.4)
        {
          p = new OngoingParticle(this, mover, coords, velocity, 1.15, 0.5, 0.2 + randcolor(0.2), 0.5 + randcolor(0.3), 0.2, &(base->TexVoid), LOD, type);
          p->state = 1;
        }
        else
        {
          p = new OngoingParticle(this, mover, coords, velocity, 0.65, 1.0, randcolor(0.1), 0.2 + randcolor(0.1), 0.2, &(base->TexWater), LOD, type);
          p->state = 0;
        }
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
  }
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef EYE_CANDY
