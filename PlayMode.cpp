#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint game3_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > game3_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("game3.pnct"));
	game3_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > game3_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("game3.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = game3_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = game3_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

// Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
// 	return new Sound::Sample(data_path("dusty-floor.opus"));
// });

Load<Sound::Sample> yeet_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("yeet.wav"));
}); Load<Sound::Sample> back_a_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("back_a.wav"));
}); Load<Sound::Sample> low_e_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("low_e.wav"));
}); Load<Sound::Sample> i_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("i.wav"));
}); Load<Sound::Sample> u_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("u.wav"));
}); Load<Sound::Sample> high_o_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("high_o.wav"));
}); Load<Sound::Sample> schwa_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("schwa.wav"));
}); Load<Sound::Sample> correct_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("correct.wav"));
}); Load<Sound::Sample> incorrect_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("incorrect.wav"));
});

PlayMode::PlayMode() : scene(*game3_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "wug4") {
			wugs[0].xform = &transform;
			wugs[0].favorite_vowel_idx = 4;
		}
		else if (transform.name == "wug2") {
			wugs[1].xform = &transform;
			wugs[1].favorite_vowel_idx = 2;
		}
		else if (transform.name == "wug0") {
			wugs[2].xform = &transform;
			wugs[2].favorite_vowel_idx = 1;
		}
		else if (transform.name == "wug1") {
			wugs[3].xform = &transform;
			wugs[3].favorite_vowel_idx = 5;
		}
		else if (transform.name == "wug3") {
			wugs[4].xform = &transform;
			wugs[4].favorite_vowel_idx = 3;
		}

		else if (transform.name == "schwa") {
			vowels[0].sound = schwa_sample;
			vowels[0].xform = &transform;
		} else if (transform.name == "back_a") {
			vowels[1].sound = back_a_sample;
			vowels[1].xform = &transform;
		} else if (transform.name == "low_e") {
			vowels[2].sound = low_e_sample;
			vowels[2].xform = &transform;
		} else if (transform.name == "i") {
			vowels[3].sound = i_sample;
			vowels[3].xform = &transform;
		} else if (transform.name == "u") {
			vowels[4].sound = u_sample;
			vowels[4].xform = &transform;
		} else if (transform.name == "high_o") {
			vowels[5].sound = high_o_sample;
			vowels[5].xform = &transform;
		}
	};
	for (uint8_t i = 0; i < num_wugs; i++) {
		if (wugs[i].xform == nullptr) {
			printf("Wug number %d\n", i);
			throw std::runtime_error("Wug not found.");
		}
	}; for (uint8_t i = 0; i < num_vowels; i++) {
		if (vowels[i].xform == nullptr) {
			printf("Vowel number %d\n", i);
			throw std::runtime_error("Vowel not found.");
		}
	}; 
	// wug and vowel 0 start out as being selected
	wugs[0].is_selected = 1;
	vowels[0].is_selected = 1;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	// selected wug is rotated
	wugs[0].xform->rotation = glm::angleAxis(atan2(-camera->transform->position.y,-camera->transform->position.x), glm::vec3(0., 0., 1.));

	// initialize vowel positions/rotations
	for (uint8_t i = 0; i < num_vowels; i++) {
		vowels[i].xform->position = -camera->transform->make_local_to_parent()[2] * 0.2f + camera->transform->position;
	}

	//start music loop playing:
	// (note: position will be over-ridden in update())
	// leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, wugs[0].xform->position , 10.0f);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			Wug prev_wug = wugs[selected_wug_idx];
			prev_wug.is_selected = false;
			prev_wug.xform->rotation = glm::angleAxis(0.f, glm::vec3(0., 0., 1.));
			selected_wug_idx = (selected_wug_idx + 1) % num_wugs;
			Wug new_wug = wugs[selected_wug_idx];
			new_wug.is_selected = true;
			new_wug.xform->rotation = glm::angleAxis(atan2(-camera->transform->position.y,-camera->transform->position.x), glm::vec3(0., 0., 1.));
			Sound::play(*(vowels[new_wug.favorite_vowel_idx].sound), 1.0f);
		} else if (evt.key.keysym.sym == SDLK_v) {
			vowels[selected_vowel_idx].is_selected = false;
			selected_vowel_idx = (selected_vowel_idx + 1) % num_vowels;
			vowels[selected_vowel_idx].is_selected = true;
			Sound::play(*(vowels[selected_vowel_idx].sound), 1.0f);
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			// set vowel to be thrown
			if (!vowels[selected_vowel_idx].is_thrown) {
				vowels[selected_vowel_idx].is_thrown = true;
				Sound::play(*yeet_sample, 1.0f);
			}
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	// yeet vowel at selected wug
	Vowel curr_vowel = vowels[selected_vowel_idx];
	if (curr_vowel.is_thrown) {
		Wug curr_wug = wugs[selected_wug_idx];

		float vowel_speed = 1.f;
		glm::vec3 vowel_pos = curr_vowel.xform->position;
		glm::vec3 vowel_direction = glm::normalize(curr_wug.xform->position - vowel_pos);
		curr_vowel.xform->position += vowel_direction * vowel_speed * elapsed;

		// wug collision check
		if (std::abs(glm::distance(vowel_pos, curr_wug.xform->position)) < 0.01f) {
			// check correctness
			if (curr_wug.favorite_vowel_idx == selected_vowel_idx) {
				Sound::play(*correct_sample);
			} else Sound::play(*incorrect_sample);
			// move vowel back
			curr_vowel.xform->position = -camera->transform->make_local_to_parent()[2] * 0.2f + camera->transform->position;
			vowels[selected_vowel_idx].is_thrown = false;
		}
	}

	//move sound to follow leg tip position:
	// leg_tip_loop->set_position(wugs[0].xform->position, 1.0f / 60.0f);

	//move camera:
	{
		//combine inputs into a move:
		constexpr float PlayerSpeed = 1.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 frame_forward = -frame[2];

		camera->transform->position += move.x * frame_right + move.y * frame_forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	// make wug face camera
	wugs[selected_wug_idx].xform->rotation = glm::angleAxis(
		atan2(-camera->transform->position.y,-camera->transform->position.x), 
		glm::vec3(0., 0., 1.));

	for (uint8_t i = 0; i < num_vowels; i++) {
		if (!vowels[i].is_thrown) vowels[i].xform->position = -camera->transform->make_local_to_parent()[2] * 0.2f + camera->transform->position;
		if (!vowels[i].is_selected) vowels[i].xform->position += 0.4f * camera->transform->make_local_to_parent()[2];
	}

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; arrow keys move; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; arrow keys move; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}
