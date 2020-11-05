#include "PuzzleMode.hpp"

#include "Collisions.hpp"

PuzzleMode::PuzzleMode() {
	// TODO : Read level data and create platforms

	// #HACK : Creating a set of test platforms
	for (size_t i = 0; i < 5; i++)
	{
		PlatformTile *platform = new PlatformTile(glm::vec2(i * 120.0f + 60.0f, 400.0f - i * 60.0f), glm::vec2(100.0f, 20.0f));
		std::cout << platform->position.x << ", " << platform->position.y << std::endl;
		platforms.emplace_back(platform);
	}

  // #HACK : spawn 2 default players
  add_player(glm::vec2(200, 499), SDLK_a, SDLK_d, SDLK_w);
  add_player(glm::vec2(600, 500), SDLK_LEFT, SDLK_RIGHT, SDLK_UP);
}

PuzzleMode::~PuzzleMode() {}

void PuzzleMode::add_player(glm::vec2 position,
    SDL_Keycode leftkey, SDL_Keycode rightkey, SDL_Keycode jumpkey) {
  Input* left = input_manager.register_key(leftkey);
  Input* right = input_manager.register_key(rightkey);
  Input* jump = input_manager.register_key(jumpkey);
  players.emplace_back(Player(position, left, right, jump));
}

bool PuzzleMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	return input_manager.handle_event(evt);
}

void PuzzleMode::update(float elapsed) {
  // Calculate inputs and movement for each player
  for (auto& player : players) {
    player.velocity.x = 0;
    player.velocity.y = 0;

    if(player.left->held()) {
      player.velocity.x -= Player::movespeed * elapsed;
    }

    if(player.right->held()) {
      player.velocity.x += Player::movespeed * elapsed;
    }

    // Process jumping
    if(player.jump->held() && !player.falling) {
       player.jump_input = true;
       if(player.input_jump_time < Player::max_jump_time) {
         player.input_jump_time += elapsed;
         if(player.input_jump_time >= Player::max_jump_time) {
           player.input_jump_time = Player::max_jump_time;
         }
       }
    }

    if(player.jump->just_released()) {
       player.jump_input = false;
       if(player.input_jump_time < Player::min_jump_time) {
         player.input_jump_time = Player::min_jump_time;
       }
    }

    if(player.cur_jump_time < player.input_jump_time) {
      player.cur_jump_time += elapsed;
      player.velocity.y += Player::jumpspeed * elapsed;

      if(player.cur_jump_time >= player.input_jump_time) {
        player.jump_clear = true;
      }
    }

    if(!player.jump->held() && player.jump_clear) {
        player.cur_jump_time = 0.0f;
        player.input_jump_time = 0.0f;
        player.jump_clear = false;
    }

    player.position += player.velocity;
  }

  // Player-player collision
  for (auto t1 = players.begin(); t1 != players.end(); t1++) {
    for (auto t2 = t1 + 1; t2 != players.end(); t2++) {
    Shapes::Rectangle rect1 = Shapes::Rectangle(t1->position, Player::size.x, Player::size.y, false);
      Shapes::Rectangle rect2 = Shapes::Rectangle(t2->position, Player::size.x, Player::size.y, false);
      t1->position += Collisions::rectangle_rectangle_collision(rect1, rect2, 2);
    }
  }

  // Player-platform collision
  for (auto& player : players) {
    // Check for collision with platforms
    Shapes::Rectangle player_rect = Shapes::Rectangle(player.position,
        Player::size.x, Player::size.y, false);

    for (auto& platform : platforms) {
      Shapes::Rectangle platform_rect = Shapes::Rectangle(platform->position,
        platform->size.x, platform->size.y, true);

      if(Collisions::rectangle_rectangle_collision(player_rect, platform_rect)) {
        player.position += Collisions::rectangle_rectangle_collision(player_rect,
            platform_rect, 2);
        break;
      }
    }
  }

  // Gravity
  for (auto& player : players) {
    glm::vec2 gravity = glm::vec2(0, -Player::gravityspeed * elapsed);

    player.falling = true;

    // Check for collision with platforms
    Shapes::Rectangle player_rect = Shapes::Rectangle(player.position + gravity,
        Player::size.x, Player::size.y, false);

    for (auto& platform : platforms) {
      Shapes::Rectangle platform_rect = Shapes::Rectangle(platform->position,
        platform->size.x, platform->size.y, true);

      if(Collisions::rectangle_rectangle_collision(player_rect, platform_rect)) {
        glm::vec2 movement = Collisions::rectangle_rectangle_collision(player_rect,
            platform_rect, 1);

        //FIXME bug in collisions with nonzero x value
        movement.x = 0;

        player.position += gravity + movement;
        player.falling = false;
        break;
      }
    }

    if(player.falling) {
      player.position += gravity;
    }
  }

  input_manager.tick();
}

void PuzzleMode::draw(glm::uvec2 const &drawable_size) {
	glClearColor(1.0f, 0.01f, 0.01f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	GL_ERRORS();

	for (auto &&platform : platforms)
	{
		platform->draw(drawable_size);
	}

  for (auto& player : players) {
    //player.draw(drawable_size);
  }
  std::cout << std::endl;
	GL_ERRORS();
}
