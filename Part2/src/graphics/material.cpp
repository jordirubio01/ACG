#include "material.h"

#include "application.h"

#include <istream>
#include <fstream>
#include <algorithm>

FlatMaterial::FlatMaterial(glm::vec4 color)
{
	this->color = color;
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/flat.fs");
}

FlatMaterial::~FlatMaterial() { }

void FlatMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	// Upload node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	this->shader->setUniform("u_color", this->color);
}

void FlatMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (mesh && this->shader) {
		// Enable shader
		this->shader->enable();

		// Upload uniforms
		setUniforms(camera, model);

		// Do the draw call
		mesh->render(GL_TRIANGLES);

		this->shader->disable();
	}
}

void FlatMaterial::renderInMenu()
{
	ImGui::Text("Material Type: %s", std::string("Flat").c_str());

	ImGui::ColorEdit3("Color", (float*)&this->color);
}

WireframeMaterial::WireframeMaterial()
{
	this->color = glm::vec4(1.f);
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial() { }

void WireframeMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (this->shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);

		// Enable shader
		this->shader->enable();

		// Upload material specific uniforms
		setUniforms(camera, model);

		// Do the draw call
		mesh->render(GL_TRIANGLES);

		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

StandardMaterial::StandardMaterial(glm::vec4 color)
{
	this->color = color;
	this->base_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/basic.fs");
	this->normal_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/normal.fs");
	this->shader = this->base_shader;
}

StandardMaterial::~StandardMaterial() { }

void StandardMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	// Upload node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	this->shader->setUniform("u_color", this->color);

	if (this->texture) {
		this->shader->setUniform("u_texture", this->texture, 0);
	}
}

void StandardMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	bool first_pass = true;
	if (mesh && this->shader)
	{
		// Enable shader
		this->shader->enable();

		// Multi pass render
		int num_lights = (int)Application::instance->light_list.size();
		for (int nlight = -1; nlight < num_lights; nlight++)
		{
			if (nlight == -1) { nlight++; } // hotfix

			// Upload uniforms
			setUniforms(camera, model);

			// Upload light uniforms
			if (!first_pass) {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				glDepthFunc(GL_LEQUAL);
			}
			this->shader->setUniform("u_ambient_light", Application::instance->ambient_light * (float)first_pass);

			if (num_lights > 0) {
				Light* light = Application::instance->light_list[nlight];
				light->setUniforms(this->shader, model);
			}
			else {
				// Set some uniforms in case there is no light
				this->shader->setUniform("u_light_intensity", 1.f);
				this->shader->setUniform("u_light_shininess", 1.f);
				this->shader->setUniform("u_light_color", glm::vec4(0.f));
			}

			// Do the draw call
			mesh->render(GL_TRIANGLES);
            
			first_pass = false;
		}

		// Disable shader
		this->shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	ImGui::Text("Material Type: %s", std::string("Standard").c_str());

	if (ImGui::Checkbox("Show Normals", &this->show_normals)) {
		if (this->show_normals) {
			this->shader = this->normal_shader;
		}
		else {
			this->shader = this->base_shader;
		}
	}

	if (!this->show_normals) ImGui::ColorEdit3("Color", (float*)&this->color);
}

VolumeMaterial::VolumeMaterial(glm::vec4 color)
{
	this->color = color;
	this->absorption_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/absorption.fs");
	this->emission_absorption_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/emission-absorption.fs");
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/absorption.fs"); // Default shader
	this->absorption_coeff = 0.15f;
	this->shader_type = 0;
	this->absorption_type = 0;
	this->step_size = 0.015f;
	this->noise_freq = 4.0f;
	this->density_scale = 1.0f;
}

VolumeMaterial::~VolumeMaterial()
{
}

void VolumeMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	// Upload node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	glm::mat4 invModel = glm::inverse(model);
	glm::vec4 temp = glm::vec4(camera->eye, 1.0);
	temp = invModel * temp;
	glm::vec3 local_camera_pos = glm::vec3(temp.x / temp.w, temp.y / temp.w, temp.z / temp.w);
	this->shader->setUniform("u_local_camera_pos", local_camera_pos);

	this->shader->setUniform("u_color", this->color);
	this->shader->setUniform("u_bg_color", Application::instance->background_color);
	this->shader->setUniform("u_absorption_coeff", this->absorption_coeff);

	this->shader->setUniform("u_absorption_type", this->absorption_type);
	this->shader->setUniform("u_step_size", this->step_size);
	this->shader->setUniform("u_noise_freq", this->noise_freq);
	this->shader->setUniform("u_density_scale", this->density_scale);
}

void VolumeMaterial::renderInMenu()
{
	ImGui::Text("Material Type: %s", std::string("Volume").c_str());

	// Absorption shader or Emission-abosrption shader
	const char* shader_types[] = { "Absorption", "Emission-Absorption" };
	if (ImGui::Combo("Shader Type", (int*)&this->shader_type, shader_types, IM_ARRAYSIZE(shader_types)))
	{
		// Change used shader
		if (this->shader_type == 0) this->shader = this->absorption_shader;
		else this->shader = this->emission_absorption_shader;
	}
	// Homogeneous or heterogeneous selector
	const char* types[] = { "Homogeneous", "Heterogeneous" };
	ImGui::Combo("Absorption Type", (int*)&this->absorption_type, types, IM_ARRAYSIZE(types));
	// Homogeneous GUI
	if (this->absorption_type==0) ImGui::SliderFloat("Absorption Coefficient", (float*)&this->absorption_coeff, 0.0f, 5.0f);
	// Heterogeneous GUI
	else {
		ImGui::SliderFloat("Step Size", (float*)&this->step_size, 0.01f, 0.02f);
		ImGui::SliderFloat("Noise Frequency", (float*)&this->noise_freq, 2.5f, 10.0f);
		ImGui::SliderFloat("Density Scale", (float*)&this->density_scale, 0.0f, 10.0f);
	}
	// Emission-Absorption GUI
	if (this->shader_type==1) ImGui::ColorEdit3("Emmited Color", (float*)&this->color);
}