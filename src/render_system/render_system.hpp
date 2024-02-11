#pragma once

#include <array>
#include <utility>

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
	std::array<ivec2, texture_count> texture_dimensions; // TODO: figure out how to use this, doesn't seem to be working

	// IMPORTANT: Make sure these paths remain in sync with the associated enumerators on components.hpp
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		// TODO: specify meshes of other assets here like so:
		// ex std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj"))
	};

	// IMPORTANT: Make sure these paths remain in sync with the associated enumerators on components.hpp
	const std::array<std::string, texture_count> texture_paths = {
		// TODO: specify textures of other assets here like so:
		textures_path("player.png"),
		textures_path("obstacle.png"),
		textures_path("lvl1_background.png"),
		textures_path("lvl1_wall.png"),
		textures_path("lvl1_wall_bottom_corner.png"),
		textures_path("lvl1_wall_end.png"),
		textures_path("lvl1_wall_top_corner.png")
	};

	std::array<GLuint, effect_count> effects;
	// IMPORTANT: Make sure these paths remain in sync with the associated enumerators on components.hpp
	const std::array<std::string, effect_count> effect_paths = {
		// TODO: specify shader scripts here like so:
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("textured"),
		shader_path("post_process")
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

	void initializeGlEffects();

	void initializeGlMeshes();
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

	mat3 createProjectionMatrix();

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
