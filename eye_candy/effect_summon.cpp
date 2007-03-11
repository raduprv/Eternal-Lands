#ifdef SFX

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_summon.h"

namespace ec
{

extern MathCache_Lorange math_cache;

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

OuterSummonParticle::OuterSummonParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, const Uint16 _LOD) : Particle(_effect, _mover, _pos, _velocity)
{
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  size = _size * (0.2 + randcoord());
  alpha = _alpha;
  velocity /= size;
  flare_max = 1.6;
  flare_exp = 0.2;
  flare_frequency = 2.0;
  LOD = _LOD;
}

bool OuterSummonParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  if (alpha < 0.03)
    return false;

  const alpha_t scalar = math_cache.powf_05_close((float)delta_t / 100000);
  alpha *= scalar;
  
  return true;
}

GLuint OuterSummonParticle::get_texture(const Uint16 res_index)
{
  return base->TexFlare.get_texture(res_index);
}

InnerSummonParticle::InnerSummonParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD) : Particle(_effect, _mover, _pos, _velocity)
{
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  texture = _texture;
  size = _size * (0.2 + randcoord());
  alpha = _alpha;
  velocity /= size;
  flare_max = 4.0;
  flare_exp = 0.2;
  flare_frequency = 2.0;
  LOD = _LOD;
  state = 0;
}

bool InnerSummonParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  if (state == 0)
  {
    const Uint64 cur_time = get_time();
    const Uint64 age = cur_time - born;
    if (age > 1200000)
      state = 1;
  }
  else if (state == 1)
  {
    const alpha_t scalar = math_cache.powf_05_close((float)delta_t / 500000);
    alpha *= scalar;
  
    if (alpha < 0.015)
      return false;
  }
  
  return true;
}

GLuint InnerSummonParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

SummonEffect::SummonEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const SummonType _type, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "SummonEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  type = _type;
  inner_spawner = new IFSParticleSpawner();
  outer_spawner = new IFSParticleSpawner();
  smoke_mover = new SmokeMover(this);
  gravity_center = *pos;
  gravity_center.y += 0.2;
  gravity_mover = new GravityMover(this, &gravity_center);
  
  desired_LOD = _LOD;
  inner_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 1, 0.0), 0.5));
  inner_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1, -1, -1), 0.5));
  inner_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.155, -1, -1.155), 0.5));
  inner_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, -1, 1), 0.5));
  
  switch(type)
  {
    case RABBIT:
    {
      inner_color[0] = 0.7;
      inner_color[1] = 0.6;
      inner_color[2] = 0.4;
      outer_color[0] = 0.6;
      outer_color[1] = 0.3;
      outer_color[2] = 0.75;
      inner_texture = &(base->TexVoid);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.155, 0.0, 1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.155, 0.0, 1.0), 0.5));
      outer_alpha = 1.0;
      break;
    }
    case RAT:
    {
      inner_color[0] = 0.6;
      inner_color[1] = 0.5;
      inner_color[2] = 0.3;
      outer_color[0] = 0.55;
      outer_color[1] = 0.3;
      outer_color[2] = 0.8;
      inner_texture = &(base->TexFlare);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.8, 0.0, 1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, 0.5), 0.9));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.2, 0.0, -0.9), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.9, 0.0, -0.3), 0.5));
      outer_alpha = 1.0;
      break;
    }
    case BEAVER:
    {
      inner_color[0] = 0.7;
      inner_color[1] = 0.7;
      inner_color[2] = 0.3;
      outer_color[0] = 0.5;
      outer_color[1] = 0.3;
      outer_color[2] = 0.85;
      inner_texture = &(base->TexVoid);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, 1.0), 0.8));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.6, 0.0, -0.3), 0.2));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, -1.0), 0.2));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.6, 0.0, -0.3), 0.2));
      outer_alpha = 1.0;
      break;
    }
    case DEER:
    {
      inner_color[0] = 0.7;
      inner_color[1] = 0.8;
      inner_color[2] = 0.4;
      outer_color[0] = 0.45;
      outer_color[1] = 0.3;
      outer_color[2] = 0.9;
      inner_texture = &(base->TexVoid);
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.3, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, 0.0), 0.2));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, -1.0), 0.2));
      outer_alpha = 1.0;
      break;
    }
    case GREEN_SNAKE:
    {
      inner_color[0] = 0.3;
      inner_color[1] = 1.0;
      inner_color[2] = 0.2;
      outer_color[0] = 0.3;
      outer_color[1] = 0.3;
      outer_color[2] = 1.0;
      inner_texture = &(base->TexFlare);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSSinusoidalElement(0.5, Vec3(0.3, 0.0, 0.8), Vec3(0.30, 0.0, 0.54), Vec3(1.0, 0.0, 0.85)));
      outer_spawner->ifs_elements.push_back(new IFSSinusoidalElement(0.5, Vec3(1.1, 0.0, 2.2), Vec3(0.21, 0.0, 0.77), Vec3(0.89, 0.0, 0.91)));
      outer_spawner->ifs_elements.push_back(new IFSSinusoidalElement(0.5, Vec3(2.9, 0.0, 2.2), Vec3(0.81, 0.0, 0.71), Vec3(0.77, 0.0, 0.71)));
      outer_alpha = 1.0;
      break;
    }
    case RED_SNAKE:
    {
      inner_color[0] = 1.0;
      inner_color[1] = 0.2;
      inner_color[2] = 0.3;
      outer_color[0] = 0.3;
      outer_color[1] = 0.45;
      outer_color[2] = 0.9;
      inner_texture = &(base->TexFlare);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSSinusoidalElement(0.5, Vec3(3.3, 0.0, 0.8), Vec3(0.30, 0.0, 0.54), Vec3(1.0, 0.0, 0.85)));
      outer_spawner->ifs_elements.push_back(new IFSSinusoidalElement(0.5, Vec3(3.1, 0.0, 2.2), Vec3(0.21, 0.0, 0.77), Vec3(0.89, 0.0, 0.91)));
      outer_spawner->ifs_elements.push_back(new IFSSinusoidalElement(0.5, Vec3(3.9, 0.0, 2.2), Vec3(0.81, 0.0, 0.71), Vec3(0.77, 0.0, 0.71)));
      outer_alpha = 1.0;
      break;
    }
    case BROWN_SNAKE:
    {
      inner_color[0] = 0.7;
      inner_color[1] = 0.6;
      inner_color[2] = 0.5;
      outer_color[0] = 0.3;
      outer_color[1] = 0.55;
      outer_color[2] = 0.8;
      inner_texture = &(base->TexFlare);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, 1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSSinusoidalElement(0.5, Vec3(5.3, 0.0, 3.8), Vec3(0.30, 0.0, 0.54), Vec3(1.0, 0.0, 0.85)));
      outer_spawner->ifs_elements.push_back(new IFSSinusoidalElement(0.5, Vec3(5.1, 0.0, 5.2), Vec3(0.21, 0.0, 0.77), Vec3(0.89, 0.0, 0.91)));
      outer_spawner->ifs_elements.push_back(new IFSSinusoidalElement(0.5, Vec3(5.9, 0.0, 5.2), Vec3(0.81, 0.0, 0.71), Vec3(0.77, 0.0, 0.71)));
      outer_alpha = 1.0;
      break;
    }
    case FOX:
    {
      inner_color[0] = 0.9;
      inner_color[1] = 0.5;
      inner_color[2] = 0.5;
      outer_color[0] = 0.3;
      outer_color[1] = 0.7;
      outer_color[2] = 0.7;
      inner_texture = &(base->TexVoid);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, -1.0), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.8, 0.0, 0.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, 0.7), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.7, 0.0, 0.5), 0.3));
      outer_spawner->ifs_elements.push_back(new IFS2DSwirlElement(0.5));
      outer_alpha = 1.0;
      break;
    }
    case BOAR:
    {
      inner_color[0] = 0.5;
      inner_color[1] = 0.4;
      inner_color[2] = 0.4;
      outer_color[0] = 0.3;
      outer_color[1] = 0.8;
      outer_color[2] = 0.55;
      inner_texture = &(base->TexVoid);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, 0.0), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, 0.0), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.7, 0.0, 0.8), 0.2));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.7, 0.0, 0.8), 0.2));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.7, 0.0, -0.8), 0.3));
      outer_spawner->ifs_elements.push_back(new IFS2DSwirlElement(0.5));
//      outer_spawner->ifs_elements.push_back(new IFSSinusoidalElement(2.5, Vec3(1.3, 0.0, 2.3), Vec3(0.88, 0.0, 0.93), Vec3(1.0, 0.0, 1.0)));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.5, Vec3(0.0, 0.0, 0.0), Vec3(1.0, 1e6, 1.0)));
      outer_alpha = 1.0;
      break;
    }
    case WOLF:
    {
      inner_color[0] = 0.5;
      inner_color[1] = 0.5;
      inner_color[2] = 0.5;
      outer_color[0] = 0.3;
      outer_color[1] = 0.9;
      outer_color[2] = 0.45;
      inner_texture = &(base->TexFlare);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.8, 0.0, -1.0), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.7, 0.0, 0.3), 0.7));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.3, 0.0, 0.2), 0.2));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.8, 0.0, -1.0), 0.6));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, -0.7), 0.5));
      outer_spawner->ifs_elements.push_back(new IFS2DSwirlElement(0.05));
      outer_alpha = 1.0;
      break;
    }
    case PUMA:
    {
      inner_color[0] = 0.8;
      inner_color[1] = 0.8;
      inner_color[2] = 0.6;
      outer_color[0] = 0.3;
      outer_color[1] = 1.0;
      outer_color[2] = 0.3;
      inner_texture = &(base->TexCrystal);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.8, 0.0, -1.0), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.7, 0.0, 0.3), 0.7));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.4, 0.0, 0.6), 0.7));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.8, 0.0, -1.0), 0.2));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, -0.7), 0.5));
      outer_spawner->ifs_elements.push_back(new IFS2DSwirlElement(0.05));
      outer_spawner->ifs_elements.push_back(new IFS2DSwirlElement(0.3));
      outer_alpha = 1.0;
      break;
    }
    case BEAR:
    {
      inner_color[0] = 0.3;
      inner_color[1] = 0.2;
      inner_color[2] = 0.2;
      outer_color[0] = 0.45;
      outer_color[1] = 0.9;
      outer_color[2] = 0.3;
      inner_texture = &(base->TexCrystal);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, 1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, 1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, -0.5), 0.5));
      outer_spawner->ifs_elements.push_back(new IFS2DSwirlElement(0.01));
      outer_alpha = 1.0;
      break;
    }
    case SKELETON:
    {
      inner_color[0] = 1.0;
      inner_color[1] = 1.0;
      inner_color[2] = 0.8;
      outer_color[0] = 0.55;
      outer_color[1] = 0.8;
      outer_color[2] = 0.3;
      inner_texture = &(base->TexInverse);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.6, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.6, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, 0.2), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, 0.2), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, 1.0), 0.5));
      outer_alpha = 1.0;
      break;
    }
    case SMALL_GARGOYLE:
    {
      inner_color[0] = 0.6;
      inner_color[1] = 0.6;
      inner_color[2] = 0.6;
      outer_color[0] = 0.7;
      outer_color[1] = 0.7;
      outer_color[2] = 0.3;
      inner_texture = &(base->TexInverse);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.6, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.6, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, 0.2), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, 0.2), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, 1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.2, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.2, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_alpha = 1.0;
      break;
    }
    case MEDIUM_GARGOYLE:
    {
      inner_color[0] = 0.7;
      inner_color[1] = 0.7;
      inner_color[2] = 0.7;
      outer_color[0] = 0.75;
      outer_color[1] = 0.6;
      outer_color[2] = 0.3;
      inner_texture = &(base->TexInverse);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.6, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.6, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, 0.2), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, 0.2), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, 1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.35, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.35, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_alpha = 1.0;
      break;
    }
    case LARGE_GARGOYLE:
    {
      inner_color[0] = 0.8;
      inner_color[1] = 0.8;
      inner_color[2] = 0.8;
      outer_color[0] = 0.8;
      outer_color[1] = 0.55;
      outer_color[2] = 0.3;
      inner_texture = &(base->TexInverse);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.6, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.6, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, 0.2), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, 0.2), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, 1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.5, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.5, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_alpha = 1.0;
      break;
    }
    case FLUFFY:
    {
      inner_color[0] = 1.0;
      inner_color[1] = 1.0;
      inner_color[2] = 1.0;
      outer_color[0] = 0.85;
      outer_color[1] = 0.5;
      outer_color[2] = 0.3;
      inner_texture = &(base->TexWater);
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.3, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, 0.0), 0.2));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, -1.0), 0.2));
      outer_alpha = 1.0;
      break;
    }
    case CHIMERAN_WOLF:
    {
      inner_color[0] = 0.9;
      inner_color[1] = 1.0;
      inner_color[2] = 0.8;
      outer_color[0] = 0.9;
      outer_color[1] = 0.45;
      outer_color[2] = 0.3;
      inner_texture = &(base->TexFlare);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, 0.5), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.8, 0.0, -1.0), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.3, 0.0, -0.2), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.1, 0.0, 1.0), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.3, 0.0, 0.3), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.5, 0.0, -1.0), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, 0.2), 0.3));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.5, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.5, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_alpha = 1.0;
      break;
    }
    case YETI:
    {
      inner_color[0] = 1.0;
      inner_color[1] = 0.9;
      inner_color[2] = 1.0;
      outer_color[0] = 1.0;
      outer_color[1] = 0.3;
      outer_color[2] = 0.3;
      inner_texture = &(base->TexFlare);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.0, 0.0, -1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.155, 0.0, 1.0), 0.5));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.155, 0.0, 1.0), 0.5));
      outer_alpha = 1.0;
      break;
    }
    case ARCTIC_CHIMERAN:
    {
      inner_color[0] = 1.0;
      inner_color[1] = 1.0;
      inner_color[2] = 1.0;
      outer_color[0] = 1.0;
      outer_color[1] = 0.4;
      outer_color[2] = 0.4;
      inner_texture = &(base->TexVoid);
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(1.0, 0.0, 0.5), 0.8));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.8, 0.0, -1.0), 0.8));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.3, 0.0, -0.2), 0.8));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(0.1, 0.0, 1.0), 0.8));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.3, 0.0, 0.3), 0.8));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-0.5, 0.0, -1.0), 0.8));
      outer_spawner->ifs_elements.push_back(new IFSLinearElement(Vec3(-1.0, 0.0, 0.2), 0.8));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.5, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.5, Vec3(1.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_alpha = 1.0;
      break;
    }
    case GIANT:
    {
      inner_color[0] = 1.0;
      inner_color[1] = 0.2;
      inner_color[2] = 0.1;
      outer_color[0] = 1.0;
      outer_color[1] = 0.5;
      outer_color[2] = 0.5;
      inner_texture = &(base->TexShimmer);
      outer_spawner->ifs_elements.push_back(new IFS2DSwirlElement(0.5));
      outer_spawner->ifs_elements.push_back(new IFSSinusoidalElement(2.5, Vec3(1.3, 0.0, 2.3), Vec3(0.88, 0.0, 0.93), Vec3(1.0, 0.0, 1.0)));
      outer_spawner->ifs_elements.push_back(new IFSRingElement(0.5, Vec3(0.0, 0.0, 1.0), Vec3(1.0, 1e6, 1.0)));
      outer_alpha = 1.0;
      break;
    }
  }
  
  request_LOD(_LOD);
  
  inner_color[0] = outer_color[0];
  inner_color[1] = outer_color[1];
  inner_color[2] = outer_color[2];

  while ((int)particles.size() < LOD * 5)
  {
    Vec3 velocity(0.0, 0.5, 0.0);
/*
    Vec3 coords = outer_spawner->get_new_coords() * outer_radius / 2.0 + *pos;
    coords.y += 0.05;
    Particle* p = new OuterSummonParticle(this, smoke_mover, coords, velocity, outer_size, outer_alpha, outer_color[0], outer_color[1], outer_color[2], LOD);
    if (!base->push_back_particle(p))
      break;
    coords = outer_spawner->get_new_coords() * outer_radius / 2.0 + *pos;
    coords.y += 0.05;
    p = new OuterSummonParticle(this, smoke_mover, coords, velocity, outer_size, outer_alpha, outer_color[0], outer_color[1], outer_color[2], LOD);
    if (!base->push_back_particle(p))
      break;
    coords = outer_spawner->get_new_coords() * outer_radius / 2.0 + *pos;
    coords.y += 0.05;
    p = new OuterSummonParticle(this, smoke_mover, coords, velocity, outer_size, outer_alpha, outer_color[0], outer_color[1], outer_color[2], LOD);
    if (!base->push_back_particle(p))
      break;
    coords = outer_spawner->get_new_coords() * outer_radius / 2.0 + *pos;
    coords.y += 0.05;
    p = new OuterSummonParticle(this, smoke_mover, coords, velocity, outer_size, outer_alpha, outer_color[0], outer_color[1], outer_color[2], LOD);
    if (!base->push_back_particle(p))
      break;
    coords = outer_spawner->get_new_coords() * outer_radius / 2.0 + *pos;
    coords.y += 0.05;
    p = new OuterSummonParticle(this, smoke_mover, coords, velocity, outer_size, outer_alpha, outer_color[0], outer_color[1], outer_color[2], LOD);
    if (!base->push_back_particle(p))
      break;
*/
    Vec3 coords = inner_spawner->get_new_coords() * inner_size / 40.0;
    coords.y /= 5;
    coords += *pos;
    coords.y += 0.2;
    velocity = Vec3(0.0, 0.0, 0.0);
    Particle* p = new InnerSummonParticle(this, gravity_mover, coords, velocity, inner_size, inner_alpha, inner_color[0], inner_color[1], inner_color[2], inner_texture, LOD);
    if (!base->push_back_particle(p))
      break;
  }
  
  count = 0;
}

SummonEffect::~SummonEffect()
{
  delete inner_spawner;
  delete outer_spawner;
  delete smoke_mover;
  delete gravity_mover;
  if (EC_DEBUG)
    std::cout << "SummonEffect (" << this << ") destroyed." << std::endl;
}

void SummonEffect::request_LOD(const Uint16 _LOD)
{
  if (_LOD <= desired_LOD)
    LOD = _LOD;
  else
    LOD = desired_LOD;

  switch(type)
  {
    case RABBIT:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 2.9 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 17;
      break;
    }
    case RAT:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 3.0 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 11;
      break;
    }
    case BEAVER:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 3.2 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 11;
      break;
    }
    case DEER:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 3.3 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 13;
      break;
    }
    case GREEN_SNAKE:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 3.5 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 9;
      break;
    }
    case RED_SNAKE:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 3.6 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 9;
      break;
    }
    case BROWN_SNAKE:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 3.8 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 9;
      break;
    }
    case FOX:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 3.9 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 11;
      break;
    }
    case BOAR:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 4.1 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 11;
      break;
    }
    case WOLF:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 4.2 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 9;
      break;
    }
    case PUMA:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 5.8 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 13;
      break;
    }
    case BEAR:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 6.0 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 13;
      break;
    }
    case SKELETON:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 6.2 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 15;
      break;
    }
    case SMALL_GARGOYLE:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 6.4 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 15;
      break;
    }
    case MEDIUM_GARGOYLE:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 6.6 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 15;
      break;
    }
    case LARGE_GARGOYLE:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 6.8 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 15;
      break;
    }
    case FLUFFY:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 5.3 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 12;
      break;
    }
    case CHIMERAN_WOLF:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 7.5 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 17;
      break;
    }
    case YETI:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 11.0 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 24;
      break;
    }
    case ARCTIC_CHIMERAN:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 9.0 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 25;
      break;
    }
    case GIANT:
    {
      inner_alpha = 1.6 / (LOD + 3);
      inner_size = 39.0 * 7.8 / (LOD + 3);
      gravity_mover->mass = 5e8 * inner_size * (LOD + 3) / 10.0;
      outer_size = inner_size / 15;
      break;
    }
  }
  outer_radius = outer_size * (LOD + 3) / 10;
  count_scalar = 3000 / _LOD;
}

bool SummonEffect::idle(const Uint64 usec)
{
  if (particles.size() == 0)
    return false;
  
  if (recall)
    return false;
  
  const Uint64 cur_time = get_time();
  const Uint64 age = cur_time - born;
  if (age < 1000000)
  {
    count += usec;

    while (count > 0)
    {
      Vec3 coords = outer_spawner->get_new_coords() * outer_radius / 2.0 + *pos;
      coords.y += 0.05;
      Particle* p = new OuterSummonParticle(this, smoke_mover, coords, Vec3(0.0, 0.05, 0.0), outer_size, outer_alpha, outer_color[0], outer_color[1], outer_color[2], LOD);
      if (!base->push_back_particle(p))
      {
        count = 0;
        break;
      }
      count -= count_scalar;
    }
  }
  
  gravity_center.y += usec / 10000000.0;
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef SFX
