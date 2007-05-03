#ifdef EYE_CANDY

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_impact.h"

namespace ec
{

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

ImpactParticle::ImpactParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD, const ImpactEffect::ImpactType _type) : Particle(_effect, _mover, _pos, _velocity)
{
  type = _type;
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  texture = _texture;
  size = _size * (0.3 + randcoord()) * 15 / 3.16 * invsqrt(_LOD);
  alpha = _alpha;
  velocity /= size;
  flare_max = 1.0;
  flare_exp = 0.1;
  flare_frequency = 50.0;
  LOD = _LOD;
  state = 0;
}

bool ImpactParticle::idle(const Uint64 delta_t)
{
  const float float_time = delta_t / 1000000.0;
  switch(type)
  {
    case ImpactEffect::MAGIC_PROTECTION:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 4.0);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
    case ImpactEffect::SHIELD:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 4.0);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
    case ImpactEffect::MAGIC_IMMUNITY:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 2.0);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
    case ImpactEffect::POISON:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 1.4);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
    case ImpactEffect::BLOOD:
    {
      const alpha_t scalar = 1.0 - math_cache.powf_0_1_rough_close(randfloat(), float_time * 0.8);
      alpha -= scalar;
      if (alpha < 0.02)
        return false;
      break;
    }
  }
  
  return true;
}

void ImpactParticle::draw(const Uint64 usec)
{
  if (state == 1)
  {
    glEnable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Vec3 normal;
    normal.randomize();
    normal.normalize();
    glNormal3f(normal.x, normal.y, normal.z);
  }

  Particle::draw(usec);

  if (state == 1)
  {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_LIGHTING);
  }
}

GLuint ImpactParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

ImpactEffect::ImpactEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const Vec3 _angle, const ImpactType _type, const Uint16 _LOD, const float _strength)
{
  if (EC_DEBUG)
    std::cout << "ImpactEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  center = *pos;
  angle = _angle;
  type = _type;
  LOD = base->last_forced_LOD;
  desired_LOD = _LOD;
  spawner = NULL;
  mover = NULL;
  strength = _strength;
  const coord_t size_scalar = strength * 1.3;
  const coord_t vel_scalar = sqrt(strength) * 0.44;
  
  switch(type)
  {
    case MAGIC_PROTECTION:
    {
      angle.normalize(2.0 * vel_scalar);
      mover = new ParticleMover(this);
      for (int i = 0; i < 50 * LOD ; i++)
      {
        Vec3 coords = center;
        Vec3 velocity = -angle;
        Vec3 offset;
        offset.randomize(0.3);
        velocity += offset;
        Particle * p = new ImpactParticle(this, mover, coords, velocity, 0.3 * size_scalar, 1.0, 0.7, 0.2, 0.4, &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case SHIELD:
    {
      angle.normalize(2.0 * vel_scalar);
      mover = new ParticleMover(this);
      for (int i = 0; i < 50 * LOD ; i++)
      {
        Vec3 coords = center;
        Vec3 velocity = -angle;
        Vec3 offset;
        offset.randomize(0.3);
        velocity += offset;
        Particle * p = new ImpactParticle(this, mover, coords, velocity, 0.3 * size_scalar, 1.0, 0.9, 0.9, 0.9, &(base->TexShimmer), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case MAGIC_IMMUNITY:
    {
      angle.normalize(2.5 * vel_scalar);
      mover = new ParticleMover(this);
      for (int i = 0; i < 50 * LOD ; i++)
      {
        Vec3 coords = center;
        Vec3 velocity = -angle;
        Vec3 offset;
        offset.randomize(0.4);
        velocity += offset;
        Particle * p = new ImpactParticle(this, mover, coords, velocity, 0.35 * size_scalar, 1.0, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexVoid), LOD, type);
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case POISON:
    {
      angle.normalize(1.0 * vel_scalar);
      mover = new ParticleMover(this);
      for (int i = 0; i < 50 * LOD ; i++)
      {
        Vec3 coords = center;
        Vec3 velocity = -angle;
        Vec3 offset;
        offset.randomize(0.7);
        velocity += offset;
        Particle* p;
        if (randfloat() < 0.4)
        {
          p = new ImpactParticle(this, mover, coords, velocity, 0.6 * size_scalar, 0.5, 0.2 + randcolor(0.2), 0.5 + randcolor(0.3), 0.2, &(base->TexFlare), LOD, type);
          p->state = 1;
        }
        else
        {
          p = new ImpactParticle(this, mover, coords, velocity, 0.3 * size_scalar, 1.0, randcolor(0.1), 0.2 + randcolor(0.1), 0.2, &(base->TexWater), LOD, type);
          p->state = 0;
        }
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
    case BLOOD:	
    {
      angle.normalize(0.8 * vel_scalar);
      mover = new SimpleGravityMover(this);
      for (int i = 0; i < 20 * LOD ; i++)
      {
        Vec3 coords = center;
        Vec3 velocity = -angle;
        Vec3 offset;
        offset.randomize(0.7);
        velocity += offset;
        velocity.normalize(0.8 * vel_scalar);
        Particle * p = new ImpactParticle(this, mover, coords, velocity, square(square(randcoord(0.85))) * size_scalar, 0.5, 0.3 + randcolor(0.7), 0.15 + randcolor(0.1), 0.15 + randcolor(0.1), &(base->TexWater), LOD, type);
        p->state = 1;
        if (!base->push_back_particle(p))
          break;
      }
      break;
    }
  }
}

ImpactEffect::~ImpactEffect()
{
  if (spawner)
    delete spawner;
  if (mover)
    delete mover;
  if (EC_DEBUG)
    std::cout << "ImpactEffect (" << this << ") destroyed." << std::endl;
}

bool ImpactEffect::idle(const Uint64 usec)
{
  if (particles.size() == 0)
    return false;
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef EYE_CANDY
