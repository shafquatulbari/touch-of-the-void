#pragma once

#include <array>
#include <utility>
#include <map>

#include "common/common.hpp"
#include "components/components.hpp"
#include "ecs/ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;
	// number of sprites per row and column in the sprite sheet
	std::array<ivec2, sheet_count> sheet_sprite_count = {
		ivec2(12,1),
		ivec2(6,1)
	};
	std::array<ivec2, sheet_count> sheet_dimensions;

	// IMPORTANT: Make sure these paths remain in sync with the associated enumerators on components.hpp
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		// TODO: specify meshes of other assets here like so:
		// ex std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj"))
		
		// player.obj contains points to a convex hull
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER_CH, mesh_path("player_ch.obj")),
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::BULLET_CH, mesh_path("bullet_ch.obj")),
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::ENEMY_SPITTER_CH, mesh_path("enemy_spitter_ch.obj"))
	};

	// IMPORTANT: Make sure these paths remain in sync with the associated enumerators on components.hpp
	const std::array<std::string, texture_count> texture_paths = {
		// TODO: specify textures of other assets here like so:
		textures_path("bullet_8x8.png"),
		textures_path("enemy_spitter_32x32.png"),
		textures_path("lvl1_background_480x480.png"),
		textures_path("lvl1_doors_96x28.png"),

		textures_path("lvl1_full_wall_closed_door_416x32.png"),
		textures_path("right_lvl1_full_wall_closed_door_416x32.png"),
		textures_path("bottom_lvl1_full_wall_closed_door_416x32.png"),
		textures_path("left_lvl1_full_wall_closed_door_416x32.png"),
		
		textures_path("lvl1_full_wall_no_door_416x32.png"),
		textures_path("lvl1_full_wall_open_door_416x32.png"),
		textures_path("lvl1_obstacle_32x32.png"),
		textures_path("lvl1_wall_bottom_corner_32x32.png"),
		textures_path("lvl1_wall_end_32x32.png"),
		textures_path("lvl1_wall_top_corner_32x32.png"),
		textures_path("lvl1_wall_64x32.png"),
		textures_path("player_32x32.png")
	};

	std::array<GLuint, effect_count> effects;
	// IMPORTANT: Make sure these paths remain in sync with the associated enumerators on components.hpp
	const std::array<std::string, effect_count> effect_paths = {
		// TODO: specify shader scripts here like so:
		shader_path("coloured"),
		shader_path("textured"),
		shader_path("post_process"),
	};

	std::array<GLuint, font_count> fonts;
	// IMPORTANT: Make sure these paths remain in sync with the associated enumerators on components.hpp
	const std::array<std::string, font_count> font_paths = {
		fonts_path("Vermin_Vibes_1989.ttf")
	};

	std::array<GLuint, sheet_count> sheets;
	// IMPORTANT: Make sure these paths remain in sync with the associated enumerators on components.hpp
	const std::array<std::string, sheet_count> sheet_paths = {
		//sheets_path("blue_effect_bullet_impact_explosion_32x32.png"),
		sheets_path("explosion_96x96.png"),
		sheets_path("exploding_skull_32x32.png")
		//sheets_path("green_effect_bullet_impact_explosion_32x32.png"),
		//sheets_path("purple_effect_bullet_impact_explosion_32x32.png"),
		//sheets_path("red_effect_bullet_impact_explosion_32x32.png"),
		//sheets_path("yellow_effect_bullet_impact_explosion_32x32.png")
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlSheets();

	void initializeGlEffects();

	void initializeGlMeshes();

	bool initializeFonts();


	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the wind
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	// Draw all text entities
	void drawText(const mat3& projection);

	mat3 createProjectionMatrix();

	bool loadEffectFromFile(
		const std::string& vs_path, const std::string& fs_path, GLuint& out_program);

	bool loadFontFromFile(
		const std::string& font_path, unsigned int font_default_size);

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen(const mat3& projection);

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;

	GLuint vao;
	GLuint vbo;

	// Fonts
	std::map<char, Character> m_ftCharacters;
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;

	// Sprite Sheets

	std::array<std::map<std::pair<int, int>, Sprite>, sheet_count> m_ftSpriteSheets;
};