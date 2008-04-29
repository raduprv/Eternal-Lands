#ifdef CLUSTER_INSIDES

#include "cluster.h"
#include "actors.h"

static short* clusters = NULL;

short current_cluster = 0;

void set_clusters (const char* data)
{
	const short* cdata = (const short*) data;
	int nx = tile_map_size_x * 6;
	int ny = tile_map_size_y * 6;
	int idx;

	clusters = calloc (nx * ny, sizeof (short));
	for (idx = 0; idx < nx*ny; idx++)
		clusters[idx] = SDL_SwapLE16 (cdata[idx]);	
}

#ifdef MAP_EDITOR
void get_clusters (char** data, int *len)
{
	if (!clusters)
	{
		*data = NULL;
		*len = 0;
	}
	else
	{
		int nx = tile_map_size_x * 6;
		int ny = tile_map_size_y * 6;
		int idx;
		short* cdata = calloc (nx*ny, sizeof (short));

		for (idx = 0; idx < nx*ny; idx++)
			cdata[idx] = SDL_SwapLE16 (clusters[idx]);

		*len = nx * ny * sizeof (short);
		*data = (char *) cdata;
	}
}
#endif

void compute_clusters (const char* occupied) 
{
	int nr_clusters;
	short cluster_idx[1024];
	short nb_idx[4];
	int ic, cnr;

	int nx = tile_map_size_x * 6;
	int ny = tile_map_size_y * 6;
	int x, y, idx;

	clusters = calloc (nx * ny, sizeof (short));
	
	nr_clusters = 0;
	cluster_idx[0] = 0;
	idx = 0;
	if (occupied[idx])
	{
		nr_clusters++;
		clusters[idx] = cluster_idx[nr_clusters] = nr_clusters;
	}

	for (++idx; idx < nx; idx++)
	{
		if (occupied[idx])
		{
			if (occupied[idx-1])
			{
				clusters[idx] = clusters[idx-1];
			}
			else
			{
				nr_clusters++;
				clusters[idx] = cluster_idx[nr_clusters] = nr_clusters;
			}
		}
	}

	for (y = 1; y < ny; y++)
	{
		for (x = 0; x < nx; x++, idx++)
		{
			if (occupied[idx])
			{
				int cidx, i;

				nb_idx[0] = x > 0 ? cluster_idx[clusters[idx-nx-1]] : 0;
				nb_idx[1] = cluster_idx[clusters[idx-nx]];
				nb_idx[2] = x < nx-1 ? cluster_idx[clusters[idx-nx+1]] : 0;
				nb_idx[3] = x > 0 ? cluster_idx[clusters[idx-1]] : 0;

				cidx = 0;
				for (i = 0; i < 4; i++)
					if (nb_idx[i] && (!cidx || nb_idx[i] < cidx))
						cidx = nb_idx[i];

				if (!cidx)
				{
					nr_clusters++;
					clusters[idx] = cluster_idx[nr_clusters] = nr_clusters;
				}
				else
				{
					clusters[idx] = cidx;
					for (i = 0; i < 4; i++)
						if (nb_idx[i] && cluster_idx[nb_idx[i]] > cidx)
							cluster_idx[nb_idx[i]] = cidx;
				}
			}
		}
	}

	cnr = 0;
	for (ic = 1; ic <= nr_clusters; ic++)
	{
		if (cluster_idx[ic] == ic)
			cluster_idx[ic] = ++cnr;
		else
			cluster_idx[ic] = cluster_idx[cluster_idx[ic]];
	}

	for (idx = 0; idx < nx*ny; idx++)
		clusters[idx] = cluster_idx[clusters[idx]];
}

short get_cluster (int x, int y)
{
	if (clusters == NULL)
		return 0;
	if (x < 0 || x >= tile_map_size_x*6 || y < 0 || y >= tile_map_size_y*6)
		return 0;
	return clusters[y*tile_map_size_x*6+x];
}

void destroy_clusters_array ()
{
	if (clusters)
		free (clusters);
	clusters = NULL;
}

#ifndef MAP_EDITOR
short get_actor_cluster ()
{
	actor *me = get_our_actor ();
	return me ? me->cluster : 0;
}
#endif

#endif // CLUSTER_INSIDES

