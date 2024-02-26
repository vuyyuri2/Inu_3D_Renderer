#include "gfx.h"

#include <stddef.h>
#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <vector>

#include "model_loading/image/stb_image.h"

#include "utils/general.h"
#include "utils/vectors.h"
#include "utils/log.h"

std::vector<material_t> materials;

// VBO
vbo_t create_vbo(const float* vertices, const int data_size) {
	vbo_t vbo;
	glGenBuffers(1, &vbo.id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo.id);
	glBufferData(GL_ARRAY_BUFFER, data_size, vertices, GL_STATIC_DRAW);
	return vbo;
}

vbo_t create_dyn_vbo(const int data_size) {
	vbo_t vbo;
	glGenBuffers(1, &vbo.id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo.id);
	glBufferData(GL_ARRAY_BUFFER, data_size, NULL, GL_DYNAMIC_DRAW);
	return vbo;
}

void update_vbo_data(const vbo_t& vbo, const float* vertices, const int data_size) {
	glBindBuffer(GL_ARRAY_BUFFER, vbo.id);
	glBufferSubData(GL_ARRAY_BUFFER, 0, data_size, vertices);
}

void bind_vbo(const vbo_t& vbo) {
	glBindBuffer(GL_ARRAY_BUFFER, vbo.id);
}

void unbind_vbo() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void delete_vbo(const vbo_t& vbo) {
	glDeleteBuffers(1, &vbo.id);
}

// EBO
ebo_t create_ebo(const unsigned int* indicies, const int size_of_buffer) {
	ebo_t ebo;
	ebo.num_indicies = size_of_buffer / sizeof(indicies[0]);
	glGenBuffers(1, &ebo.id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_of_buffer, indicies, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return ebo;
}

void draw_ebo(const ebo_t& ebo) {
	glDrawElements(GL_TRIANGLES, ebo.num_indicies, GL_UNSIGNED_INT, 0);
}

void bind_ebo(const ebo_t& ebo) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.id);
}

void unbind_ebo() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void delete_ebo(const ebo_t& ebo) {
	glDeleteBuffers(1, &ebo.id);
}

// VAO
vao_t create_vao() {
	vao_t vao;
	glGenVertexArrays(1, &vao.id);
	return vao;
}

void bind_vao(const vao_t& vao) {
	glBindVertexArray(vao.id);
}

void unbind_vao() {
	glBindVertexArray(0);
}

void vao_enable_attribute(vao_t& vao, const vbo_t& vbo, const int attr_id, const int num_values, const int d_type, const int stride, const int offset) {
	bind_vao(vao);
	bind_vbo(vbo);
	glVertexAttribPointer(attr_id, num_values, d_type, GL_FALSE, stride, reinterpret_cast<void*>(offset));
	glEnableVertexAttribArray(attr_id);
	unbind_vbo();
	unbind_vao();
}

void vao_bind_ebo(vao_t& vao, ebo_t& ebo) {
	bind_vao(vao);
	bind_ebo(ebo);
	unbind_vao();
	unbind_ebo();
}

void delete_vao(const vao_t& vao) {
	glDeleteVertexArrays(1, &vao.id);
}

// Shaders
shader_t create_shader(const char* vert_source_path, const char* frag_source_path) {
	shader_t shader;
	shader.id = glCreateProgram();

	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	char* vert_source = get_file_contents(vert_source_path);
	glShaderSource(vert, 1, &vert_source, NULL);

	int success;
	glCompileShader(vert);
	glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char info_log[512]{};
		glGetShaderInfoLog(vert, 512, NULL, info_log);
		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED %s\n", info_log);
		throw std::runtime_error("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" + std::string(info_log));
	}

	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
	char* frag_source = get_file_contents(frag_source_path);
	glShaderSource(frag, 1, &frag_source, NULL);
	glCompileShader(frag);
	glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char info_log[512]{};
		glGetShaderInfoLog(frag, 512, NULL, info_log);
		printf("ERROR::SHADER::FRAG::COMPILATION_FAILED %s\n", info_log);
		throw std::runtime_error("ERROR::SHADER::FRAG::COMPILATION_FAILED\n" + std::string(info_log));
	}

	glAttachShader(shader.id, vert);
	glAttachShader(shader.id, frag);
	glLinkProgram(shader.id);

	glDeleteShader(vert);
	glDeleteShader(frag);

	free(vert_source);
	free(frag_source);

	return shader;
}

void bind_shader(shader_t& shader) {
	glUseProgram(shader.id);
}

void unbind_shader() {
	glUseProgram(0);
}

void shader_set_int(shader_t& shader, const char* var_name, int val) {
	glUseProgram(shader.id);
	GLint loc = glGetUniformLocation(shader.id, var_name);
    if (loc == -1) {
        printf("%s does not exist in shader %i\n", var_name, shader.id);
    }
	glUniform1i(loc, val);
	unbind_shader();
}

void shader_set_float(shader_t& shader, const char* var_name, float val) {
	glUseProgram(shader.id);
	GLint loc = glGetUniformLocation(shader.id, var_name);
    if (loc == -1) {
        printf("%s does not exist in shader %i\n", var_name, shader.id);
    }
	glUniform1f(loc, val);
	unbind_shader();
}

void shader_set_vec3(shader_t& shader, const char* var_name, vec3 vec) {
	glUseProgram(shader.id);
	GLint loc = glGetUniformLocation(shader.id, var_name);
    if (loc == -1) {
        printf("%s does not exist in shader %i\n", var_name, shader.id);
    }
	glUniform3fv(loc, 1, (GLfloat*)&vec);
	unbind_shader();
}

// TEXTURES
static std::vector<texture_t> textures;
int create_texture(const char* img_path) {
	for (texture_t& t : textures) {
		if (strcmp(img_path, t.path.c_str()) == 0) {
			return t.id;
		}
	}

	texture_t texture;
	texture.tex_slot = 0;	
	texture.path = std::string(img_path);

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(img_path, &texture.width, &texture.height, &texture.num_channels, 0);
	inu_assert(data, "image data not loaded");

	texture.id = textures.size();

	glGenTextures(1, &texture.gl_id);
	glBindTexture(GL_TEXTURE_2D, texture.gl_id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	if (texture.num_channels == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);
	textures.push_back(texture);
	return texture.id;
}

void bind_texture(int tex_id) {
	texture_t& tex = textures[tex_id];
	glActiveTexture(GL_TEXTURE0 + tex.tex_slot);
	glBindTexture(GL_TEXTURE_2D, tex.gl_id);
}

void unbind_texture() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

// MATERIALS
shader_t material_t::associated_shader;

int create_material(vec4 color, int tex0_sampler_handle) {
	material_t mat;
	mat.color = color;
	mat.tex0_sampler_handle = tex0_sampler_handle;
	mat.angle = 0;
	materials.push_back(mat);
	return materials.size()-1;
}

void bind_material(int mat_idx) {
	inu_assert(mat_idx < materials.size(), "mat idx out of bounds");
	shader_t& shader = material_t::associated_shader;

	material_t& mat = materials[mat_idx];
	shader_set_float(shader, "angle", mat.angle);
  mat.angle += 0.01f;
  if (mat.angle >= 360.f) {
    mat.angle -= 360.f;
  }
  
  vec3 color;
  if (mat.angle > 180) {
    color.x = 0;
    color.y = 1;
    color.z = 0;
  } else {
    color.x = 0;
    color.y = 0;
    color.z = 1;
  }
  color.x = mat.color.x;
  color.y = mat.color.y;
  color.z = mat.color.z;
  shader_set_vec3(shader, "in_color", color);

	bind_texture(mat.tex0_sampler_handle);
	shader_set_int(shader, "tex0", 0);

  bind_shader(shader);
}
