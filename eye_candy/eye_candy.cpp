#ifdef SFX

// I N C L U D E S ////////////////////////////////////////////////////////////

#include <SDL.h>
#include <SDL_image.h>
#include <errno.h>

#include "eye_candy.h"
#include "math_cache.h"

namespace ec
{

// G L O B A L S //////////////////////////////////////////////////////////////

MathCache_Lorange math_cache;

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

Texture::Texture()
{
}

Texture::~Texture()
{
  for (int i = 0; i < 4; i++)
  {
    for (std::vector<GLuint>::iterator iter = texture_ids[i].begin(); iter != texture_ids[i].end(); iter++)
    {
      const GLuint texture = *iter;
      glDeleteTextures(1, &texture);
    }
  }
}

GLuint Texture::get_texture(const Uint16 res_index) const
{
  return get_texture(res_index, randint(texture_ids[res_index].size()));
}

GLuint Texture::get_texture(const Uint16 res_index, const int frame) const
{
  return texture_ids[res_index][frame];
}

GLuint Texture::get_texture(const Uint16 res_index, const Uint64 born, const Uint64 changerate) const
{
  return get_texture(res_index, ((get_time() - born) / changerate) % texture_ids[res_index].size());
}

void Texture::push_texture(const std::string filename)
{
  SDL_Surface* tex;
  GLuint texture_id;
  tex = IMG_Load(filename.c_str());
  if (!tex)
  {
    std::cerr << "ERROR: Cannot load texture '" << filename << "'." << std::endl;
    exit(1);
  }
  if (tex->format->palette)
  {
    std::cerr << "ERROR: Cannot use paletted texture '" << filename << "'." << std::endl;
    exit(1);
  }
  if (tex->w != tex->h)
  {
    std::cerr << "ERROR: Textures must be square; please fix '" << filename << "'." << std::endl;
    exit(1);
  }
  if ((tex->w != 16) && (tex->w != 32) && (tex->w != 64) && (tex->w != 128))
  {
    std::cerr << "ERROR: Only 16x16, 32x32, 64x64, and 128x128 textures supportex; fix '" << filename << "'." << std::endl;
    exit(1);
  }

  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, tex->format->BytesPerPixel, tex->w, tex->h, 0, (tex->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, tex->pixels);
  SDL_FreeSurface(tex);
  
  if (tex->w == 16)
    texture_ids[0].push_back(texture_id);
  if (tex->w == 32)
    texture_ids[1].push_back(texture_id);
  if (tex->w == 64)
    texture_ids[2].push_back(texture_id);
  if (tex->w == 128)
    texture_ids[3].push_back(texture_id);
}

Shape::~Shape()
{
  delete[] vertices;
  delete[] facets;
  delete[] normals;
}

void Shape::draw()
{

#if 0
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_INDEX_ARRAY);
#endif
  
  glPushMatrix();
  glTranslated(pos.x, pos.y, pos.z);
  glDisable(GL_TEXTURE_2D);
  glColor4f(color.x, color.y, color.z, alpha);

#if 0
  glNormalPointer(GL_DOUBLE, 0, normals);
  glVertexPointer(3, GL_DOUBLE, 0, vertices);
  glIndexPointer(GL_INT, 0, facets);
  glDrawArrays(GL_TRIANGLES, 0, facet_count);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_INDEX_ARRAY);
#else
  glBegin(GL_TRIANGLES);
  {
    for (int i = 0; i < facet_count; i++)
    {
      glNormal3d(normals[facets[i * 3] * 3], normals[facets[i * 3] * 3 + 1], normals[facets[i * 3] * 3 + 2]);
      glVertex3d(vertices[facets[i * 3] * 3], vertices[facets[i * 3] * 3 + 1], vertices[facets[i * 3] * 3 + 2]);
      glNormal3d(normals[facets[i * 3 + 1] * 3], normals[facets[i * 3 + 1] * 3 + 1], normals[facets[i * 3 + 1] * 3 + 2]);
      glVertex3d(vertices[facets[i * 3 + 1] * 3], vertices[facets[i * 3 + 1] * 3 + 1], vertices[facets[i * 3 + 1] * 3 + 2]);
      glNormal3d(normals[facets[i * 3 + 2] * 3], normals[facets[i * 3 + 2] * 3 + 1], normals[facets[i * 3 + 2] * 3 + 2]);
      glVertex3d(vertices[facets[i * 3 + 2] * 3], vertices[facets[i * 3 + 2] * 3 + 1], vertices[facets[i * 3 + 2] * 3 + 2]);
    }
  }
  glEnd();
#endif
  glEnable(GL_TEXTURE_2D);
  glPopMatrix();
}

CaplessCylinder::CaplessCylinder(const Vec3 _start, const Vec3 _end, const Vec3 _color, const alpha_t _alpha, const coord_t _radius, const int polys)
{
  radius = _radius;
  start = _start;
  end = _end;
  orig_start = _start;
  orig_end = _end;
  pos = (start + end) / 2;
  start -= pos;
  end -= pos;
  color = _color;
  alpha = _alpha;
  
  Vec3 normalized = start;
  normalized.normalize();

  const int subdivisions = ((polys - 1) / 2) + 1;
  vertex_count = subdivisions * 2;
  vertices = new coord_t[vertex_count * 3];
  normals = new coord_t[vertex_count * 3];

  // Get the coordinates.
  const angle_t radian_increment = 2 * PI / subdivisions;
  int i = 0;
  for (angle_t rad = 0; rad < 2 * PI - 0.0001; rad += radian_increment, i++)
  {
    vertices[i * 3] = end.x + radius * (cos(rad) * normalized.z + sin(rad) * normalized.y);
    vertices[i * 3 + 1] = end.y + radius * (cos(rad) * normalized.x + sin(rad) * normalized.z);
    vertices[i * 3 + 2] = end.z + radius * (cos(rad) * normalized.y + sin(rad) * normalized.x);
    normals[i * 3] = cos(rad) * normalized.z + sin(rad) * normalized.y;
    normals[i * 3 + 1] = cos(rad) * normalized.x + sin(rad) * normalized.z;
    normals[i * 3 + 2] = cos(rad) * normalized.y + sin(rad) * normalized.x;
    vertices[i * 3 + subdivisions * 3] = start.x + radius * (cos(rad - radian_increment / 2) * normalized.z + sin(rad - radian_increment / 2) * normalized.y);
    vertices[i * 3 + 1 + subdivisions * 3] = start.y + radius * (cos(rad - radian_increment / 2) * normalized.x + sin(rad - radian_increment / 2) * normalized.z);
    vertices[i * 3 + 2 + subdivisions * 3] = start.z + radius * (cos(rad - radian_increment / 2) * normalized.y + sin(rad - radian_increment / 2) * normalized.x);
    normals[i * 3 + subdivisions * 3] = cos(rad - radian_increment / 2) * normalized.z + sin(rad - radian_increment / 2) * normalized.y;
    normals[i * 3 + subdivisions * 3 + 1] = cos(rad - radian_increment / 2) * normalized.x + sin(rad - radian_increment / 2) * normalized.z;
    normals[i * 3 + subdivisions * 3 + 2] = cos(rad - radian_increment / 2) * normalized.y + sin(rad - radian_increment / 2) * normalized.x;
  }

  facet_count = subdivisions * 2;
  facets = new GLuint[facet_count * 3];

  // Add in the sides.
  for (int i = 0; i < subdivisions; i++)
  {
    facets[i * 6] = i;
    facets[i * 6 + 1] = i + subdivisions;
    facets[i * 6 + 2] = i + subdivisions + 1;
    facets[i * 6 + 3] = i + subdivisions + 1;
    facets[i * 6 + 4] = i + 1;
    facets[i * 6 + 5] = i;
  }
  
  // Wraparound.
  facets[(subdivisions - 1) * 6 + 2] = subdivisions;
  facets[(subdivisions - 1) * 6 + 3] = subdivisions;
  facets[(subdivisions - 1) * 6 + 4] = 0;
}

Cylinder::Cylinder(const Vec3 _start, const Vec3 _end, const Vec3 _color, const alpha_t _alpha, const coord_t _radius, const int polys)
{
  radius = _radius;
  start = _start;
  end = _end;
  orig_start = _start;
  orig_end = _end;
  pos = (start + end) / 2;
  start -= pos;
  end -= pos;
  color = _color;
  alpha = _alpha;
  
  Vec3 normalized = start;
  normalized.normalize();

  const int subdivisions = ((polys - 1) / 4) + 1;
  vertex_count = subdivisions * 4 + 2;	//+2 is for the centerpoints of the caps.
  vertices = new coord_t[vertex_count * 3];
  normals = new coord_t[vertex_count * 3];

  // Get the coordinates.
  const angle_t radian_increment = 2 * PI / subdivisions;
  int i = 0;
  for (angle_t rad = 0; rad < 2 * PI - 0.0001; rad += radian_increment, i++)
  {
    vertices[i * 3] = end.x + radius * (cos(rad) * normalized.z + sin(rad) * normalized.y);
    vertices[i * 3 + 1] = end.y + radius * (cos(rad) * normalized.x + sin(rad) * normalized.z);
    vertices[i * 3 + 2] = end.z + radius * (cos(rad) * normalized.y + sin(rad) * normalized.x);
    normals[i * 3] = normalized.x;
    normals[i * 3 + 1] = normalized.y;
    normals[i * 3 + 2] = normalized.z;
    vertices[i * 3 + subdivisions * 3] = start.x + radius * (cos(rad - radian_increment / 2) * normalized.z + sin(rad - radian_increment / 2) * normalized.y);
    vertices[i * 3 + 1 + subdivisions * 3] = start.y + radius * (cos(rad - radian_increment / 2) * normalized.x + sin(rad - radian_increment / 2) * normalized.z);
    vertices[i * 3 + 2 + subdivisions * 3] = start.z + radius * (cos(rad - radian_increment / 2) * normalized.y + sin(rad - radian_increment / 2) * normalized.x);
    normals[i * 3 + subdivisions * 3] = -normalized.x;
    normals[i * 3 + 1 + subdivisions * 3] = -normalized.y;
    normals[i * 3 + 2 + subdivisions * 3] = -normalized.z;
    vertices[i * 3 + subdivisions * 6] = vertices[i * 3];
    vertices[i * 3 + subdivisions * 6 + 1] = vertices[i * 3 + 1];
    vertices[i * 3 + subdivisions * 6 + 2] = vertices[i * 3 + 2];
    normals[i * 3 + subdivisions * 6] = cos(rad) * normalized.z + sin(rad) * normalized.y;
    normals[i * 3 + subdivisions * 6 + 1] = cos(rad) * normalized.x + sin(rad) * normalized.z;
    normals[i * 3 + subdivisions * 6 + 2] = cos(rad) * normalized.y + sin(rad) * normalized.x;
    vertices[i * 3 + subdivisions * 9] = vertices[i * 3 + subdivisions * 3];
    vertices[i * 3 + subdivisions * 9 + 1] = vertices[i * 3 + subdivisions * 3 + 1];
    vertices[i * 3 + subdivisions * 9 + 2] = vertices[i * 3 + subdivisions * 3 + 2];
    normals[i * 3 + subdivisions * 9] = cos(rad - radian_increment / 2) * normalized.z + sin(rad - radian_increment / 2) * normalized.y;
    normals[i * 3 + subdivisions * 9 + 1] = cos(rad - radian_increment / 2) * normalized.x + sin(rad - radian_increment / 2) * normalized.z;
    normals[i * 3 + subdivisions * 9 + 2] = cos(rad - radian_increment / 2) * normalized.y + sin(rad - radian_increment / 2) * normalized.x;
  }

  // Don't forget the centerpoints of the caps.
  vertices[subdivisions * 12] = end.x;
  vertices[subdivisions * 12 + 1] = end.y;
  vertices[subdivisions * 12 + 2] = end.z;
  normals[subdivisions * 12] = normalized.x;
  normals[subdivisions * 12 + 1] = normalized.y;
  normals[subdivisions * 12 + 2] = normalized.z;
  vertices[subdivisions * 12 + 3] = start.x;
  vertices[subdivisions * 12 + 4] = start.y;
  vertices[subdivisions * 12 + 5] = start.z;
  normals[subdivisions * 12 + 3] = -normalized.x;
  normals[subdivisions * 12 + 4] = -normalized.y;
  normals[subdivisions * 12 + 5] = -normalized.z;

  facet_count = subdivisions * 4;
  facets = new GLuint[facet_count * 3];

  // First, add in the caps.
  for (int i = 0; i < subdivisions; i++)
  {
    facets[i * 3] = subdivisions * 4;
    facets[i * 3 + 1] = i;
    facets[i * 3 + 2] = i + 1;
    facets[i * 3 + subdivisions * 3] = subdivisions * 4 + 1;
    facets[i * 3 + 1 + subdivisions * 3] = i + subdivisions;
    facets[i * 3 + 2 + subdivisions * 3] = i + 1 + subdivisions;
  }
  // Wraparound.
  facets[(subdivisions - 1) * 3 + 2] = 0;
  facets[(subdivisions - 1) * 3 + 2 + subdivisions * 3] = subdivisions;
  
  // Now, add in the sides.
  for (int i = 0; i < subdivisions; i++)
  {
    facets[subdivisions * 3 * 2 + i * 6] = i + subdivisions * 2;
    facets[subdivisions * 3 * 2 + i * 6 + 1] = i + subdivisions * 3;
    facets[subdivisions * 3 * 2 + i * 6 + 2] = i + subdivisions * 3 + 1;
    facets[subdivisions * 3 * 2 + i * 6 + 3] = i + subdivisions * 3 + 1;
    facets[subdivisions * 3 * 2 + i * 6 + 4] = i + subdivisions * 2 + 1;
    facets[subdivisions * 3 * 2 + i * 6 + 5] = i + subdivisions * 2;
  }
  
  // Wraparound.
  facets[subdivisions * 3 * 2 + (subdivisions - 1) * 6 + 2] = subdivisions * 3;
  facets[subdivisions * 3 * 2 + (subdivisions - 1) * 6 + 3] = subdivisions * 3;
  facets[subdivisions * 3 * 2 + (subdivisions - 1) * 6 + 4] = subdivisions * 2;
}

Sphere::Sphere(const Vec3 _pos, const Vec3 _color, const alpha_t _alpha, const coord_t _radius, const int polys)
{
  radius = _radius;
  pos = _pos;
  color = _color;
  alpha = _alpha;
  
  // Start with an octahedron.
  typedef std::pair<angle_t, angle_t> SphericalCoord;
  std::vector<SphericalCoord> spherical_vertices;

  spherical_vertices.push_back(SphericalCoord(0.0, 0.0));
  spherical_vertices.push_back(SphericalCoord(0.0, PI * 0.5));
  spherical_vertices.push_back(SphericalCoord(PI * 0.5, PI * 0.5));
  spherical_vertices.push_back(SphericalCoord(PI, PI * 0.5));
  spherical_vertices.push_back(SphericalCoord(PI * 1.5, PI * 0.5));
  spherical_vertices.push_back(SphericalCoord(0.0, PI));

  std::vector<Facet> spherical_facets;
  spherical_facets.push_back(Facet(0, 1, 2));
  spherical_facets.push_back(Facet(0, 2, 3));
  spherical_facets.push_back(Facet(0, 3, 4));
  spherical_facets.push_back(Facet(0, 4, 1));
  spherical_facets.push_back(Facet(5, 1, 4));
  spherical_facets.push_back(Facet(5, 2, 1));
  spherical_facets.push_back(Facet(5, 3, 2));
  spherical_facets.push_back(Facet(5, 4, 3));
  
  // Tesselate it until we've reached our subdivision limit.
  while ((int)spherical_facets.size() < polys)
  {
    std::vector<Facet> spherical_facets2;
    for (std::vector<Facet>::iterator iter = spherical_facets.begin(); iter != spherical_facets.end(); iter++)
    {
      const int p1_index = iter->f[0];
      const int p2_index = iter->f[1];
      const int p3_index = iter->f[2];
      const SphericalCoord p1(spherical_vertices[p1_index]);
      const SphericalCoord p2(spherical_vertices[p2_index]);
      const SphericalCoord p3(spherical_vertices[p3_index]);
      coord_t p4, q4;
      average_points(p1.first, p2.first, p1.second, p2.second, p4, q4);
      const int p4_index = spherical_vertices.size();
      spherical_vertices.push_back(SphericalCoord(p4, q4));
      coord_t p5, q5;
      average_points(p2.first, p3.first, p2.second, p3.second, p5, q5);
      const int p5_index = spherical_vertices.size();
      spherical_vertices.push_back(SphericalCoord(p5, q5));
      coord_t p6, q6;
      average_points(p3.first, p1.first, p3.second, p1.second, p6, q6);
      const int p6_index = spherical_vertices.size();
      spherical_vertices.push_back(SphericalCoord(p6, q6));
      spherical_facets2.push_back(Facet(p1_index, p4_index, p6_index));
      spherical_facets2.push_back(Facet(p2_index, p5_index, p4_index));
      spherical_facets2.push_back(Facet(p3_index, p6_index, p5_index));
      spherical_facets2.push_back(Facet(p4_index, p5_index, p6_index));
    }
    spherical_facets = spherical_facets2;
  }

  // Convert spherical to rectangular.
  vertex_count = spherical_vertices.size();
  vertices = new coord_t[vertex_count * 3];
  normals = new coord_t[vertex_count * 3];

  for (int i = 0; i < vertex_count; i++)
  {
    const coord_t p = spherical_vertices[i].first;
    const coord_t q = spherical_vertices[i].second;
    normals[i * 3] = sin(p) * sin(q);
    normals[i * 3 + 1] = cos(q);
    normals[i * 3 + 2] = cos(p) * sin(q);
    vertices[i * 3] = normals[i * 3] * radius;
    vertices[i * 3 + 1] = normals[i * 3 + 1] * radius;
    vertices[i * 3 + 2] = normals[i * 3 + 2] * radius;
  }

  
  // Convert facets to OpenGL-suitable array.
  facet_count = spherical_facets.size();
  facets = new GLuint[facet_count * 3];
  for (int i = 0; i < facet_count; i++)
  {
    facets[i * 3] = spherical_facets[i].f[0];
    facets[i * 3 + 1] = spherical_facets[i].f[1];
    facets[i * 3 + 2] = spherical_facets[i].f[2];
  }
}

void Sphere::average_points(const coord_t p1_first, const coord_t p2_first, const coord_t p1_second, const coord_t p2_second, coord_t& p, coord_t& q)
{
  if ((fabs(p2_second) < 0.00001) || (fabs(p2_second - PI) < 0.00001))
    p = p1_first;
  else if ((fabs(p1_second) < 0.00001) || (fabs(p1_second -  PI) < 0.00001))
    p = p2_first;
  else if (fabs(p1_first - p2_first) > PI)
  {
    p = (p1_first + p2_first - 2 * PI) / 2;
    if (p < 0)
      p += 2 * PI;
  }
  else
    p = (p1_first + p2_first) / 2;
  
  q = (p1_second + p2_second) / 2;
}

Obstruction::Obstruction(const coord_t _max_distance, const coord_t _force)
{
  max_distance = _max_distance;
  max_distance_squared = square(max_distance);
  force = _force;
}

Vec3 SimpleCylinderObstruction::get_force_gradient(const Vec3 position)
{	//Vertical cylinder, infinite height.
  const Vec3 translated_pos = position - *(pos);

  const coord_t distsquared = square(translated_pos.x) + square(translated_pos.z);
  if (distsquared < max_distance_squared)
    return translated_pos * (force / (distsquared + 0.0001));
  else
    return Vec3(0.0, 0.0, 0.0);
}

CylinderObstruction::CylinderObstruction(Vec3* _start, Vec3* _end, const coord_t _max_distance, const coord_t _force) : Obstruction(_max_distance, _force)
{
  start = _start;
  end = _end;
  length_vec = *end - *start;	// Assume that this doesn't change.
  length_vec_mag = length_vec.magnitude();
};

Vec3 CylinderObstruction::get_force_gradient(const Vec3 position)
{
  Vec3 v_offset;
  const Vec3 v1 = position - *start;
  angle_t dotprod1;
  if ((dotprod1 = length_vec.dot(v1)) <= 0)
    v_offset = v1;
  else if (length_vec_mag <= dotprod1)
    v_offset = position - *end;
  else
  {
    const coord_t scalar = dotprod1 / length_vec_mag;
    v_offset = *start + (length_vec * scalar);
  }
  const coord_t distsquared = v_offset.magnitude_squared();
  
  if (distsquared < max_distance_squared)
    return v_offset * (force / (distsquared + 0.0001));
  else
    return Vec3(0.0, 0.0, 0.0);
}

Vec3 SphereObstruction::get_force_gradient(const Vec3 position)
{
  const Vec3 translated_pos = position - *(pos);

  const coord_t distsquared = square(translated_pos.x) + square(translated_pos.y) + square(translated_pos.z);
  if (distsquared < square(max_distance))
    return translated_pos * (force / (distsquared + 0.0001));
  else
    return Vec3(0.0, 0.0, 0.0);
}

PolarCoordElement::PolarCoordElement(const coord_t _frequency, const coord_t _offset, const coord_t _scalar, const coord_t _power)
{
  frequency = _frequency;
  offset = _offset;
  scalar = _scalar;
  power = _power;
}
  
coord_t PolarCoordElement::get_radius(const angle_t angle) const
{
  const angle_t temp_cos = cos((angle + offset) * frequency);
  coord_t ret = math_cache.powf_0_1_rough_close(fabs(temp_cos), power) * scalar + scalar;
  ret = copysign(ret, temp_cos);
  return ret;
}

Particle::Particle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity)
{
  effect = _effect;
  base = effect->base;
  cur_motion_blur_point = 0;
  motion_blur = new ParticleHistory[effect->motion_blur_points];
  for (int i = 0; i < effect->motion_blur_points; i++)
    motion_blur[i].alpha = 0;
  mover = _mover;
  pos = _pos;
  velocity = _velocity;
  energy = mover->calculate_energy(*this);
  born = get_time();
}

Particle::~Particle()
{
  delete[] motion_blur;
};

void Particle::draw(const Uint64 usec)
{
  if (base->draw_method == EyeCandy::POINT_SPRITES)
  {
    coord_t tempsize = base->temp_sprite_scalar * size * invsqrt(square(pos.x - base->camera.x) + square(pos.y - base->camera.y) + square(pos.z - base->camera.z));
    tempsize *= flare();
  
    alpha_t tempalpha = alpha;
    if (tempsize < 5.0)			// Pseudo-antialiasing.  Makes the particles look nice.
    {
      tempalpha /= square(5.0 / tempsize);
      tempsize = 5.0;
    }
    else if (tempsize > base->max_point_size)
      tempsize = base->max_point_size;
    tempalpha = 1.0;
    int res_index;
    if (tempsize <= 16)
      res_index = 0;
    else if (tempsize <= 32)
      res_index = 1;
    else if (tempsize <= 64)
      res_index = 2;
    else
      res_index = 3;
    const GLuint texture = get_texture(res_index);
    base->draw_point_sprite_particle(tempsize, texture, color[0], color[1], color[2], tempalpha, pos);
    if (effect->motion_blur_points > 0)
    {
      const alpha_t faderate = math_cache.powf_0_1_rough_close(effect->motion_blur_fade_rate, (float)usec / 1000000);
    
      for (int i = 0; i < effect->motion_blur_points; i++)
        base->draw_point_sprite_particle(motion_blur[i].size, motion_blur[i].texture, motion_blur[i].color[0], motion_blur[i].color[1], motion_blur[i].color[2], motion_blur[i].alpha, motion_blur[i].pos);
      
      motion_blur[cur_motion_blur_point] = ParticleHistory(tempsize, texture, color[0], color[1], color[2], tempalpha, pos);
      cur_motion_blur_point++;
      
      for (int i = 0; i < effect->motion_blur_points; i++)
        motion_blur[i].alpha *= faderate;
      
      if (cur_motion_blur_point == effect->motion_blur_points)
        cur_motion_blur_point = 0;
    }
  }
  else
  {
    coord_t tempsize = base->billboard_scalar * size;
    tempsize *= flare();
    
    const GLuint texture = get_texture(3);	// Always hires, since we're not checking distance.
//    std::cout << this << ": " << tempsize << ", " << size << ": " << pos << std::endl;
    if (base->draw_method == EyeCandy::FAST_BILLBOARDS)
      base->draw_fast_billboard_particle(tempsize, texture, color[0], color[1], color[2], alpha, pos);
    else
      base->draw_accurate_billboard_particle(tempsize, texture, color[0], color[1], color[2], alpha, pos);
    if (effect->motion_blur_points > 0)
    {
      const alpha_t faderate = math_cache.powf_0_1_rough_close(effect->motion_blur_fade_rate, (float)usec / 1000000);
    
      if (base->draw_method == EyeCandy::FAST_BILLBOARDS)
      {
        for (int i = 0; i < effect->motion_blur_points; i++)
          base->draw_fast_billboard_particle(motion_blur[i].size, motion_blur[i].texture, motion_blur[i].color[0], motion_blur[i].color[1], motion_blur[i].color[2], motion_blur[i].alpha, motion_blur[i].pos);
      }
      else
      {
        for (int i = 0; i < effect->motion_blur_points; i++)
          base->draw_accurate_billboard_particle(motion_blur[i].size, motion_blur[i].texture, motion_blur[i].color[0], motion_blur[i].color[1], motion_blur[i].color[2], motion_blur[i].alpha, motion_blur[i].pos);
      }
      
      motion_blur[cur_motion_blur_point] = ParticleHistory(tempsize, texture, color[0], color[1], color[2], alpha, pos);
      cur_motion_blur_point++;
      
      for (int i = 0; i < effect->motion_blur_points; i++)
        motion_blur[i].alpha *= faderate;
      
      if (cur_motion_blur_point == effect->motion_blur_points)
        cur_motion_blur_point = 0;
    }
  }
}

coord_t Particle::flare() const
{
  assert(flare_frequency);
  if (flare_max == 1.0)
    return 1.0;
  const int offset = long(&alpha);	//Unique to the particle.
  math_cache.powf_0_1_rough_close(fabs(sin((pos.x + pos.y + pos.z + offset) / flare_frequency)), flare_exp);
  const coord_t flare_val = 1.0 / (math_cache.powf_0_1_rough_close(fabs(sin((pos.x + pos.y + pos.z) / flare_frequency + offset)), flare_exp));
  if (flare_val > flare_max)
    return flare_max;
  else
    return flare_val;
}

ParticleMover::ParticleMover(Effect* _effect)
{
  effect = _effect;
  base = effect->base;
}

Vec3 ParticleMover::vec_shift(const Vec3 src, const Vec3 dest, const percent_t percent) const
{
#if 0	// Slow but clear version.  Consider this a comment.
  const coord_t magnitude = src.magnitude();
  Vec3 ret = nonpreserving_vec_shift(src, dest, percent);
  ret.normalize(magnitude);
#else	// Fast but obfuscated
  const coord_t magnitude_squared = src.magnitude_squared();
  Vec3 ret = nonpreserving_vec_shift(src, dest, percent);
  ret *= invsqrt(ret.magnitude_squared() / magnitude_squared);
#endif
  return ret;
}

Vec3 ParticleMover::vec_shift_amount(const Vec3 src, const Vec3 dest, const coord_t amount) const
{
  const Vec3 diff = dest - src;
  const coord_t magnitude = diff.magnitude();
  if (magnitude > amount)
    return dest;
  const percent_t percent = magnitude / amount;
  return vec_shift(src, dest, percent);
}

Vec3 ParticleMover::nonpreserving_vec_shift(const Vec3 src, const Vec3 dest, const percent_t percent) const
{
  return (dest - src) * percent + src * (1.0 - percent);
}

Vec3 ParticleMover::nonpreserving_vec_shift_amount(const Vec3 src, const Vec3 dest, const coord_t amount) const
{
  const Vec3 diff = dest - src;
  const coord_t magnitude = diff.magnitude();
  if (magnitude > amount)
    return dest;
  const percent_t percent = magnitude / amount;
  return nonpreserving_vec_shift(src, dest, percent);
}

void GradientMover::move(Particle& p, Uint64 usec)
{
  const coord_t scalar = usec / 1000000.0;
  Vec3 gradient_velocity = p.velocity + get_force_gradient(p.pos) * scalar;
  p.velocity = gradient_velocity + get_obstruction_gradient(p.pos) * scalar;
#if 0	// Slow but clear version.  Consider this a comment.
  p.velocity.normalize(gradient_velocity.magnitude());
#else	// Fast but obfuscated
  p.velocity *= invsqrt(p.velocity.magnitude_squared() / gradient_velocity.magnitude_squared());
#endif
  p.pos += p.velocity * scalar;
}

Vec3 GradientMover::get_force_gradient(const Vec3& pos) const
{
  return Vec3(0.0, 0.0, 0.0);
}

Vec3 SmokeMover::get_force_gradient(const Vec3& pos) const
{
  return Vec3(0.0, 0.2 * strength, 0.0);
}

Vec3 SpiralMover::get_force_gradient(const Vec3& pos) const
{
  Vec3 shifted_pos = pos - *center;
  return Vec3(shifted_pos.z * spiral_speed - shifted_pos.x * pinch_rate, 0.0, shifted_pos.x * spiral_speed - shifted_pos.z * pinch_rate);
}

PolarCoordsBoundingMover::PolarCoordsBoundingMover(Effect* _effect, const Vec3 _center_pos, const std::vector<PolarCoordElement> _bounding_range, const coord_t _force) : GradientMover(_effect)
{
  center_pos = _center_pos;
  bounding_range = _bounding_range;
  force = _force;
}

Vec3 PolarCoordsBoundingMover::get_force_gradient(const Vec3& pos) const
{
  Vec3 shifted_pos = pos - center_pos;
  const coord_t radius = fastsqrt(square(shifted_pos.x) + square(shifted_pos.z));
  coord_t max_radius = 0.0;
  const angle_t angle = atan2(shifted_pos.x, shifted_pos.z);
  for (std::vector<PolarCoordElement>::const_iterator iter = bounding_range.begin(); iter != bounding_range.end(); iter++)
    max_radius += iter->get_radius(angle);
  if (radius > max_radius)
  {
    const coord_t diff = (radius - max_radius) / radius;
    return -shifted_pos * diff;
  }
  else
    return Vec3(0.0, 0.0, 0.0);
}

Vec3 SimpleGravityMover::get_force_gradient(const Vec3& pos) const
{
  return Vec3(0.0, -0.98, 0.0);
}

Vec3 GradientMover::get_obstruction_gradient(const Vec3& pos) const
{	//Unlike normal force gradients, obstruction gradients are used in a magnitude-preserving fashion.
  Vec3 ret(0.0, 0.0, 0.0);
  for (std::vector<Obstruction*>::iterator iter = effect->obstructions.begin(); iter != effect->obstructions.end(); iter++)
    ret += (*iter)->get_force_gradient(pos);
  return ret;
}

GravityMover::GravityMover(Effect* _effect, Vec3* _center) : GradientMover(_effect)
{
  mass = 2e10;
  max_gravity = 20.0;
  gravity_center_ptr = _center;
  gravity_center = *gravity_center_ptr;
  old_gravity_center = gravity_center;
}

GravityMover::GravityMover(Effect* _effect, Vec3* _center, const energy_t _mass) : GradientMover(_effect)
{
  mass = _mass;
  max_gravity = mass / 1e9;
  gravity_center_ptr = _center;
  gravity_center = *gravity_center_ptr;
  old_gravity_center = gravity_center;
}

void GravityMover::set_gravity_center(Vec3* _gravity_center)
{
  gravity_center_ptr = _gravity_center;
}

void GravityMover::move(Particle& p, Uint64 usec)
{
  old_gravity_center = gravity_center;
  gravity_center = *gravity_center_ptr;
  
  if (usec >= base->max_usec_per_particle_move)
    usec = base->max_usec_per_particle_move;
    
  const coord_t dist = gravity_dist(p, gravity_center);
  if (gravity_center != old_gravity_center)
  {
    const coord_t old_dist = gravity_dist(p, old_gravity_center);
    p.energy += G * mass * (dist - old_dist);
  }
  Vec3 gravity_vec(gravity_center.x - p.pos.x, gravity_center.y - p.pos.y, gravity_center.z - p.pos.z);
 
  // Simulated point gravity sources tend to promote extreme forces that, even if you fix the energy balance, will throw off angles.  Cancel them out.
  energy_t scalar = G * mass / square(dist) + 0.00001;
  if (scalar > max_gravity)
    scalar = max_gravity;
  gravity_vec.normalize(scalar);
  p.pos += p.velocity * ((coord_t)usec / 1000000.0);
  p.velocity += gravity_vec * ((coord_t)usec / 1000000.0);

  // Simulated point gravity sources tend to create/destroy energy when a particle passes near the center.  Cancel it out.
  const energy_t velocity_energy = calculate_velocity_energy(p);
  const energy_t new_energy = G * mass * dist + velocity_energy;
  const energy_t energy_difference = p.energy - new_energy;
  const energy_t new_velocity_energy = velocity_energy + energy_difference;
  if (new_velocity_energy >= 0)
  {
#if 0	// Slow but clear.  Consider this a comment.
    const coord_t new_velocity = fastsqrt(2.0 * new_velocity_energy);
    if (new_velocity)
      p.velocity.normalize(new_velocity);
    else
      p.velocity = Vec3(0.0, 0.0, 0.0);
#else	// Fast but obfuscated
    const coord_t new_velocity_squared = 2.0 * new_velocity_energy;
    if (new_velocity_squared)
      p.velocity *= invsqrt(p.velocity.magnitude_squared() / new_velocity_squared + 0.000001);
    else
      p.velocity = Vec3(0.0, 0.0, 0.0);
#endif
  }
  
  // Factor in the force gradient, if any.
  Vec3 gradient_velocity = p.velocity + get_force_gradient(p.pos);
  Vec3 obstruction_velocity = gradient_velocity + get_obstruction_gradient(p.pos);
  const coord_t grad_mag_squared = gradient_velocity.magnitude_squared();
#if 0	// Slow but clear.  Consider this a comment.
  if (grad_mag_squared)
    obstruction_velocity.normalize(gradient_velocity.magnitude());
  else
    obstruction_velocity = Vec3(0.0, 0.0, 0.0);
#else	// Fast but obfuscated.
  if (grad_mag_squared)
    obstruction_velocity *= invsqrt(obstruction_velocity.magnitude_squared() / grad_mag_squared + 0.000001);
  else
    obstruction_velocity = Vec3(0.0, 0.0, 0.0);
#endif
  const coord_t obstruction_velocity_energy = 0.5 * obstruction_velocity.magnitude_squared();
  p.energy += obstruction_velocity_energy - new_velocity_energy;
  p.velocity = obstruction_velocity;
}

energy_t GravityMover::calculate_velocity_energy(const Particle& p) const
{
  return 0.5 * p.velocity.magnitude_squared();
}
    
energy_t GravityMover::calculate_position_energy(const Particle& p) const
{
  return G * mass * gravity_dist(p, gravity_center);
}

coord_t GravityMover::gravity_dist(const Particle& p, const Vec3& center) const
{
  return fastsqrt(square(p.pos.x - center.x) + square(p.pos.y - center.y) + square(p.pos.z - center.z));
}
    
energy_t GravityMover::calculate_energy(const Particle& p) const
{
  return calculate_velocity_energy(p) + calculate_position_energy(p);
}

Vec3 IFSLinearElement::get_new_coords(const Vec3& pos)
{
  return (pos * inv_scale) + (center * scale);
}

Vec3 IFSSinusoidalElement::get_new_coords(const Vec3& pos)
{
  const Vec3 result(sin(pos.x * scalar.x + offset.x) * scalar2.x, sin(pos.y * scalar.y + offset.y) * scalar2.y, sin(pos.z * scalar.z + offset.z) * scalar2.z);
  return (pos * inv_scale) + (result * scale);
}

Vec3 IFSSphericalElement::get_new_coords(const Vec3& pos)
{
  const coord_t r = pos.magnitude();
  const Vec3 result((pos.x + numerator_adjust.x) / (r + denominator_adjust.x), (pos.y + numerator_adjust.y) / (r + denominator_adjust.y), (pos.z + numerator_adjust.z) / (r + denominator_adjust.z));
  return (pos * inv_scale) + (result * scale);
}

Vec3 IFSRingElement::get_new_coords(const Vec3& pos)
{
  const coord_t r = fastsqrt(square(pos.x) + square(pos.z));
  const Vec3 result((pos.x + numerator_adjust.x) / (r + denominator_adjust.x), 0.0, (pos.z + numerator_adjust.z) / (r + denominator_adjust.z));
  return (pos * inv_scale) + (result * scale);
}

Vec3 IFSSwirlElement::get_new_coords(const Vec3& pos)
{
  const coord_t r = pos.magnitude();
  const angle_t theta_x = atan2(pos.z, pos.y);
  const angle_t theta_y = atan2(pos.x, pos.z);
  const angle_t theta_z = atan2(pos.y, pos.x);
  const Vec3 result(r * cos(theta_x + r), r * cos(theta_y + r), r * cos(theta_z + r));
  return (pos * inv_scale) + (result * scale);
}

Vec3 IFS2DSwirlElement::get_new_coords(const Vec3& pos)
{
  const coord_t r = fastsqrt(square(pos.x) + square(pos.z));
  const angle_t theta_x = atan(pos.z);
  const angle_t theta_z = atan(pos.x);
  const Vec3 result(r * cos(theta_x + r), 0.0, r * cos(theta_z + r));
  return (pos * inv_scale) + (result * scale);
}

Vec3 IFSHorseshoeElement::get_new_coords(const Vec3& pos)
{
  const coord_t r = pos.magnitude();
  const angle_t theta_x = atan2(pos.z, pos.y + 0.0000001f);
  const angle_t theta_y = atan2(pos.x, pos.z + 0.0000001f);
  const angle_t theta_z = atan2(pos.y, pos.x + 0.0000001f);
  const Vec3 result(r * cos(2 * theta_x), r * cos(2 * theta_y), r * cos(2 * theta_z));
  return (pos * inv_scale) + (result * scale);
}

Vec3 IFS2DHorseshoeElement::get_new_coords(const Vec3& pos)
{
  const coord_t r = pos.magnitude();
  const angle_t theta_x = atan(pos.z);
  const angle_t theta_z = atan(pos.y);
  const Vec3 result(r * cos(2 * theta_x), 0.0, r * cos(2 * theta_z));
  return (pos * inv_scale) + (result * scale);
}

IFSParticleSpawner::IFSParticleSpawner(const int count, const coord_t size)
{
  generate(count, Vec3(size, size, size));
  pos = Vec3(0.0, 0.0, 0.0);
}

IFSParticleSpawner::IFSParticleSpawner(const int count, const Vec3 scale)
{
  generate(count, scale);
  pos = Vec3(0.0, 0.0, 0.0);
}

IFSParticleSpawner::~IFSParticleSpawner()
{
  for (std::vector<IFSParticleElement*>::iterator iter = ifs_elements.begin(); iter != ifs_elements.end(); iter++)
    delete *iter;
}

void IFSParticleSpawner::generate(const int count, const Vec3 scale)
{
  for (int i = 0; i < count; i++)
  {
    Vec3 v;
    v.randomize();
    v.x *= scale.x;
    v.y *= scale.y;
    v.z *= scale.z;
    ifs_elements.push_back(new IFSLinearElement(v, randcoord()));
  }
}

Vec3 IFSParticleSpawner::get_new_coords()
{
  std::vector<IFSParticleElement*>::iterator iter = ifs_elements.begin() + randint(ifs_elements.size());
  pos = ifs_elements[randint(ifs_elements.size())]->get_new_coords(pos);
  return pos;
}

Vec3 FilledSphereSpawner::get_new_coords()
{
  Vec3 ret;
  ret.randomize();
  ret.normalize(randcoord() * radius);
  return ret;
}

Vec3 FilledEllipsoidSpawner::get_new_coords()
{
  Vec3 ret;
  ret.randomize();
  ret.normalize(randcoord());
  ret.x *= radius.x;
  ret.y *= radius.y;
  ret.z *= radius.z;
  return ret;
}

Vec3 HollowSphereSpawner::get_new_coords()
{
  Vec3 ret;
  ret.randomize();
  ret.normalize(radius);
  return ret;
}

Vec3 HollowEllipsoidSpawner::get_new_coords()
{
  Vec3 ret;
  ret.randomize();
  ret.normalize();
  ret.x *= radius.x;
  ret.y *= radius.y;
  ret.z *= radius.z;
  return ret;
}

Vec3 FilledDiscSpawner::get_new_coords()
{
  const angle_t angle = randangle(2 * PI);
  const coord_t scale = (1.0 - square(randcoord())) * radius;
  const Vec3 ret(scale * sin(angle), 0.0, scale * cos(angle));
  return ret;
}

Vec3 HollowDiscSpawner::get_new_coords()
{
  const angle_t angle = randangle(2 * PI);
  const Vec3 ret(radius * sin(angle), 0.0, radius * cos(angle));
  return ret;
}

Vec3 FilledPolarCoordsSpawner::get_new_coords()
{
  const angle_t angle = randangle(2 * PI);
  coord_t radius = 0.0;
  for (std::vector<PolarCoordElement>::iterator iter = bounding_range.begin(); iter != bounding_range.end(); iter++)
    radius += iter->get_radius(angle);
  const coord_t scalar = fastsqrt(randcoord());
  return Vec3(sin(angle) * scalar * radius, 0.0, cos(angle) * scalar * radius);
}

coord_t FilledPolarCoordsSpawner::get_area() const
{	// Not 100% accurate, but goot enough.  :)
  coord_t avg_radius = 0;
  for (std::vector<PolarCoordElement>::const_iterator iter = bounding_range.begin(); iter != bounding_range.end(); iter++)
    avg_radius += iter->scalar;
  return PI * square(avg_radius);
}

Vec3 HollowPolarCoordsSpawner::get_new_coords()
{
  const angle_t angle = randangle(2 * PI);
  coord_t radius = 0.0;
  for (std::vector<PolarCoordElement>::iterator iter = bounding_range.begin(); iter != bounding_range.end(); iter++)
    radius += iter->get_radius(angle);
  return Vec3(sin(angle) * radius, 0.0, cos(angle) * radius);
}

coord_t HollowPolarCoordsSpawner::get_area() const
{	// Not 100% accurate, but goot enough.  :)
  coord_t avg_radius = 0;
  for (std::vector<PolarCoordElement>::const_iterator iter = bounding_range.begin(); iter != bounding_range.end(); iter++)
    avg_radius += iter->scalar;
  return PI * square(avg_radius);
}

EyeCandy::EyeCandy()
{
  set_thresholds(10000, 20);
  max_usec_per_particle_move = 100000;
  sprite_scalar = 1.2;
  max_point_size = 500.0;
  lighting_scalar = 1000.0;
  light_estimate = 0.0;
  draw_method = FAST_BILLBOARDS;
  billboard_scalar = 0.2;
  width = 800;
  height = 600;
  temp_sprite_scalar = sprite_scalar * height;
  last_forced_LOD = 10;
}

EyeCandy::EyeCandy(int _max_particles)
{
  set_thresholds(_max_particles, 20);
  max_usec_per_particle_move = 100000;
  sprite_scalar = 1.2;
  max_point_size = 500.0;
  lighting_scalar = 1000.0;
  light_estimate = 0.0;
  draw_method = FAST_BILLBOARDS;
  billboard_scalar = 0.2;
  width = 800;
  height = 600;
  temp_sprite_scalar = sprite_scalar * height;
  last_forced_LOD = 10;
}

EyeCandy::~EyeCandy()
{
  for (std::vector<Effect*>::iterator iter = effects.begin(); iter != effects.end(); iter++)
    delete *iter;
  for (std::vector<Particle*>::iterator iter = particles.begin(); iter != particles.end(); iter++)
    delete *iter;
  for (std::vector<GLenum>::iterator iter = lights.begin(); iter != lights.end(); iter++)
    glDisable(*iter);

}

void EyeCandy::set_thresholds(int _max_particles, int min_framerate)
{
  max_particles = _max_particles;
  LOD_9_threshold = max_particles * 73 / 100;
  LOD_8_threshold = max_particles * 76 / 100;
  LOD_7_threshold = max_particles * 79 / 100;
  LOD_6_threshold = max_particles * 82 / 100;
  LOD_5_threshold = max_particles * 85 / 100;
  LOD_4_threshold = max_particles * 88 / 100;
  LOD_3_threshold = max_particles * 91 / 100;
  LOD_2_threshold = max_particles * 94 / 100;
  LOD_1_threshold = max_particles * 97 / 100;
  LOD_9_time_threshold = (Uint64)(1000000.0 / (min_framerate * 1.9));
  LOD_8_time_threshold = (Uint64)(1000000.0 / (min_framerate * 1.8));
  LOD_7_time_threshold = (Uint64)(1000000.0 / (min_framerate * 1.7));
  LOD_6_time_threshold = (Uint64)(1000000.0 / (min_framerate * 1.6));
  LOD_5_time_threshold = (Uint64)(1000000.0 / (min_framerate * 1.5));
  LOD_4_time_threshold = (Uint64)(1000000.0 / (min_framerate * 1.4));
  LOD_3_time_threshold = (Uint64)(1000000.0 / (min_framerate * 1.3));
  LOD_2_time_threshold = (Uint64)(1000000.0 / (min_framerate * 1.2));
  LOD_1_time_threshold = (Uint64)(1000000.0 / (min_framerate * 1.1));
//  allowable_particles_to_add = max_particles;
}

void EyeCandy::load_textures(const std::string basepath)
{
  // Load the textures.
  TexSimple.push_texture(basepath + "/16x16/simple.png");
  TexFlare.push_texture(basepath + "/16x16/flare1.png");
  TexFlare.push_texture(basepath + "/16x16/flare2.png");
  TexFlare.push_texture(basepath + "/16x16/flare3.png");
  TexVoid.push_texture(basepath + "/16x16/void1.png");
  TexVoid.push_texture(basepath + "/16x16/void2.png");
  TexVoid.push_texture(basepath + "/16x16/void3.png");
  TexTwinflare.push_texture(basepath + "/16x16/twinflare1.png");
  TexTwinflare.push_texture(basepath + "/16x16/twinflare2.png");
  TexTwinflare.push_texture(basepath + "/16x16/twinflare3.png");
  TexTwinflare.push_texture(basepath + "/16x16/twinflare4.png");
  TexTwinflare.push_texture(basepath + "/16x16/twinflare5.png");
  TexInverse.push_texture(basepath + "/16x16/inverse1.png");
  TexInverse.push_texture(basepath + "/16x16/inverse2.png");
  TexInverse.push_texture(basepath + "/16x16/inverse3.png");
  TexInverse.push_texture(basepath + "/16x16/inverse4.png");
  TexShimmer.push_texture(basepath + "/16x16/shimmer1.png");
  TexShimmer.push_texture(basepath + "/16x16/shimmer2.png");
  TexShimmer.push_texture(basepath + "/16x16/shimmer3.png");
  TexCrystal.push_texture(basepath + "/16x16/crystal1.png");
  TexCrystal.push_texture(basepath + "/16x16/crystal2.png");
  TexCrystal.push_texture(basepath + "/16x16/crystal3.png");
  TexWater.push_texture(basepath + "/16x16/water1.png");
  TexWater.push_texture(basepath + "/16x16/water2.png");
  TexWater.push_texture(basepath + "/16x16/water3.png");
  TexLeafMaple.push_texture(basepath + "/16x16/leaf_maple.png");
  TexLeafOak.push_texture(basepath + "/16x16/leaf_oak.png");
  TexLeafAsh.push_texture(basepath + "/16x16/leaf_ash.png");
  TexPetal.push_texture(basepath + "/16x16/petal.png");
  TexSnowflake.push_texture(basepath + "/16x16/snowflake.png");
  TexSimple.push_texture(basepath + "/32x32/simple.png");
  TexFlare.push_texture(basepath + "/32x32/flare1.png");
  TexFlare.push_texture(basepath + "/32x32/flare2.png");
  TexFlare.push_texture(basepath + "/32x32/flare3.png");
  TexVoid.push_texture(basepath + "/32x32/void1.png");
  TexVoid.push_texture(basepath + "/32x32/void2.png");
  TexVoid.push_texture(basepath + "/32x32/void3.png");
  TexTwinflare.push_texture(basepath + "/32x32/twinflare1.png");
  TexTwinflare.push_texture(basepath + "/32x32/twinflare2.png");
  TexTwinflare.push_texture(basepath + "/32x32/twinflare3.png");
  TexTwinflare.push_texture(basepath + "/32x32/twinflare4.png");
  TexTwinflare.push_texture(basepath + "/32x32/twinflare5.png");
  TexInverse.push_texture(basepath + "/32x32/inverse1.png");
  TexInverse.push_texture(basepath + "/32x32/inverse2.png");
  TexInverse.push_texture(basepath + "/32x32/inverse3.png");
  TexInverse.push_texture(basepath + "/32x32/inverse4.png");
  TexShimmer.push_texture(basepath + "/32x32/shimmer1.png");
  TexShimmer.push_texture(basepath + "/32x32/shimmer2.png");
  TexShimmer.push_texture(basepath + "/32x32/shimmer3.png");
  TexCrystal.push_texture(basepath + "/32x32/crystal1.png");
  TexCrystal.push_texture(basepath + "/32x32/crystal2.png");
  TexCrystal.push_texture(basepath + "/32x32/crystal3.png");
  TexWater.push_texture(basepath + "/32x32/water1.png");
  TexWater.push_texture(basepath + "/32x32/water2.png");
  TexWater.push_texture(basepath + "/32x32/water3.png");
  TexLeafMaple.push_texture(basepath + "/32x32/leaf_maple.png");
  TexLeafOak.push_texture(basepath + "/32x32/leaf_oak.png");
  TexLeafAsh.push_texture(basepath + "/32x32/leaf_ash.png");
  TexPetal.push_texture(basepath + "/32x32/petal.png");
  TexSnowflake.push_texture(basepath + "/32x32/snowflake.png");
  TexSimple.push_texture(basepath + "/64x64/simple.png");
  TexFlare.push_texture(basepath + "/64x64/flare1.png");
  TexFlare.push_texture(basepath + "/64x64/flare2.png");
  TexFlare.push_texture(basepath + "/64x64/flare3.png");
  TexVoid.push_texture(basepath + "/64x64/void1.png");
  TexVoid.push_texture(basepath + "/64x64/void2.png");
  TexVoid.push_texture(basepath + "/64x64/void3.png");
  TexTwinflare.push_texture(basepath + "/64x64/twinflare1.png");
  TexTwinflare.push_texture(basepath + "/64x64/twinflare2.png");
  TexTwinflare.push_texture(basepath + "/64x64/twinflare3.png");
  TexTwinflare.push_texture(basepath + "/64x64/twinflare4.png");
  TexTwinflare.push_texture(basepath + "/64x64/twinflare5.png");
  TexInverse.push_texture(basepath + "/64x64/inverse1.png");
  TexInverse.push_texture(basepath + "/64x64/inverse2.png");
  TexInverse.push_texture(basepath + "/64x64/inverse3.png");
  TexInverse.push_texture(basepath + "/64x64/inverse4.png");
  TexShimmer.push_texture(basepath + "/64x64/shimmer1.png");
  TexShimmer.push_texture(basepath + "/64x64/shimmer2.png");
  TexShimmer.push_texture(basepath + "/64x64/shimmer3.png");
  TexCrystal.push_texture(basepath + "/64x64/crystal1.png");
  TexCrystal.push_texture(basepath + "/64x64/crystal2.png");
  TexCrystal.push_texture(basepath + "/64x64/crystal3.png");
  TexWater.push_texture(basepath + "/64x64/water1.png");
  TexWater.push_texture(basepath + "/64x64/water2.png");
  TexWater.push_texture(basepath + "/64x64/water3.png");
  TexLeafMaple.push_texture(basepath + "/64x64/leaf_maple.png");
  TexLeafOak.push_texture(basepath + "/64x64/leaf_oak.png");
  TexLeafAsh.push_texture(basepath + "/64x64/leaf_ash.png");
  TexPetal.push_texture(basepath + "/64x64/petal.png");
  TexSnowflake.push_texture(basepath + "/64x64/snowflake.png");
  TexSimple.push_texture(basepath + "/128x128/simple.png");
  TexFlare.push_texture(basepath + "/128x128/flare1.png");
  TexFlare.push_texture(basepath + "/128x128/flare2.png");
  TexFlare.push_texture(basepath + "/128x128/flare3.png");
  TexVoid.push_texture(basepath + "/128x128/void1.png");
  TexVoid.push_texture(basepath + "/128x128/void2.png");
  TexVoid.push_texture(basepath + "/128x128/void3.png");
  TexTwinflare.push_texture(basepath + "/128x128/twinflare1.png");
  TexTwinflare.push_texture(basepath + "/128x128/twinflare2.png");
  TexTwinflare.push_texture(basepath + "/128x128/twinflare3.png");
  TexTwinflare.push_texture(basepath + "/128x128/twinflare4.png");
  TexTwinflare.push_texture(basepath + "/128x128/twinflare5.png");
  TexInverse.push_texture(basepath + "/128x128/inverse1.png");
  TexInverse.push_texture(basepath + "/128x128/inverse2.png");
  TexInverse.push_texture(basepath + "/128x128/inverse3.png");
  TexInverse.push_texture(basepath + "/128x128/inverse4.png");
  TexShimmer.push_texture(basepath + "/128x128/shimmer1.png");
  TexShimmer.push_texture(basepath + "/128x128/shimmer2.png");
  TexShimmer.push_texture(basepath + "/128x128/shimmer3.png");
  TexCrystal.push_texture(basepath + "/128x128/crystal1.png");
  TexCrystal.push_texture(basepath + "/128x128/crystal2.png");
  TexCrystal.push_texture(basepath + "/128x128/crystal3.png");
  TexWater.push_texture(basepath + "/128x128/water1.png");
  TexWater.push_texture(basepath + "/128x128/water2.png");
  TexWater.push_texture(basepath + "/128x128/water3.png");
  TexLeafMaple.push_texture(basepath + "/128x128/leaf_maple.png");
  TexLeafOak.push_texture(basepath + "/128x128/leaf_oak.png");
  TexLeafAsh.push_texture(basepath + "/128x128/leaf_ash.png");
  TexPetal.push_texture(basepath + "/128x128/petal.png");
  TexSnowflake.push_texture(basepath + "/128x128/snowflake.png");
}

void EyeCandy::push_back_effect(Effect* e)
{
  effects.push_back(e);
}

bool EyeCandy::push_back_particle(Particle* p)
{
  if /*(*/((int)particles.size() >= max_particles)/* || (!allowable_particles_to_add))*/
  {
    delete p;
    return false;
  }
  else
  {
    particles.push_back(p);
    p->effect->register_particle(p);
    light_estimate += p->estimate_light_level();
//    allowable_particles_to_add--;
    return true;
  }
}

void EyeCandy::start_draw()
{
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(false);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  if (draw_method == FAST_BILLBOARDS)
  {
    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
          
    const Vec3 right(modelview[0], modelview[4], modelview[8]);
    const Vec3 up(modelview[1], modelview[5], modelview[9]);
    corner_offset1 = (right + up) * billboard_scalar;
    corner_offset2 = (right - up) * billboard_scalar;
  }

  if (draw_method == POINT_SPRITES)
  {
// Scale
//    glPointSize(100);	// Max on some vidcards
//    float quadratic[] = {0.0f, 0.0f, 0.01f};
//    glPointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic);
//    glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, 200.0f);
//    glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, 1.0f);

    glEnable(GL_POINT_SPRITE_ARB);
    glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
  }
}

void EyeCandy::end_draw()
{
  if (draw_method == POINT_SPRITES)
  {
    glDisable(GL_POINT_SPRITE_ARB);
  }
  
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(true);
  glDisable(GL_COLOR_MATERIAL);
  glEnable(GL_LIGHTING);
}

void EyeCandy::draw()
{
  start_draw();

  // Draw effects (any special drawing functionality) and their particles.
  for (std::vector<Effect*>::const_iterator iter = effects.begin(); iter != effects.end(); iter++)
  {
    Effect* e = *iter;
    if (!e->active)
      continue;
      
    e->draw(time_diff);

    // Draw particles
    for (std::map<Particle*, bool>::const_iterator iter2 = (*iter)->particles.begin(); iter2 != (*iter)->particles.end(); iter2++)
    {
      Particle* p = iter2->first;
      p->draw(time_diff);
    }
  }

  end_draw();
  // Draw lights.
  if (particles.size() > 0)
  {
    while (light_particles.size() < lights.size())
      light_particles.push_back(std::pair<Particle*, float>(particles[randint(particles.size())], 0.0));
    for (int i = 0; i < (int)light_particles.size(); i++)
    {
      Particle* p = light_particles[i].first;
      if (p->get_light_level() < 0.00005)
      {
        int j;
        for (j = 0; j < 40; j++)
        {
          light_particles[i] = std::pair<Particle*, float>(*(particles.begin() + randint(particles.size())), 0.0);
          Particle* p = light_particles[i].first;
          if (p->get_light_level() > 0.0001)
            break;
        }
        if (j == 40)
          continue;
      }
      
      light_particles[i].second += time_diff / 100000.0;
      if (light_particles[i].second >= 1.0)
        light_particles[i].second = 1.0;
      
      const light_t light_level = p->get_light_level() * light_particles[i].second;
      const GLenum light_id = lights[i];
      const GLfloat light_pos[4] = {p->pos.x, p->pos.y, p->pos.z, 1.0};
      const light_t brightness = light_estimate * lighting_scalar * light_level;
      const GLfloat light_color[4] = {p->color[0] * brightness, p->color[1] * brightness, p->color[2] * brightness, 0.0};
      glEnable(light_id);
      glLightfv(light_id, GL_POSITION, light_pos);
      glLightfv(light_id, GL_DIFFUSE, light_color);
    }
  }
  else
  {
    for (int i = 0; i < (int)lights.size(); i++)
      glDisable(lights[i]);	// Save the graphics card some work when rendering the rest of the scene, ne? :)
  }
  
}

void EyeCandy::idle()
{
  const Uint64 cur_time = get_time();
  for (int i = 0; i < (int)effects.size(); )
  {
    std::vector<Effect*>::iterator iter = effects.begin() + i;
    Effect* e = *iter;

//    std::cout << e << ": " << e->get_expire_time() << ", " << cur_time << std::endl;
    if (e->get_expire_time() < cur_time)
    {
      e->recall = true;
    }

    float distance_squared = (camera - *(e->pos)).magnitude_squared();
//    std::cout << camera << ", " << *e->pos << ": " << (camera - *(e->pos)).magnitude_squared() << " <? " << MAX_DRAW_DISTANCE_SQUARED << std::endl;
    if (!e->active)
    {
      if (distance_squared < MAX_DRAW_DISTANCE_SQUARED)
      {
        if (EC_DEBUG)
          std::cout << "Activating effect " << e << "(" << distance_squared << " < " << MAX_DRAW_DISTANCE_SQUARED << ")" << std::endl;
        e->active = true;
      }
      else if (!e->recall)
      {
        i++;
        continue;
      }
    }
    else
    {
      if (distance_squared > MAX_DRAW_DISTANCE_SQUARED)
      {
        if (EC_DEBUG)
          std::cout << "Deactivating effect " << e << "(" << distance_squared << " > " << MAX_DRAW_DISTANCE_SQUARED << ")" << std::endl;
        e->active = false;
      }
    }
    
    const bool ret = e->idle(time_diff);
    if (!ret)
    {
      e->recall = true;
      *(e->dead) = true;
      effects.erase(iter);
      delete e;
    }
    else
      i++;
  }

  // If we're nearing our particle limit, lower our level of detail.
  Uint16 change_LOD;
  if (particles.size() > LOD_1_threshold)
    change_LOD = 1;
  else if (particles.size() > LOD_2_threshold)
    change_LOD = 2;
  else if (particles.size() > LOD_3_threshold)
    change_LOD = 3;
  else if (particles.size() > LOD_4_threshold)
    change_LOD = 4;
  else if (particles.size() > LOD_5_threshold)
    change_LOD = 5;
  else if (particles.size() > LOD_6_threshold)
    change_LOD = 6;
  else if (particles.size() > LOD_7_threshold)
    change_LOD = 7;
  else if (particles.size() > LOD_8_threshold)
    change_LOD = 8;
  else if (particles.size() > LOD_9_threshold)
    change_LOD = 9;
  else
    change_LOD = 10;

  Uint16 change_LOD2;
  if (time_diff > LOD_1_time_threshold)
    change_LOD2 = 1;
  else if (time_diff > LOD_2_time_threshold)
    change_LOD2 = 2;
  else if (time_diff > LOD_3_time_threshold)
    change_LOD2 = 3;
  else if (time_diff > LOD_4_time_threshold)
    change_LOD2 = 4;
  else if (time_diff > LOD_5_time_threshold)
    change_LOD2 = 5;
  else if (time_diff > LOD_6_time_threshold)
    change_LOD2 = 6;
  else if (time_diff > LOD_7_time_threshold)
    change_LOD2 = 7;
  else if (time_diff > LOD_8_time_threshold)
    change_LOD2 = 8;
  else if (time_diff > LOD_9_time_threshold)
    change_LOD2 = 9;
  else
    change_LOD2 = 10;
    
//  std::cout << change_LOD << " / " << change_LOD2 << " (" << allowable_particles_to_add << " remaining)" << std::endl;

  if (change_LOD > change_LOD2)	//Pick whichever one is lower.
    change_LOD = change_LOD2;
  
  for (std::vector<Effect*>::iterator iter = effects.begin(); iter != effects.end(); iter++)
    (*iter)->request_LOD(change_LOD);

  const float particle_cleanout_rate = (1.0 - math_cache.powf_05_close(time_diff / 3000000.0 / change_LOD));
//  std::cout << (1.0 / particle_cleanout_rate) << std::endl;
  float counter = randfloat();
  for (int i = 0; i < (int)particles.size(); )	//Iterate using an int, not an iterator, because we may be adding/deleting entries, and that messes up iterators.
  {
    std::vector<Particle*>::iterator iter = particles.begin() + i;
    Particle* p = *iter;
    
    counter -= particle_cleanout_rate;
    if (counter < 0)	// Kill off a random particle.
    {
      counter++;
      if ((p->deletable()) && (!p->effect->active))
      {
        particles.erase(iter);
        for (int j = 0; j < (int)light_particles.size(); )
        {
          std::vector< std::pair<Particle*, light_t> >::iterator iter2 = light_particles.begin() + j;
          if (iter2->first == p)
          {
            light_particles.erase(iter2);
            continue;
          }
          j++;
        }
        p->effect->unregister_particle(p);
        light_estimate -= p->estimate_light_level();
        delete p;
      }
      
      i++;
      continue;
    }
    
    if ((!p->effect->active) && (!p->effect->recall))
    {
      i++;
      continue;
    }
    const bool ret = p->idle(time_diff);
    if (!ret)
    {
      iter = particles.begin() + i;	//Why the heck do I need to redo this just because I've push_back'ed entries to the vector in idle()?  :P  Makes no sense.  My best guess: array resizing.
      particles.erase(iter);
      for (int j = 0; j < (int)light_particles.size(); )
      {
        std::vector< std::pair<Particle*, light_t> >::iterator iter2 = light_particles.begin() + j;
        if (iter2->first == p)
        {
          light_particles.erase(iter2);
          continue;
        }
        j++;
      }
      p->effect->unregister_particle(p);
      light_estimate -= p->estimate_light_level();
      delete p;
    }
    else
    {
      p->mover->move(*p, time_diff);
      i++;
    }
  }
  last_forced_LOD = change_LOD;
  
//  allowable_particles_to_add = 1 + (int)(particles.size() * 0.00005 * time_diff / 1000000.0 * (max_particles - particles.size()) * change_LOD);
//  std::cout << "Current: " << particles.size() << "; Allowable new: " << allowable_particles_to_add << std::endl;
  
}

void EyeCandy::add_light(GLenum light_id)
{
  glDisable(light_id);
  lights.push_back(light_id);
  GLfloat light_pos[4] = {0.0, 0.0, 0.0, 1.0};
  const GLfloat light_white[4] = {0.0, 0.0, 0.0, 0.0};
  glLightfv(light_id, GL_SPECULAR, light_white);
  glLightfv(light_id, GL_POSITION, light_pos);
  glLightf(light_id, GL_LINEAR_ATTENUATION, 1.0);
}

void EyeCandy::draw_point_sprite_particle(coord_t size, const GLuint texture, const color_t r, const color_t g, const color_t b, const alpha_t alpha, const Vec3 pos)
{
//  std::cout << "A: " << size << ", " << texture << ", " << Vec3(r, g, b) << ", " << alpha << std::endl;
  glPointSize(size);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBegin(GL_POINTS);
  {
    glColor4f(r, g, b, alpha);
    glVertex3d(pos.x, pos.y, pos.z);
  }
  glEnd();
}

void EyeCandy::draw_fast_billboard_particle(coord_t size, const GLuint texture, const color_t r, const color_t g, const color_t b, const alpha_t alpha, const Vec3 pos)
{
//  std::cout << "B: " << size << ", " << texture << ", " << Vec3(r, g, b) << ", " << alpha << std::endl;
  const Vec3 corner1(pos - corner_offset1 * size);
  const Vec3 corner2(pos + corner_offset2 * size);
  const Vec3 corner3(pos + corner_offset1 * size);
  const Vec3 corner4(pos - corner_offset2 * size);
  
//  std::cout << corner1 << ", " << corner2 << ", " << corner3 << ", " << corner4 << std::endl;
//  std::cout << size << ", " << texture << ", " << Vec3(r, g, b) << ", " << alpha << std::endl;

  glBindTexture(GL_TEXTURE_2D, texture);
  glBegin(GL_QUADS);
  {
    glColor4f(r, g, b, alpha);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(corner1.x, corner1.y, corner1.z);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(corner2.x, corner2.y, corner2.z);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(corner3.x, corner3.y, corner3.z);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(corner4.x, corner4.y, corner4.z);
  }
  glEnd();
}

void EyeCandy::draw_accurate_billboard_particle(coord_t size, const GLuint texture, const color_t r, const color_t g, const color_t b, const alpha_t alpha, const Vec3 pos)
{
  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z);

  Vec3 object_to_camera_projection(camera.x - pos.x, 0, camera.z - pos.z);
  object_to_camera_projection.normalize();

  const Vec3 look_at(0, 0, 1);
  const Vec3 up = look_at.cross(object_to_camera_projection);
  const float cos_angle = look_at.dot(object_to_camera_projection);

  if ((cos_angle < 0.9999) && (cos_angle > -0.9999))
    glRotatef(acos(cos_angle) * 180.0 / PI, up.x, up.y, up.z);
      
  Vec3 object_to_camera = camera - pos;
  object_to_camera.normalize();
  const float cos_angle2 = object_to_camera_projection.dot(object_to_camera);

  if ((cos_angle2 < 0.9999) && (cos_angle2 > -0.9999))
  {
    if (object_to_camera.y < 0)
      glRotatef(acos(cos_angle2) * 180.0 / PI, 1, 0, 0);    
    else
      glRotatef(acos(cos_angle2) * 180.0 / PI, -1, 0, 0);   
  }

  const float scaled_size = size * billboard_scalar;
  glBindTexture(GL_TEXTURE_2D, texture);
/*
  glBegin(GL_QUADS);
  {
    glColor4f(r, g, b, alpha);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(pos.x - scaled_size, pos.y - scaled_size, pos.z);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(pos.x + scaled_size, pos.y - scaled_size, pos.z);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(pos.x + scaled_size, pos.y + scaled_size, pos.z);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(pos.x - scaled_size, pos.y + scaled_size, pos.z);
  }
  glEnd();
*/
  glBegin(GL_QUADS);
  {
    glColor4f(r, g, b, alpha);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-scaled_size, -scaled_size, 0.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(scaled_size, -scaled_size, 0.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(scaled_size, scaled_size, 0.0);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-scaled_size, scaled_size, 0.0);
  }
  glEnd();
  glPopMatrix();
}

// F U N C T I O N S //////////////////////////////////////////////////////////

int randint(const int upto)
{
  return rand() % upto;
}

Uint8 rand8()
{
  return (Uint8)rand();
}

Uint16 rand16()
{
#if RAND_MAX >= 0xFFFF
  return (Uint16)rand();
#elif RAND_MAX > 0xFF
  return (((Uint16)rand()) << 8) | (Uint16)rand();
#else
  return (((Uint16)rand8()) << 8) | (Uint16)rand8();
#endif
}

Uint32 rand32()
{
#if RAND_MAX >= 0xFFFFFFFF
  return (Uint32)rand();
#elif RAND_MAX > 0xFFFF
  return (((Uint32)rand()) << 16) | (Uint32)rand();
#else
  return (((Uint32)rand16()) << 16) | (Uint32)rand16();
#endif
}

Uint64 rand64()
{
#if RAND_MAX >= 0xFFFFFFFFFFFFFFFF
  return (Uint64)rand();
#elif RAND_MAX > 0xFFFFFFFF
  return (((Uint64)rand()) << 32) | (Uint64)rand();
#else
  return (((Uint64)rand16()) << 32) | (Uint64)rand16();
#endif
}

Uint8 rand7()
{
  return (Uint8)rand();
}

Uint16 rand15()
{
#if RAND_MAX >= 0x8FFF
  return (Uint16)rand();
#elif RAND_MAX > 0xFF
  return (((Uint16)rand()) << 8) | (Uint16)rand();
#else
  return (((Uint16)rand8()) << 8) | (Uint16)rand8();
#endif
}

Uint32 rand31()
{
#if RAND_MAX >= 0x8FFFFFFF
  return (Uint32)rand();
#elif RAND_MAX > 0xFFFF
  return (((Uint32)rand()) << 16) | (Uint32)rand();
#else
  return (((Uint32)rand16()) << 16) | (Uint32)rand16();
#endif
}

Uint64 rand63()
{
#if RAND_MAX >= 0x8FFFFFFFFFFFFFFF
  return (Uint64)rand();
#elif RAND_MAX > 0xFFFFFFFF
  return (((Uint64)rand()) << 32) | (Uint64)rand();
#else
  return (((Uint64)rand16()) << 32) | (Uint64)rand16();
#endif
}

double randdouble()
{
  return (double)rand() / (double)RAND_MAX;
}

float randfloat()
{
  return (float)rand() / (float)RAND_MAX;
}

double randdouble(const double scale)
{
  return scale * randdouble();
}

float randfloat(const float scale)
{
  return scale * randfloat(); 
}

coord_t randcoord(void)
{
  if (sizeof(coord_t) == 4)	// Compiler should optimize this out.
    return (coord_t)randfloat();
  else
    return (coord_t)randdouble();
}

coord_t randcoord(const coord_t scale)
{
  return scale * randcoord();
}

color_t randcolor(void)
{
  if (sizeof(color_t) == 4)
    return (color_t)randfloat();
  else
    return (color_t)randdouble();
}

color_t randcolor(const color_t scale)
{
  return scale * randcolor();
}

alpha_t randalpha(void)
{
  if (sizeof(alpha_t) == 4)
    return (alpha_t)randfloat();
  else
    return (alpha_t)randdouble();
}

alpha_t randalpha(const alpha_t scale)
{
  return scale * randalpha();
}

energy_t randenergy(void)
{
  if (sizeof(energy_t) == 4)
    return (energy_t)randfloat();
  else
    return (energy_t)randdouble();
}

energy_t randenergy(const energy_t scale)
{
  return scale * randenergy();
}

light_t randlight(void)
{
  if (sizeof(light_t) == 4)
    return (light_t)randfloat();
  else
    return (light_t)randdouble();
}

light_t randlight(const light_t scale)
{
  return scale * randlight();
}

percent_t randpercent(void)
{
  if (sizeof(percent_t) == 4)
    return (percent_t)randfloat();
  else
    return (percent_t)randdouble();
}

percent_t randpercent(const percent_t scale)
{
  return scale * randpercent();
}

angle_t randangle(void)
{
  if (sizeof(angle_t) == 4)
    return (angle_t)randfloat();
  else
    return (angle_t)randdouble();
}

angle_t randangle(const angle_t scale)
{
  return scale * randangle();
}

double square(const double d)
{
  return d * d;
}

float square(const float f)
{
  return f * f;
}

int square(const int i)
{
  return i * i;
}

double cube(const double d)
{
  return d * d * d;
}

float cube(const float f)
{
  return f * f * f;
}

int cube(const int i)
{
  return i * i * i;
}

float fastsqrt(float f)	// This could probably stand to be faster; use invsqrt wherever possible.
{
  return 1.0 / invsqrt(f);
}

#ifdef WINDOWS
__declspec(noinline) float invsqrt_workaround(int i)
#else
__attribute__ ((noinline)) float invsqrt_workaround(int i)
#endif
{
  return *(float*)((void*)&i);
}

#ifdef WINDOWS
__declspec(noinline) float invsqrt(float f)	// The famous Quake3 inverse square root function.  About 4x faster in my benchmarks!
#else
__attribute__ ((noinline)) float invsqrt(float f)	// The famous Quake3 inverse square root function.  About 4x faster in my benchmarks!
#endif
{
  float half = 0.5f * f;
  int i = *(int*)((void*)&f);
  i = 0x5f3759df - (i >> 1);
  f = invsqrt_workaround(i);
  f = f * (1.5f - half * f * f);
  return f;
}

Uint64 get_time()
{
#ifdef WINDOWS
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  Uint64 ret = ft.dwHighDateTime;
  ret <<= 32;
  ret |= ft.dwLowDateTime;
  return ret;
#else
  struct timeval t;
  gettimeofday(&t, NULL);
  return ((Uint64)t.tv_sec)*1000000ul + (Uint64)t.tv_usec;
#endif
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef SFX
