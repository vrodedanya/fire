#include <iostream>
#include <thread>
#include <SDL2/SDL.h>
#include <cmath>
#include "DeltaTime.h"

#define PART_QUANT 4800
#define THREADS 12

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

void spawn_particles(particle_storage* data, const int& center_x, const int& center_y, const int& radius_x, const int& radius_y, const int& exclude_radius)
{
	register int radius_buffer = 0;
	register double angle;
	for (register int i = data->start ; i < data->end ; ++i)
	{
		if (data->p[i].y <= 0 || data->p[i].t <= 0)
		{
			angle = rand()%365 * 0.0174;
			
			if (radius_x != 0) radius_buffer = exclude_radius + rand()%radius_x;
			data->p[i].x = center_x + cos(angle) * radius_buffer;
			if (radius_y != 0) radius_buffer = exclude_radius + rand()%radius_y;
			data->p[i].y = center_y + sin(angle) * radius_buffer;

			data->p[i].t = 20;
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
			if (green > 255) green = 255;
			SDL_SetRenderDrawColor(renderer, 255, green, 0, 0);
			SDL_RenderDrawPoint(renderer, p[i].x, p[i].y);
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

void move_particles(particle_storage* data, const SDL_Event& event)
{
	register double speed;
	for (register int i = data->start ; i < data->end ; ++i)
	{
		if (data->p[i].t > 0)
		{	
			speed = DeltaTime::delta * (30 + data->p[i].t) * 9;
			data->p[i].y -= speed;
			if (sqrt(pow(data->p[i].x - event.motion.x,2) + pow(data->p[i].y - event.motion.y,2) <= 900))
			{
				if (data->p[i].x > event.motion.x)
				{
					data->p[i].x = sqrt(900 - pow(data->p[i].y - event.motion.y,2)) + event.motion.x;
				}
				else
				{
					data->p[i].x = -1 * sqrt(900 - pow(data->p[i].y - event.motion.y,2)) + event.motion.x;

				}
			}
			else
			{
				data->p[i].x = data->p[i].x + (-300 + rand()%500) * DeltaTime::delta;
			}
		}
	}
}
void check_environment(particle_storage* data)
{
	for (register int i = data->start ; i < data->end ; ++i)
	{
		int env = 0;
		for (register int j = 0 ; j < PART_QUANT ; ++j)
		{
			if (i == j) continue;
			if (sqrt(pow(data->p[i].x - data->p[j].x, 2) + pow(data->p[i].y - data->p[j].y, 2) <= 40))
			{
				env++;
			}
		}
		if (env == 0) data->p[i].t -= 4;
		else if (env >= 10) ++data->p[i].t;
	}
}

int main()
{
	SDL_Window* window = SDL_CreateWindow("Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, SDL_WINDOW_FULLSCREEN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Event event;
	
	srand(time(NULL));

	Particle particles[PART_QUANT];
	init_particles(particles);

	std::thread threads[THREADS];

	particle_storage pd[THREADS];
	for (int i = 0 ; i < THREADS ; ++i)
	{
		pd[i].p = particles;
		pd[i].start = i * PART_QUANT / THREADS;
		pd[i].end = (i + 1) * PART_QUANT / THREADS;
	}

	DeltaTime dt;

	bool isWork = true;
	while(isWork)
	{
		dt.begin();

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);	

		for (int i = 0 ; i < THREADS ; i++)
		{
			threads[i] = std::thread(check_environment, &pd[i]);
		}
		for (int i = 0 ; i < THREADS ; i++)
		{
			threads[i].join();
			threads[i] = std::thread(spawn_particles, &pd[i], 1920 / 2, 1080, 600, 30, 0);
		}
		for (int i = 0 ; i < THREADS ; i++)
		{
			threads[i].join();
			threads[i] = std::thread(move_particles, &pd[i], event);
		}
		for (int i = 0 ; i < THREADS ; i++)
		{
			threads[i].join();
		}

		draw_particles(renderer, particles);
		
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) isWork = false;
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT) SDL_ShowCursor(0);
				else SDL_ShowCursor(1);
			}
			if (event.type == SDL_QUIT) isWork = false;
		}
		SDL_RenderPresent(renderer);
		dt.end();
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
