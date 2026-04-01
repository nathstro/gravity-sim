#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <miniaudio/miniaudio.h>
#include <random>
#include <string>
#include "Renderer.hpp"

int main()
{
	Renderer renderer;
	if (renderer.init())
	{
		return -1;
	}

	if (renderer.initUI())
	{
		return -1;
	}

	ma_engine engine;
	if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
	    printf("ERROR: Failed to init audio engine\n");
	    return -1;
	}

	ma_sound music;

	GLFWwindow* window = renderer.getWindow();

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> dist(0, 3);
	std::uniform_int_distribution<int> play(0, 600);
	int lastSong = -1;

	while (!glfwWindowShouldClose(window))
	{
		renderer.processRendering();

		if (!ma_sound_is_playing(&music))
		{
			if (play(rng) == 600)
			{
				int index = lastSong;

				while (index == lastSong) 
		    		index = dist(rng);

		    	std::string m = "audio/music" + std::to_string(index) + ".mp3";
		    	ma_sound_init_from_file(&engine, m.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &music);
		    	ma_sound_start(&music);
		    	lastSong = index;
			}
		}
	}

	ma_sound_uninit(&music);
	renderer.drop();
	return 0;
}