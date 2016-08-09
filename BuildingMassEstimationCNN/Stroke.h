#pragma once

#include <glm/gtc/matrix_transform.hpp>

class Stroke {
public:
	static enum { TYPE_LINE = 0, TYPE_VERTICAL };

public:
	glm::vec2 start;
	glm::vec2 end;
	int type;

public:
	Stroke() {}
	Stroke(const glm::vec2& start, const glm::vec2& end) : start(start), end(end) {}
};