//
// Coherent noise function over 1, 2 or 3 dimensions
// (copyright Ken Perlin)
//


#include "noise.h"
#include "../errors.h"
#include "../load_gl_extensions.h"

#define MAXB 0x100
#define N 0x1000
#define NP 12   // 2^N
#define NM 0xfff

static int p[MAXB + MAXB + 2];
static double g3[MAXB + MAXB + 2][3];

int start;
int noise_B;
int noise_BM;

static __inline__ void normalize3(double v[3])
{
	double s;

	s = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}

static __inline__ void init_noise()
{
	int i, j, k;

	srand(30757);

	for (i = 0; i < noise_B; i++)
	{
		p[i] = i;
		for (j = 0; j < 3; j++)
		{
			g3[i][j] = (double)((rand() % (noise_B + noise_B)) - noise_B) / noise_B;
		}
		normalize3(g3[i]);
	}

	for (i = 0; i < noise_B; i++)
	{
		j = rand() % noise_B;

		k = p[i];
		p[i] = p[j];
		p[j] = k;
	}

	for (i = 0; i < noise_B + 2; i++)
	{
		p[noise_B + i] = p[i];
		for (j = 0; j < 3; j++)
		{
			g3[noise_B + i][j] = g3[i][j];
		}
	}
}

static __inline__ double s_curve(double t)
{
	return t * t * (3.0 - 2.0 * t);
}

static __inline__ double lerp(double t, double a, double b)
{
	return a + t * (b - a);
}

static __inline__ void setup(double v, int *b0, int *b1, double *r0, double *r1)
{
	double t;

        t = v + N;
        *b0 = ((int)t) & noise_BM;
        *b1 = (*b0 + 1) & noise_BM;
        *r0 = t - (int)t;
        *r1 = *r0 - 1.0;
}

static __inline__ double at3(double q[3], double rx, double ry, double rz)
{
	return rx * q[0] + ry * q[1] + rz * q[2];
}

static __inline__ void set_noise_frequency(int frequency)
{
	start = 1;
	noise_B = frequency;
	noise_BM = noise_B-1;
}

static __inline__ double dnoise3(double vec[3], int offset)
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	double rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
	int i, j;

	if (start)
	{
		start = 0;
		init_noise();
	}

	setup(vec[0], &bx0, &bx1, &rx0, &rx1);
	setup(vec[1], &by0, &by1, &ry0, &ry1);
	setup(vec[2], &bz0, &bz1, &rz0, &rz1);

	i = p[bx0];
	j = p[bx1];

	b00 = p[i + by0] + offset;
	b10 = p[j + by0] + offset;
	b01 = p[i + by1] + offset;
	b11 = p[j + by1] + offset;

	t  = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);

	q = g3[b00 + bz0];
	u = at3(q, rx0, ry0, rz0);
	q = g3[b10 + bz0];
	v = at3(q, rx1, ry0, rz0);
	a = lerp(t, u, v);

	q = g3[b01 + bz0];
	u = at3(q, rx0, ry1, rz0);
	q = g3[b11 + bz0];
	v = at3(q, rx1, ry1, rz0);
	b = lerp(t, u, v);

	c = lerp(sy, a, b);

	q = g3[b00 + bz1];
	u = at3(q, rx0, ry0, rz1);
	q = g3[b10 + bz1];
	v = at3(q, rx1, ry0, rz1);
	a = lerp(t, u, v);

	q = g3[b01 + bz1];
	u = at3(q, rx0, ry1, rz1);
	q = g3[b11 + bz1];
	v = at3(q, rx1, ry1, rz1);
	b = lerp(t, u, v);

	d = lerp(sy, a, b);

	return lerp(sz, c, d);
}

static __inline__ GLubyte* make_3d_noise_texture(int size, int frequency, int dimensions)
{
	int f, i, j, k, l;
	int numOctaves = 4;
	double ni[3];
	double inci, incj, inck;
	double* tmp;
	double* ptr;
	GLubyte* data;
	double amp = 0.5;

	data = (GLubyte*)malloc(size * size * size * dimensions);
	tmp = (double*)malloc(size * size * size * dimensions * sizeof(double));
	memset(tmp, 0, size * size * size * dimensions * sizeof(double));

	for (f = 0; f < numOctaves; f++, frequency *= 2, amp *= 0.5)
	{
		LOG_DEBUG("Generating 3D noise: octave %d/%d...", f + 1, numOctaves);
		set_noise_frequency(frequency);
		ni[0] = ni[1] = ni[2] = 0;

		ptr = tmp;

		inci = 1.0 / (size / frequency);
		for (i = 0; i < size; i++, ni[0] += inci)
		{
			incj = 1.0 / (size / frequency);
			for (j = 0; j < size; j++, ni[1] += incj)
			{
				inck = 1.0 / (size / frequency);
				for (k = 0; k < size; k++, ni[2] += inck, ptr += dimensions)
				{
					for (l = 0; l < dimensions; l++)
					{
						ptr[l] += (dnoise3(ni, l) + 1.0) * amp * 127.5;
					}
				}
			}
		}
	}

	LOG_DEBUG("Compressing noise");
	for (i = 0; i < size * size * size * dimensions; i++)
	{
		data[i] = tmp[i];
	}

	free(tmp);

	return data;
}

GLuint build_3d_noise_texture(int size, int frequency, int dimensions)
{
	GLubyte* data;
	GLuint noise_texture;
	GLenum texture_format, input_format;

	ENTER_DEBUG_MARK("build noise");

	switch (dimensions)
	{
		case 1:
			if (have_extension(ext_texture_compression_latc))
			{
				texture_format = GL_COMPRESSED_LUMINANCE_LATC1_EXT;
			}
			else
			{
				texture_format = GL_LUMINANCE;
			}
			input_format = GL_LUMINANCE;
			break;
		case 2:
			if (have_extension(ext_texture_compression_latc))
			{
				texture_format = GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
			}
			else
			{
				if (have_extension(ati_texture_compression_3dc))
				{
					texture_format = GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI;
				}
				else
				{
					texture_format = GL_LUMINANCE_ALPHA;
				}
			}
			input_format = GL_LUMINANCE_ALPHA;
			break;
		case 3:
			texture_format = GL_RGB;
			input_format = GL_RGB;
			break;
		default:
			LEAVE_DEBUG_MARK("build noise");

			return 0;
	}

	data = make_3d_noise_texture(size, frequency, dimensions);

	LOG_DEBUG("Building noise texture");
	glGenTextures(1, &noise_texture);
	glBindTexture(GL_TEXTURE_3D, noise_texture);

	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	ELglTexImage3D(GL_TEXTURE_3D, 0, texture_format, size, size, size, 0, input_format, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_3D, 0);
	free(data);
	LOG_DEBUG("Done with noise");

	LEAVE_DEBUG_MARK("build noise");

	return noise_texture;
}

