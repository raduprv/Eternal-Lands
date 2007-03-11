#ifdef SFX

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_sword.h"

namespace ec
{

extern MathCache_Lorange math_cache;

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

SwordParticle::SwordParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD) : Particle(_effect, _mover, _pos, _velocity)
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

bool SwordParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  if (alpha < 0.03)
    return false;

  const alpha_t scalar = math_cache.powf_05_close((float)delta_t / 200000);
  alpha *= scalar;
  
  return true;
}

GLuint SwordParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

SwordEffect::SwordEffect(EyeCandy* _base, bool* _dead, Vec3* _start, Vec3* _end, const SwordType _type, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "SwordEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _start;
  start = _start;
  end = _end;
  type = _type;
  mover = new ParticleMover(this);
  
  switch(type)
  {
    case SERPENT:
    {
      color[0] = 0.6;
      color[1] = 0.8;
      color[2] = 0.3;
      texture = &(base->TexVoid);
      break;
    }
    case CUTLASS:
    {
      color[0] = 1.0;
      color[1] = 1.0;
      color[2] = 1.0;
      texture = &(base->TexInverse);
      break;
    }
    case EMERALD_CLAYMORE:
    {
      color[0] = 0.3;
      color[1] = 1.0;
      color[2] = 0.3;
      texture = &(base->TexCrystal);
      break;
    }
    case SUNBREAKER:
    {
      color[0] = 1.0;
      color[1] = 0.8;
      color[2] = 0.3;
      texture = &(base->TexVoid);
      break;
    }
    case ORC_SLAYER:
    {
      color[0] = 1.0;
      color[1] = 0.1;
      color[2] = 0.1;
      texture = &(base->TexWater);
      break;
    }
    case EAGLE_WING:
    {
      color[0] = 0.7;
      color[1] = 1.0;
      color[2] = 1.0;
      texture = &(base->TexInverse);
      break;
    }
    case JAGGED_SABER:
    {
      color[0] = 1.0;
      color[1] = 0.3;
      color[2] = 1.0;
      texture = &(base->TexTwinflare);
      break;
    }
    case SWORD_OF_FIRE:
    {
      color[0] = 1.0;
      color[1] = 0.6;
      color[2] = 0.3;
      texture = &(base->TexFlare);
      break;
    }
    case SWORD_OF_ICE:
    {
      color[0] = 0.4;
      color[1] = 0.5;
      color[2] = 1.0;
      texture = &(base->TexCrystal);
      break;
    }
    case SWORD_OF_MAGIC:
    {
      color[0] = 0.5;
      color[1] = 0.4;
      color[2] = 0.8;
      texture = &(base->TexShimmer);
      break;
    }
  }
  
  old_end = *end;
  desired_LOD = _LOD;
  request_LOD(LOD);
}

SwordEffect::~SwordEffect()
{
  delete mover;
  if (EC_DEBUG)
    std::cout << "SwordEffect (" << this << ") destroyed." << std::endl;
}

void SwordEffect::request_LOD(const Uint16 _LOD)
{
  if (_LOD <= desired_LOD)
    LOD = _LOD;
  else
    LOD = desired_LOD;
  switch(type)
  {
    case SERPENT:
    {
      alpha = 0.8;
      size = 1.1;
      break;
    }
    case CUTLASS:
    {
      alpha = 0.15;
      size = 2.0;
      break;
    }
    case EMERALD_CLAYMORE:
    {
      alpha = 0.8;
      size = 0.75;
      break;
    }
    case SUNBREAKER:
    {
      alpha = 1.0;
      size = 1.1;
      break;
    }
    case ORC_SLAYER:
    {
      alpha = 0.2;
      size = 1.3;
      break;
    }
    case EAGLE_WING:
    {
      alpha = 0.4;
      size = 2.25;
      break;
    }
    case JAGGED_SABER:
    {
      alpha = 0.8;
      size = 2.25;
      break;
    }
    case SWORD_OF_FIRE:
    {
      alpha = 1.0;
      size = 1.2;
      break;
    }
    case SWORD_OF_ICE:
    {
      alpha = 1.0;
      size = 1.25;
      break;
    }
    case SWORD_OF_MAGIC:
    {
      alpha = 1.0;
      size = 1.4;
      break;
    }
  }
  
  size *= 13.0 / (LOD + 3);
  alpha /= 13.0 / (LOD + 3);
}

bool SwordEffect::idle(const Uint64 usec)
{
  if ((recall) && (particles.size() == 0))
    return false;
    
  if (recall)
    return true;

  const Vec3 pos_change = old_end - *end;
  float speed = square(pos_change.magnitude() * 1000000.0 / usec);
  if (speed > 4.0)
    speed = 4.0;
  else if (speed < 0.1)
    speed = 0.1;
  
  while (math_cache.powf_0_1_rough_close(randfloat(), (float)usec / 12000 * speed) < 0.5)
  {
    const percent_t percent = square(randpercent());
    const Vec3 coords = (*start * percent) + (*end * (1.0 - percent));
    Vec3 velocity;
    velocity.randomize(0.2);
    Particle* p = new SwordParticle(this, mover, coords, velocity, size, alpha, color[0], color[1], color[2], texture, LOD);
    if (!base->push_back_particle(p))
      break;
  }
  
  old_end = *end;

  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef SFX
