#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	Scene::Transform *wug_xform = nullptr;

	static const uint8_t num_wugs = 5;
	static const uint8_t num_vowels = 6;

	struct Vowel {
		Scene::Transform *xform;
		Sound::Sample const *sound;
		bool is_selected = false;
		bool is_thrown = false;
	};
	
	struct Wug {
		Scene::Transform *xform;
		bool is_selected = false;
		uint8_t times_fed = 0;
		uint8_t favorite_vowel_idx;
	};

	std::array<Wug, num_wugs> wugs;
	std::array<Vowel, num_vowels> vowels;

	uint8_t selected_wug_idx = 0;
	uint8_t selected_vowel_idx = 0;

	//music coming from the tip of the leg (as a demonstration):
	// std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	// std::shared_ptr< Sound::Sample > yeet_sound;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
