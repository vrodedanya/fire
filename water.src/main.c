#include <stdlib.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>

#define GRAVITY 9.8
#define PARTICLE_SIZE 10


typedef struct
{
	SDL_Rect rect;
	double x;
	double y;
	int pressure;
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
		SDL_RenderFillRect(renderer,&p[i].rect);
	}
}

void move(double delta)
{
	for (register int i = 0 ; i < SIZE ; ++i)
	{
		p[i].x += p[i].speed_x * delta;
		p[i].y += p[i].speed_y * delta;
		p[i].rect.x = p[i].x;
		p[i].rect.y = p[i].y;
		
		Particles ghost;
		ghost.rect.w = p[i].rect.w;
		ghost.rect.h = p[i].rect.h;

		ghost.rect.x = p[i].x + p[i].speed_x * delta;
		ghost.rect.y = p[i].y;
		Particles* check_x = check_space(ghost, i); 

		ghost.rect.x = p[i].x;
		ghost.rect.y = p[i].y + p[i].speed_y * delta;
		Particles* check_y = check_space(ghost, i); 

		if (p[i].rect.y < 450 && check_y == NULL)
		{
			p[i].speed_y += pow(GRAVITY, 3) * delta;
		}
		else
		{
			p[i].speed_y = 0;
		}
		if (check_y != NULL)
		{
			check_y->pressure++;
		}
		if (check_x == NULL)
		{
			if (p[i].pressure > 0) p[i].speed_x += (-p[i].pressure + rand()%p[i].pressure) * delta;
			printf("%d %d\n",i,p[i].pressure);
		}
	}
}


int main()
{
	SDL_Window* window = SDL_CreateWindow("Water", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer =SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Event event;


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

				particle.pressure = 0;
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
