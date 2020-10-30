#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <thread>
#include <cmath>
#include "dbhelper.h"
#include <vector>

class Particle
{
private:
public:
	Particle()
	{
		x = -1;
		y = -1;
		y = 0;
	}
	double x;
	double y;
	double t;
};
class Manager
{
private:
	unsigned int threads_count;
	std::vector<Particle*> particles;
public:
	Manager(unsigned count)
	{
		for (unsigned i = 0 ; i < count ; i++)
		{
			Particle* buf = new Particle;
			particles.emplace_back(buf);
		}
		threads_count = std::thread::hardware_concurrency();
	}
	~Manager()
	{
		for (unsigned i = 0 ; i < particles.size() ; i++)
		{
			delete particles[i];
		}
	}
	void spawn_part(int begin, int end, const int& center_x, const int& center_y, const int& radius_x, const int& radius_y, const int& exclude_radius)
	{
		int radius_buffer = 0;
		double angle;
		for (int i = begin ; i < end ; i++)
		{
			if (particles[i]->y <= 0 || particles[i]->t <= 0)
			{
				angle = rand()%365 * 0.0174;
				
				if (radius_x != 0) radius_buffer = exclude_radius + rand()%radius_x;
				particles[i]->x = center_x + cos(angle) * radius_buffer;
				if (radius_y != 0) radius_buffer = exclude_radius + rand()%radius_y;
				particles[i]->y = center_y + sin(angle) * radius_buffer;

				particles[i]->t = 20;
			}
			else continue;
		}
	}
	void spawn(const int& center_x, const int& center_y, const int& radius_x, const int& radius_y, const int& exclude_radius)
	{
		std::thread threads[threads_count];
		for (unsigned i = 0 ; i < threads_count ; i++)	
		{
			threads[i] = std::thread(&Manager::spawn_part, this, particles.size() / threads_count * i, particles.size() / threads_count * (i + 1), center_x, center_y, radius_x, radius_y, exclude_radius);
		}
		for (unsigned i = 0 ; i < threads_count ; i++)
		{
			threads[i].join();
		}
	}
	void draw(SDL_Renderer* renderer)
	{
		Uint8 green;
		for (auto& particle : particles)
		{
			if (particle->t > 0)
			{
				green =  pow(particle->t, 2);
				if (green > 255) green = 255;
				SDL_SetRenderDrawColor(renderer, 255, green, 0, 0);
				SDL_RenderDrawPoint(renderer, particle->x, particle->y);
			}
		}
	}
	void move(const SDL_Event& event, int begin, int end)
	{
		double speed;
		for (int i = begin ; i < end ; i++)
		{
			if (particles[i]->t > 0)
			{	
				speed = DBHelper::delta * (30 + particles[i]->t) * 9;
				particles[i]->y -= speed;
				if (sqrt(pow(particles[i]->x - event.motion.x,2) + pow(particles[i]->y - event.motion.y,2) <= 900))
				{
					if (particles[i]->x > event.motion.x)
					{
						particles[i]->x = sqrt(900 - pow(particles[i]->y - event.motion.y,2)) + event.motion.x;
					}
					else
					{
						particles[i]->x = -1 * sqrt(900 - pow(particles[i]->y - event.motion.y,2)) + event.motion.x;

					}
				}
				else
				{
					particles[i]->x = particles[i]->x + (-300 + rand()%500) * DBHelper::delta;
				}
			}
		}
	}
	void update(const SDL_Event& event)
	{
		std::thread threads[threads_count];
		for (unsigned i = 0 ; i < threads_count ; i++)
		{
			threads[i] = std::thread(&Manager::move, this, event, i * particles.size() / threads_count, particles.size() / threads_count * (i + 1));
		}
		for (unsigned i = 0 ; i < threads_count ; i++)
		{
			threads[i].join();
		}
	}

	void check(int begin, int end)
	{
		for (int i = begin ; i < end ; i++)
		{
			int env = 0;
			for (auto& particle : particles)
			{
				if (particles[i] == particle) continue;
				if (sqrt(pow(particles[i]->x - particle->x, 2) + pow(particles[i]->y - particle->y, 2) <= 40))
				{
					env++;
				}
			}
			if (env == 0) particles[i]->t -= 4;
			else if (env >= 10) particles[i]->t++;
		}
	}
	void check_environment()
	{
		std::thread threads[threads_count];
		for (unsigned i = 0 ; i < threads_count ; i++)
		{
			threads[i] = std::thread(&Manager::check, this, i * particles.size() / threads_count, particles.size() / threads_count * (i + 1));
		}
		for (unsigned i = 0 ; i < threads_count ; i++)
		{
			threads[i].join();
		}

	}
};

void event_handler(SDL_Event& event, bool& isWork)
{
	while (1)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
			{
				isWork = false;
				return;
			}
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT) SDL_ShowCursor(0);
				else SDL_ShowCursor(1);
			}
			if (event.type == SDL_QUIT)
			{	   
				isWork = false;
				return;
			}
		}
	}
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		return 1;
	}
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Init(SDL_INIT_AUDIO);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Music* mus = Mix_LoadMUS("../sound/fire.mp3");
	if (mus == 0) return 1;

	SDL_Window* window = SDL_CreateWindow("Fire", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, SDL_WINDOW_FULLSCREEN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Event event;
	
	srand(time(NULL));

	Manager manager(std::stoi(argv[1]));

	bool isWork = true;
	std::thread handler(event_handler, std::ref(event), std::ref(isWork));
	while(isWork)
	{
		DBHelper::begin();
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		if (Mix_PlayingMusic() == 0)
		{
			Mix_PlayMusic(mus, 1);
		}
		manager.spawn(1920 / 2, 1080, 500, 50, 0);
		manager.check_environment();
		manager.update(event);
		manager.draw(renderer);
		SDL_RenderPresent(renderer);
		DBHelper::end();
	}
	handler.join();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
