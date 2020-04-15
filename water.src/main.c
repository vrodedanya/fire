#include <stdlib.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>

#define GRAVITY 9.8
#define PARTICLE_SIZE 20


typedef struct
{
	SDL_Rect rect;
	double x;
	double y;
	int energy;
	double speed_x;
	double speed_y;
}Particles;

Particles* p = NULL;
int SIZE = 0;

Particles* check_space(Particles part, int ignore)
{
	for (register int i = 0 ; i < SIZE ; ++i)
	{
		if (ignore == i) continue;
		if (part.rect.x < p[i].rect.x + p[i].rect.w && part.rect.x + part.rect.w > p[i].rect.x 
		&& part.rect.y < p[i].rect.y + p[i].rect.h && part.rect.y + part.rect.h > p[i].rect.y)
		{
			return &p[i];
		}
	}
	return NULL;
}

void push(Particles particle)
{
	if (check_space(particle, -1) == NULL)
	{
		++SIZE;
		p = realloc(p, sizeof(Particles) * SIZE);
		p[SIZE - 1] = particle;
	}
}

void draw_particles(SDL_Renderer* renderer)
{
	for (register int i = 0 ; i < SIZE ; ++i)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 0);
		SDL_RenderDrawRect(renderer,&p[i].rect);
	}
}

// The function perfom all motions of objects (a fall and e.t.c.)
void move(double delta)
{
	for (register int i = 0 ; i < SIZE ; ++i)
	{
		Particles ghost;
		ghost.rect.w = p[i].rect.w;
		ghost.rect.h = p[i].rect.h;
		// Creating 4 variables for check free space in all directions
		ghost.rect.y = p[i].rect.y;
		ghost.rect.x = p[i].rect.x + 1;
		Particles* check_right = check_space(ghost,i);
		ghost.rect.x = p[i].rect.x - 1;
		Particles* check_left = check_space(ghost,i);
		ghost.rect.x = p[i].rect.x;
		ghost.rect.y = p[i].rect.y + 1;
		Particles* check_down = check_space(ghost,i);
		ghost.rect.y = p[i].rect.y - 1;
		Particles* check_up = check_space(ghost,i);
		// Gravitation	
		if (check_down == NULL && p[i].rect.y < 450)
		{
			p[i].speed_y++;
		}
		else
		{
			p[i].speed_y = 0;
			p[i].y = p[i].rect.y;
		}

		// Pressure
		if (check_up == NULL)
		{
		}
	}
}


int main()
{
	SDL_Window* window = SDL_CreateWindow("Water", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer =SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Event event;

	srand(time(NULL));

	time_t start_time = clock(),stop_time = clock();
	double delta;

	while (1)
	{
		delta = (double)(stop_time - start_time) / CLOCKS_PER_SEC;
		start_time = clock();
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		move(delta);
		draw_particles(renderer);

		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN || event.type == SDL_QUIT) return 0; 
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				Particles particle;
				particle.rect.w = PARTICLE_SIZE;
				particle.rect.h = PARTICLE_SIZE;
				particle.rect.x = particle.x = event.motion.x - PARTICLE_SIZE / 2;
				particle.rect.y = particle.y = event.motion.y - PARTICLE_SIZE / 2;

				particle.energy = 450 - event.motion.y;
				particle.speed_x = 0;
				particle.speed_y = 0;
				push(particle);
			}
		}
		SDL_RenderPresent(renderer);
		stop_time = clock();
	}
	return 0;
}
