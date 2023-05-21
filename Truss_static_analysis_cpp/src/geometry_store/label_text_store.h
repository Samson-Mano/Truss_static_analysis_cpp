#pragma once
#include <iostream>
#include <glm/vec2.hpp>
#include "buffers/gBuffers.h"
#include "buffers/font_atlas.h"
#include "geom_parameters.h"

struct label_text
{
	// Store the individual label
	std::string label;
	glm::vec2 label_loc;
	glm::vec3 label_color;
	float label_angle;
	float label_size;
	bool label_above_loc;
};

class label_text_store
{
	// Stores all the labels
public:
	geom_parameters* geom_param_ptr;
	// font_atlas* main_font;
	unsigned int total_char_count = 0;
	gBuffers label_buffers;
	std::vector<label_text> labels;
	// float geom_scale = 1.0f;

	label_text_store();
	~label_text_store();
	// void init(font_atlas* font);
	void add_text(std::string text, glm::vec2 text_loc, glm::vec3 text_color, float font_angle,bool above_point);
	void set_buffers();
	void paint_text();
	void delete_all();
private:
	void get_buffer(label_text& label, float* vertices, 
		unsigned int& vertex_index, unsigned int* indices, unsigned int& indices_index);
	glm::vec2 rotate_pt(glm::vec2& rotate_about, glm::vec2 pt, float& rotation_angle);

};
