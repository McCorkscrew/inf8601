/*
 * dragon_tbb.c
 *
 *  Created on: 2011-08-17
 *      Author: Francis Giraldeau <francis.giraldeau@gmail.com>
 */

#include <iostream>

extern "C" {
#include "dragon.h"
#include "color.h"
#include "utils.h"
}
#include "dragon_tbb.h"
#include "tbb/tbb.h"
#include "TidMap.h"

using namespace std;
using namespace tbb;

class DragonLimits {
	piece_t* piece;
public:
	DragonLimits(piece_t *p);
	void operator() (const blocked_range<uint64_t>& range) {
		
		xy_t *position = &piece->position;
		xy_t *orientation = &piece->orientation;
		xy_t *minimums = &piece->limits.minimums;
		xy_t *maximums = &piece->limits.maximums;
		for (uint64_t n = range.begin(); n <= range.end(); n++) {
			position->x += orientation->x;
			position->y += orientation->y;

			if (((n & -n) << 1) & n)
				rotate_left(orientation);
			else
				rotate_right(orientation);
			if (minimums->x > position->x) minimums->x = position->x;
			if (minimums->y > position->y) minimums->y = position->y;
			if (maximums->x < position->x) maximums->x = position->x;
			if (maximums->y < position->y) maximums->y = position->y;
		}
	}


	DragonLimits(DragonLimits&s,tbb::split);
	void join(DragonLimits& rhs) {piece_merge(piece, *rhs.piece);}

	

};

DragonLimits::DragonLimits(piece_t *p){
	piece = p;
}

DragonLimits::DragonLimits(DragonLimits&s,tbb::split){
	piece = new piece_t;
	piece_init(piece);
}



class DragonDraw {
};

class DragonRender {
};

class DragonClear {
};

int dragon_draw_tbb(char **canvas, struct rgb *image, int width, int height, uint64_t size, int nb_thread)
{
	TODO("dragon_draw_tbb");
	struct draw_data data;
	limits_t limits;
	char *dragon = NULL;
	int dragon_width;
	int dragon_height;
	int dragon_surface;
	int scale_x;
	int scale_y;
	int scale;
	int deltaJ;
	int deltaI;

	struct palette *palette = init_palette(nb_thread);
	if (palette == NULL)
		return -1;

	/* 1. Calculer les limites du dragon */
	dragon_limits_tbb(&limits, size, nb_thread);

	task_scheduler_init init(nb_thread);

	dragon_width = limits.maximums.x - limits.minimums.x;
	dragon_height = limits.maximums.y - limits.minimums.y;
	dragon_surface = dragon_width * dragon_height;
	scale_x = dragon_width / width + 1;
	scale_y = dragon_height / height + 1;
	scale = (scale_x > scale_y ? scale_x : scale_y);
	deltaJ = (scale * width - dragon_width) / 2;
	deltaI = (scale * height - dragon_height) / 2;

	dragon = (char *) malloc(dragon_surface);
	if (dragon == NULL) {
		free_palette(palette);
		return -1;
	}

	data.nb_thread = nb_thread;
	data.dragon = dragon;
	data.image = image;
	data.size = size;
	data.image_height = height;
	data.image_width = width;
	data.dragon_width = dragon_width;
	data.dragon_height = dragon_height;
	data.limits = limits;
	data.scale = scale;
	data.deltaI = deltaI;
	data.deltaJ = deltaJ;
	data.palette = palette;
	data.tid = (int *) calloc(nb_thread, sizeof(int));

	/* 2. Initialiser la surface : DragonClear */

	/* 3. Dessiner le dragon : DragonDraw */

	/* 4. Effectuer le rendu final */
	
	init.terminate();

	free_palette(palette);
	FREE(data.tid);
	//*canvas = dragon;
	*canvas = NULL;
	return 0;
}

/*
 * Calcule les limites en terme de largeur et de hauteur de
 * la forme du dragon. Requis pour allouer la matrice de dessin.
 */
int dragon_limits_tbb(limits_t *limits, uint64_t size, int nb_thread)
{
	//TODO("dragon_limits_tbb");

	piece_t piece;
	piece_init(&piece);
	
	DragonLimits lim(&piece);

	parallel_reduce (blocked_range<uint64_t>(1, size), lim);


	*limits = piece.limits;
	return 0;
}
