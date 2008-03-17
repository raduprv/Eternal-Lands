#ifdef MISSILES

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_missile.h"

namespace ec
{

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

MissileParticle::MissileParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD) : Particle(_effect, _mover, _pos, _velocity)
{
  color[0] = red + randcolor(0.2) - 0.1;
  if (color[0] > 1.0)
    color[0] = 1.0;
  else if (color[0] < 0.0)
    color[0] = 0.0;
  color[1] = green + randcolor(0.2) - 0.1;
  if (color[1] > 1.0)
    color[1] = 1.0;
  else if (color[1] < 0.0)
    color[1] = 0.0;
  color[2] = blue + randcolor(0.2) - 0.1;
  if (color[2] > 1.0)
    color[2] = 1.0;
  else if (color[2] < 0.0)
    color[2] = 0.0;
  texture = _texture;
  size = _size * (0.2 + randcoord());
  alpha = _alpha;
  velocity /= size;
  flare_max = 1.6;
  flare_exp = 0.2;
  flare_frequency = 2.0;
  LOD = _LOD;
}

bool MissileParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  if (alpha < 0.01)
    return false;

  //const alpha_t scalar = math_cache.powf_05_close((float)delta_t / 20000);
  alpha *= math_cache.powf_0_1_rough_close(randfloat(), delta_t / 2000000.0);
  
  return true;
}

GLuint MissileParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

MissileEffect::MissileEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const MissileType _type, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "MissileEffect (" << this << ") created (" << _type << ")." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  type = _type;
  bounds = NULL;
  mover = new ParticleMover(this);
  
  switch(type)
  {
    case MAGIC:
    {
      color[0] = 0.7;
      color[1] = 0.6;
      color[2] = 0.4;
      texture = &(base->TexShimmer);
      break;
    }
    case FIRE:
    {
      color[0] = 1.0;
      color[1] = 0.6;
      color[2] = 0.3;
      texture = &(base->TexFlare);
      break;
    }
    case ICE:
    {
      color[0] = 0.4;
      color[1] = 0.5;
      color[2] = 1.0;
      texture = &(base->TexCrystal);
      break;
    }
    case EXPLOSIVE:
    {
      color[0] = 0.8;
      color[1] = 0.8;
      color[2] = 0.8;
      texture = &(base->TexInverse);
      break;
    }
  }
  
  old_pos = *pos;
  LOD = 100;
  desired_LOD = _LOD;
  request_LOD((float)base->last_forced_LOD);
  
  Vec3 randshift;
  randshift.randomize(0.1);
  const Vec3 coords = *pos + randshift;
  Vec3 velocity;
  velocity.randomize(0.25);
  Particle* p = new MissileParticle(this, mover, coords, velocity, randfloat(size), randfloat(alpha), color[0], color[1], color[2], texture, LOD);
  base->push_back_particle(p);
}

MissileEffect::~MissileEffect()
{
  if (mover)
    delete mover;
  if (EC_DEBUG)
    std::cout << "MissileEffect (" << this << ") destroyed." << std::endl;
}

void MissileEffect::request_LOD(const float _LOD)
{
  if (fabs(_LOD - (float)LOD) < 1.0)
    return;

  const Uint16 rounded_LOD = (Uint16)round(_LOD);
  if (rounded_LOD <= desired_LOD)
    LOD = rounded_LOD;
  else
    LOD = desired_LOD;
  switch(type)
  {
    case MAGIC:
    {
      alpha = 1.0;
      size = 1.4;
      break;
    }
    case FIRE:
    {
      alpha = 1.0;
      size = 1.2;
      break;
    }
    case ICE:
    {
      alpha = 1.0;
      size = 1.25;
      break;
    }
    case EXPLOSIVE:
    {
      alpha = 0.3;
      size = 1.4;
      break;
    }
  }
  
  size *= 40.0 / (LOD + 17);
  alpha /= 13.0 / (LOD + 3);
}

bool MissileEffect::idle(const Uint64 usec)
{
  
  if (particles.size() == 0)
    return false;

  const float dist = (old_pos - *pos).magnitude();
  
  if (dist < 1E-4) return true; // do not add more particles, dist < 0.0001

  for (float step = 0.0; step < dist; step += 0.025) {
	  const percent_t percent = step / dist;
	  Vec3 randshift;
	  randshift.randomize(0.05);
	  const Vec3 coords = (old_pos * percent) + (*pos * (1.0 - percent)) + randshift;
	  Vec3 velocity;
	  velocity.randomize(0.25);
	  Particle* p = new MissileParticle(this, mover, coords, velocity, randfloat(size*2.0), randfloat(alpha), color[0], color[1], color[2], texture, LOD);
	  base->push_back_particle(p);
  }
  
  old_pos = *pos;

  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif // MISSILES
