#include "optimizer.hpp"
#include <set>
#include <vector>

struct tri_data
{
	bool added;
	float score;
	Uint32 verts[3];
};

struct vert_data
{
	float score;
	std::set<Uint32> remaining_tris;
};

#ifdef	USE_BOOST
float calculate_average_cache_miss_ratio(const boost::shared_array<Uint32> &indices,
	const Uint32 offset, const Uint32 count, const Uint32 cache_size)
#else	/* USE_BOOST */
float calculate_average_cache_miss_ratio(const Uint32* indices, const Uint32 offset,
	const Uint32 count, const Uint32 cache_size)
#endif	/* USE_BOOST */
{
	std::vector<Uint32> cache(cache_size, 0xFFFFFFFF);
	Uint32 i, j, cache_ptr, cache_misses;
	bool cache_hit;

	if (count <= cache_size)
	{
		return -1.0f;
	}

	cache_ptr = 0;
	cache_misses = 0;

	for (i = 0 ; i < count; i++)
	{
		cache_hit = false;

		for (j = 0 ; j < cache_size; j++)
		{
			cache_hit |= (cache[j] == indices[offset + i]);
		}

		if (!cache_hit)
		{
			cache[cache_ptr] = indices[offset + i];
			cache_ptr = (cache_ptr + 1) % cache_size;
			cache_misses++;
		}
	}

	return (cache_misses / (count / 3.0f));
}

static const float score_table[33] =
{
	0.0f,
	2.0f / sqrt(1.0f), 2.0f / sqrt(2.0f), 2.0f / sqrt(3.0f), 2.0f / sqrt(4.0f),
	2.0f / sqrt(5.0f), 2.0f / sqrt(6.0f), 2.0f / sqrt(7.0f), 2.0f / sqrt(8.0f),
	2.0f / sqrt(9.0f), 2.0f / sqrt(10.0f), 2.0f / sqrt(11.0f), 2.0f / sqrt(12.0f),
	2.0f / sqrt(13.0f), 2.0f / sqrt(14.0f), 2.0f / sqrt(15.0f), 2.0f / sqrt(16.0f),
	2.0f / sqrt(17.0f), 2.0f / sqrt(18.0f), 2.0f / sqrt(19.0f), 2.0f / sqrt(20.0f),
	2.0f / sqrt(21.0f), 2.0f / sqrt(22.0f), 2.0f / sqrt(23.0f), 2.0f / sqrt(24.0f),
	2.0f / sqrt(25.0f), 2.0f / sqrt(26.0f), 2.0f / sqrt(27.0f), 2.0f / sqrt(28.0f),
	2.0f / sqrt(29.0f), 2.0f / sqrt(30.0f), 2.0f / sqrt(31.0f), 2.0f / sqrt(32.0f)
};

static inline float calc_new_score(const Uint32 count)
{
	return 12.0f;//score_table[std::min(count, 32u)];
}

#ifdef	USE_BOOST
bool optimize_vertex_cache_order(boost::shared_array<Uint32> &tri_indices, const Uint32 offset,
	const Uint32 count, const Uint32 cache_size)
#else	/* USE_BOOST */
bool optimize_vertex_cache_order(Uint32* tri_indices, const Uint32 offset,
	const Uint32 count, const Uint32 cache_size)
#endif	/* USE_BOOST */
{
	std::vector<float> cache_score(cache_size + 3, 0.75);
	std::vector<Uint32> cache_idx(cache_size + 3, 0xFFFFFFFF);
	std::vector<Uint32> grow_cache_idx(cache_size + 3, 0xFFFFFFFF);
	std::set<Uint32>::iterator it;
	Uint32 i, j, index;
	Uint32 num_triangles, num_vertices;
	Uint32 tris_left, best_idx;
	Uint32 a, b, c;
	float best_score, old_score, new_score;

	if ((count < 3) || (count % 3 != 0) || (cache_size < 4))
	{
		return false;
	}

	num_triangles = count / 3;
	num_vertices = 0;

	for (i = 0; i < count; i++)
	{
		if (tri_indices[offset + i] > num_vertices)
		{
			num_vertices = tri_indices[offset + i];
		}
	}
	num_vertices++;

	for (i = 3; i < cache_size; i++)
	{
		cache_score[i] = powf((cache_size - i) / (cache_size - 3.0), 1.5);
	}

	for (i = 0; i < 3; i++)
	{
		cache_score[cache_size + i] = 0.0;
	}
	tris_left = num_triangles;

	std::vector<tri_data> t(num_triangles);
	std::vector<vert_data> v(num_vertices);

	for (i = 0; i < num_vertices; i++)
	{
		v[i].score = 0.0;
		v[i].remaining_tris.clear();
	}
	for (i = 0; i < num_triangles; i++)
	{
		t[i].added = false;
		t[i].score = 0.0;
		t[i].verts[0] = tri_indices[offset + i * 3 + 0];
		t[i].verts[1] = tri_indices[offset + i * 3 + 1];
		t[i].verts[2] = tri_indices[offset + i * 3 + 2];
		v[tri_indices[offset + i * 3 + 0]].remaining_tris.insert(i);
		v[tri_indices[offset + i * 3 + 1]].remaining_tris.insert(i);
		v[tri_indices[offset + i * 3 + 2]].remaining_tris.insert(i);
	}
	for (i = 0; i < num_vertices; i++)
	{
		v[i].score = calc_new_score(v[i].remaining_tris.size());
	}
	for (i = 0; i < num_triangles; i++)
	{
		t[i].score = v[t[i].verts[0]].score + v[t[i].verts[1]].score + v[t[i].verts[2]].score;
	}

	while (tris_left > 0)
	{
		best_score = 0.0;
		best_idx = 0xFFFFFFFF;
		for (i = 0; i < num_triangles; i++)
		{
			if (!t[i].added)
			{
				if (t[i].score > best_score)
				{
					best_score = t[i].score;
					best_idx = i;
				}
			}
		}
		a = t[best_idx].verts[0];
		b = t[best_idx].verts[1];
		c = t[best_idx].verts[2];
		tri_indices[offset + (num_triangles - tris_left) * 3 + 0] = a;
		tri_indices[offset + (num_triangles - tris_left) * 3 + 1] = b;
		tri_indices[offset + (num_triangles - tris_left) * 3 + 2] = c;
		for (i = 0; i < 3; i++)
		{
			index = t[best_idx].verts[i];
			v[index].remaining_tris.erase(best_idx);
		}
		t[best_idx].added = true;
		tris_left--;
		grow_cache_idx[0] = a;
		grow_cache_idx[1] = b;
		grow_cache_idx[2] = c;
		j = 3;
		for (i = 0; i < cache_size; i++)
		{
			grow_cache_idx[i + 3] = 0xFFFFFFFF;
			if ((cache_idx[i] != a) && (cache_idx[i] != b) &&
				(cache_idx[i] != c))
			{
				grow_cache_idx[j++] = cache_idx[i];
			}
		}
		cache_idx.swap(grow_cache_idx);
		for (i = 0; i < cache_size; i++)
		{
			if (cache_idx[i] < 0xFFFFFFFF)
			{
				index = cache_idx[i];
				if (v[index].remaining_tris.size() > 0)
				{
					old_score = v[index].score;
					new_score = cache_score[i] +
						calc_new_score(v[index].remaining_tris.size());
					v[index].score = new_score;
					for (auto tri: v[index].remaining_tris)
					{
						t[tri].score += new_score - old_score;
					}
				}
			}
		}
	}
	return true;
}

