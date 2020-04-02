#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>

#define PART_QUANT 640
#define THREADS 8

typedef struct
{
	double x;
	double y;
	int t;
	int e;
}Particle;

typedef struct
{
	Particle* p;
	int start;
	int end;

}particle_storage;

typedef struct
{
	particle_storage pd;
	SDL_Renderer* renderer;
}render_storage;

void spawn_particles(Particle* p, int center_x, int center_y, int radius_x, int radius_y, int exclude_radius)
{
	register int radius_buffer;
	register double angle;
	for (register int i = 0 ; i < PART_QUANT ; ++i)
	{
		if (p[i].y <= 0 || p[i].t <= 0)
		{
			angle = rand()%365 * 0.0174;
			
			if (radius_x != 0) radius_buffer = exclude_radius + rand()%radius_x;
			p[i].x = center_x + cos(angle) * radius_buffer;
			if (radius_y != 0) radius_buffer = exclude_radius + rand()%radius_y;
			p[i].y = center_y + sin(angle) * radius_buffer;

			p[i].t = 20;
		}
		else continue;
	}
}

void draw_particles(SDL_Renderer* renderer, Particle* p)
{
	register unsigned char green;
	for (register int i = 0 ; i < PART_QUANT ; ++i)
	{
		if (p[i].t > 0)
		{
			green =  pow(p[i].t, 2);
			SDL_SetRenderDrawColor(renderer, 255, green, 0, 0);
			SDL_RenderDrawPoint(renderer, p[i].x, p[i].y);
			//SDL_RenderDrawLine(renderer, p[i].x, p[i].y, p[i + 1].x, p[i + 1].y);
		}
	}
}

void init_particles(Particle* p)
{
	for (int i = 0 ; i < PART_QUANT ; ++i)
	{
		p[i].x = -1;
		p[i].y = -1;
	}
}

void move_particles(Particle* p, SDL_Event event, double delta)
{
	register double speed;
	for (register int i = 0 ; i < PART_QUANT ; ++i)
	{
		if (p[i].t > 0)
		{	
					
			speed = delta * p[i].t * 2;
			p[i].y -= speed;
			printf("%d %d\n",event.motion.x,event.motion.y);
			if (sqrt(pow(p[i].x - event.motion.x,2) + pow(p[i].y - event.motion.y,2) <= 900))
			{
				if (p[i].x > event.motion.x)
				{
					p[i].x = sqrt(900 - pow(p[i].y - event.motion.y,2)) + event.motion.x;
				}
				else
				{
					p[i].x = -1 * sqrt(900 - pow(p[i].y - event.motion.y,2)) + event.motion.x;

				}
			}
			else
			{
				p[i].x = p[i].x + (-3 + rand()%5);
			}
		}
	}
}
void* check_environment(void* thread_data)
{
	particle_storage* data = (particle_storage*) thread_data;
	for (register int i = data->start ; i < data->end ; ++i)
	{
		data->p[i].e = 0;
		for (register int j = 0 ; j < PART_QUANT ; ++j)
		{
			if (i == j) continue;
			if (sqrt(pow(data->p[i].x - data->p[j].x, 2) + pow(data->p[i].y - data->p[j].y, 2) <= 7))
			{
				++data->p[i].e;
			}
		}
		if (data->p[i].e == 0) data->p[i].t -= 4;
		else if (data->p[i].e >= 5) ++data->p[i].t;
	}
	pthread_exit(0);
}

int main()
{
	SDL_Window* window = SDL_CreateWindow("Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 415, 500, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Event event;
	
	srand(time(NULL));

	Particle particles[PART_QUANT];
	init_particles(particles);

	pthread_t th[THREADS];

	particle_storage pd[THREADS];
	for (int i = 0 ; i < THREADS ; ++i)
	{
		pd[i].p = particles;
		pd[i].start = i * PART_QUANT / THREADS;
		pd[i].end = (i + 1) * PART_QUANT / THREADS;
	}

	double time_start = clock(), time_end = clock() + 10;
	double delta;

	while(1)
	{
		SDL_SetRenderDrawColor(renderer,0,0,0,0);
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);

		if(SDL_PollEvent(&event))
		{
			if(event.type == SDL_KEYDOWN) break;
		}
	}

	while(1)
	{
		delta = (time_end - time_start) / CLOCKS_PER_SEC;
		time_start = clock();
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);	
		for (int i = 0 ; i < THREADS ; ++i)
		{
			pthread_create(&th[i], NULL, check_environment, (void*) &pd[i]);
		}

		for (int i = 0 ; i < THREADS ; ++i)
		{
			pthread_join(th[i], NULL);
		}

		spawn_particles(particles, 207, 500, 100, 10, 0);
		move_particles(particles, event, delta);
		draw_particles(renderer,particles);
		
		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN) return 0;
			if (event.type == SDL_QUIT) return 0;
		}
		SDL_RenderPresent(renderer);
		time_end = clock();
	}
	return 0;
}
