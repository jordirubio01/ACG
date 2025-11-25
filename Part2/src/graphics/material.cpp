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
	this->full_volume_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/full-volume.fs");
	this->full_volume_isotropic_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/full-volume-isotropic-phase.fs");
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/absorption.fs"); // Current shader
	this->absorption_coeff = 0.5f;
	this->scattering_coeff = 0.5f;
	this->shader_type = 0;
	this->absorption_type = 0;
	this->step_size = 0.015f;
	this->light_steps = 2;
	this->noise_freq = 4.0f;
	this->density_scale = 1.0f;
	this->scattering_scale = 1.0f;
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
	// We compute the local camera position outside the fragment shader
	glm::mat4 invModel = glm::inverse(model);
	glm::vec4 temp = glm::vec4(camera->eye, 1.0);
	temp = invModel * temp;
	glm::vec3 local_camera_pos = glm::vec3(temp.x / temp.w, temp.y / temp.w, temp.z / temp.w);
	this->shader->setUniform("u_local_camera_pos", local_camera_pos);
	// Volume color, background color and absorption coefficient
	this->shader->setUniform("u_color", this->color);
	this->shader->setUniform("u_bg_color", Application::instance->background_color);
	this->shader->setUniform("u_absorption_coeff", this->absorption_coeff);
	this->shader->setUniform("u_scattering_coeff", this->scattering_coeff);
	// Absorption type, ray marching step size, heterogeneous noise freq and density scale
	this->shader->setUniform("u_absorption_type", this->absorption_type);
	this->shader->setUniform("u_step_size", this->step_size);
	this->shader->setUniform("u_light_steps", light_steps);
	this->shader->setUniform("u_noise_freq", this->noise_freq);
	this->shader->setUniform("u_density_scale", this->density_scale);
	this->shader->setUniform("u_scattering_scale", this->scattering_scale);
	// Add the texture if it exists
	if (this->texture) {
		this->shader->setUniform("u_texture", this->texture, 0);
	}
}

void VolumeMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (mesh && this->shader) {
		int num_lights = (int)Application::instance->light_list.size();
		// Enable shader
		this->shader->enable();

		// Upload uniforms
		// Set bounding box (local coordinates)
		this->shader->setUniform("u_boxMin", mesh->aabb_min);
		this->shader->setUniform("u_boxMax", mesh->aabb_max);
		setUniforms(camera, model);

		// Set light uniforms
		//for (int nlight = 0; nlight < num_lights; nlight++) {
		//	Light* light = Application::instance->light_list[nlight];
		//	light->setUniforms(this->shader, model);
		//}
		Light* light = Application::instance->light_list[0];
		light->setUniforms(this->shader, model);

		// Do the draw call
		mesh->render(GL_TRIANGLES);

		this->shader->disable();
	}
}

void VolumeMaterial::renderInMenu()
{
	ImGui::Text("Material Type: %s", std::string("Volume").c_str());

	// Absorption shader or Emission-absorption shader
	const char* shader_types[] = { "Absorption", "Emission-Absorption", "Full-Volume"};
	if (ImGui::Combo("Shader Type", (int*)&this->shader_type, shader_types, IM_ARRAYSIZE(shader_types)))
	{
		// Change used shader
		if (this->shader_type == 0) this->shader = this->absorption_shader;
		else if (this->shader_type == 1) this->shader = this->emission_absorption_shader;
		else this->shader = this->full_volume_isotropic_shader;
	}
	// Homogeneous or heterogeneous selector
	//const char* types[] = { "Homogeneous", "Heterogeneous" };
	const char* types[] = { "Constant density", "3D noise", "VDB file" };
	ImGui::Combo("Absorption Type", (int*)&this->absorption_type, types, IM_ARRAYSIZE(types));
	// Step length (always displayed)
	ImGui::SliderFloat("Step Length", (float*)&this->step_size, 0.01f, 0.02f);
	// Homogeneous GUI
	if (this->absorption_type == 0) {
		ImGui::SliderFloat("Absorption Coefficient", (float*)&this->absorption_coeff, 0.0f, 5.0f);
		if (this->shader_type == 2) ImGui::SliderFloat("Scattering Coefficient", (float*)&this->scattering_coeff, 0.0f, 5.0f);
	}
	// Heterogeneous GUI
	else {
		ImGui::SliderFloat("Absorption Scale", (float*)&this->density_scale, 0.0f, 10.0f);
		if (this->shader_type == 2) ImGui::SliderFloat("Scattering Scale", (float*)&this->scattering_scale, 0.0f, 10.0f);
	}
	if (this->absorption_type == 1) ImGui::SliderFloat("Noise Frequency", (float*)&this->noise_freq, 2.5f, 10.0f);
	// Emission-Absorption extra parameter (emitted color)
	if (this->shader_type != 0) ImGui::ColorEdit3("Emitted Color", (float*)&this->color);
	// Scattering extra parameter (light steps)
	if (this->shader_type == 2) ImGui::SliderInt("Light Steps", (int*)&this->light_steps, 2, 10);
}

void VolumeMaterial::loadVDB(std::string file_path)
{
	easyVDB::OpenVDBReader* vdbReader = new easyVDB::OpenVDBReader();
	vdbReader->read(file_path);

	// now, read the grid from the vdbReader and store the data in a 3D texture
	estimate3DTexture(vdbReader);
}

void VolumeMaterial::estimate3DTexture(easyVDB::OpenVDBReader* vdbReader)
{
	int resolution = 128;
	float radius = 2.0;

	int convertedGrids = 0;
	int convertedVoxels = 0;

	int totalGrids = vdbReader->gridsSize;
	int totalVoxels = totalGrids * pow(resolution, 3);

	float resolutionInv = 1.0f / resolution;
	int resolutionPow2 = pow(resolution, 2);
	int resolutionPow3 = pow(resolution, 3);

	// read all grids data and convert to texture
	for (unsigned int i = 0; i < totalGrids; i++) {
		easyVDB::Grid& grid = vdbReader->grids[i];
		float* data = new float[resolutionPow3];
		memset(data, 0, sizeof(float) * resolutionPow3);

		// Bbox
		easyVDB::Bbox bbox = easyVDB::Bbox();
		bbox = grid.getPreciseWorldBbox();
		glm::vec3 target = bbox.getCenter();
		glm::vec3 size = bbox.getSize();
		glm::vec3 step = size * resolutionInv;

		grid.transform->applyInverseTransformMap(step);
		target = target - (size * 0.5f);
		grid.transform->applyInverseTransformMap(target);
		target = target + (step * 0.5f);

		int x = 0;
		int y = 0;
		int z = 0;

		for (unsigned int j = 0; j < resolutionPow3; j++) {
			int baseX = x;
			int baseY = y;
			int baseZ = z;
			int baseIndex = baseX + baseY * resolution + baseZ * resolutionPow2;

			if (target.x >= 40 && target.y >= 40.33 && target.z >= 10.36) {
				int a = 0;
			}

			float value = grid.getValue(target);

			int cellBleed = radius;

			if (cellBleed) {
				for (int sx = -cellBleed; sx < cellBleed; sx++) {
					for (int sy = -cellBleed; sy < cellBleed; sy++) {
						for (int sz = -cellBleed; sz < cellBleed; sz++) {
							if (x + sx < 0.0 || x + sx >= resolution ||
								y + sy < 0.0 || y + sy >= resolution ||
								z + sz < 0.0 || z + sz >= resolution) {
								continue;
							}

							int targetIndex = baseIndex + sx + sy * resolution + sz * resolutionPow2;

							float offset = std::max(0.0, std::min(1.0, 1.0 - std::hypot(sx, sy, sz) / (radius / 2.0)));
							float dataValue = offset * value * 255.f;

							data[targetIndex] += dataValue;
							data[targetIndex] = std::min((float)data[targetIndex], 255.f);
						}
					}
				}
			}
			else {
				float dataValue = value * 255.f;

				data[baseIndex] += dataValue;
				data[baseIndex] = std::min((float)data[baseIndex], 255.f);
			}

			convertedVoxels++;

			if (z >= resolution) {
				break;
			}

			x++;
			target.x += step.x;

			if (x >= resolution) {
				x = 0;
				target.x -= step.x * resolution;

				y++;
				target.y += step.y;
			}

			if (y >= resolution) {
				y = 0;
				target.y -= step.y * resolution;

				z++;
				target.z += step.z;
			}

			// yield
		}

		// now we create the texture with the data
		// use this: https://www.khronos.org/opengl/wiki/OpenGL_Type
		// and this: https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage3D.xhtml
		this->texture = new Texture();
		this->texture->create3D(resolution, resolution, resolution, GL_RED, GL_FLOAT, false, data, GL_R8);
	}
}