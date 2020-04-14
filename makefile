fire: fire.src/main.c
	gcc fire.src/main.c -o fire -lm -lSDL2 -lpthread
water: water.src/main.c
	gcc water.src/main.c -o water -lm -lSDL2 -lpthread
