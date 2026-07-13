#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

inline glm::mat4 create_model_matrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1)); // yaw — Z up
	model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	model = glm::scale(model, scale);
	return model;
}

inline std::string vec3_str(const glm::vec3& v)
{
  return "(" + std::to_string(v.x) + ", "
             + std::to_string(v.y) + ", "
             + std::to_string(v.z) + ")";
}
