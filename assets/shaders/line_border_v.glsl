#version 430 core
layout (location = 0) in vec2 vertex_position;
layout (location = 1) in vec2 normal_direction;
layout (location = 2) in vec2 direction;

out vec2 tex_coord;
layout (location = 0) uniform vec2 offset;
layout (location = 1) uniform float aspect_ratio;
layout (location = 2) uniform float zoom;
layout (location = 3) uniform vec2 map_size;
layout (location = 4) uniform float border_width;

void main() {
	float zoom_level = 1.5f * clamp(zoom, 2.f, 10.f) - 0.5f;
	float thickness = border_width / zoom_level;
	vec2 normal_vector = normalize(normal_direction) * thickness;
	vec2 extend_vector = -normalize(direction) * thickness / (1 + sqrt(2));
	vec2 world_pos = vertex_position + vec2(-offset.x, offset.y);

	world_pos.x = mod(world_pos.x, 1.0f);

	world_pos.x *= map_size.x / map_size.y;
	world_pos += extend_vector + normal_vector;
	world_pos.x /= map_size.x / map_size.y;
	gl_Position = vec4(
		(2. * world_pos.x - 1.f) * zoom / aspect_ratio * map_size.x / map_size.y,
		(2. * world_pos.y - 1.f) * zoom,
		0.0, 1.0);
	tex_coord = vertex_position;
}
