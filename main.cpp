#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SFML/Graphics.hpp>

uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
	return (a << 24) + (b << 16) + (g << 8) + r;
}

void unpack_color(const uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a){
	r = (color >> 0) & 255;
	g = (color >> 8) & 255;
	b = (color >> 16) & 255;
	a = (color >> 24) & 255;
}

void draw_ppm(const std::string filename, const std::vector<uint32_t> &img, const size_t w, const size_t h){
	assert(img.size() == w * h);

	std::ofstream ofs(filename, std::ios::binary);

	ofs << "P6\n"
		<< w << " " << h << "\n255\n";

	for (size_t i = 0; i < w * h; i++) {
		uint8_t r, g, b, a;

		unpack_color(img[i], r, g, b, a);

		ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
	}

	ofs.close();
}

void draw_rectangle(std::vector<uint32_t> &img, const size_t w, const size_t h, const size_t x, const size_t y, const size_t rect_w, const size_t rect_h, const uint32_t color){
	assert(img.size() == w * h);

	for (size_t i = 0; i < rect_w; i++) {
		for (size_t j = 0; j < rect_h; j++) {
			size_t cx = x + i;
			size_t cy = y + j;
			if (cx >= w || cy >= h)
				continue;

			img[cx + cy * w] = color;
		}
	}
}

bool load_texture(std::vector<uint32_t>& texture, std::string filename, size_t &text_size, size_t &text_cnt){
	int nch = -1, w, h;

	unsigned char *pixmap = stbi_load(filename.c_str(), &w, &h, &nch, 0);

	if(!pixmap){
		std::cerr << "ERROR : CAN'T OPEN FILE" << std::endl;
		return false;
	}

	if(nch != 4){
		std::cerr << "ERROR : MY PROGRAM SUPPORTS ONLY 32BIT IMAGES (R,G,B,A)" << std::endl;
		stbi_image_free(pixmap);
		return false;
	}

	text_cnt = w/h;
	text_size = w/text_cnt;

	if(w != h*int(text_cnt)){
		std::cerr << "ERROR : THERE ARE NO N SQUARE IMAGES" << std::endl;
		stbi_image_free(pixmap);
		return false;
	}

	texture = std::vector<uint32_t>(w*h);

	for(int j = 0; j < h; j++){
		for(int i = 0; i < w; i++){
			uint8_t r = pixmap[(i + j*w) * 4 + 0];
			uint8_t g = pixmap[(i + j*w) * 4 + 1];
			uint8_t b = pixmap[(i + j*w) * 4 + 2];
			uint8_t a = pixmap[(i + j*w) * 4 + 3];

			texture[i+j*w] = pack_color(r,g,b,a);
		}
	}

	stbi_image_free(pixmap);

	return true;
}

std::vector<uint32_t> texture_column(const std::vector<uint32_t>& img, const size_t texsize, const size_t ntex, const size_t texid, const size_t texcoord, const size_t column_height){
	const size_t img_w = texsize * ntex;
	const size_t img_h = texsize;

	assert(img.size() == img_w*img_h && texcoord < texsize && texid < ntex);

	std::vector<uint32_t> column(column_height);

	for(size_t y = 0; y < column_height; y++){
		size_t pix_x = texid*texsize + texcoord;
		size_t pix_y = (y*texsize)/column_height;
		column[y] = img[pix_x + pix_y*img_w];
	}

	return column;
}

int main(){
	int w = 1024;
	int h = 512;

	sf::RenderWindow window(sf::VideoMode(w, h), "RayCaster", sf::Style::Close|sf::Style::Titlebar);


	double player_a = 1.4;

	while(window.isOpen()){

        sf::Event event;

        while(window.pollEvent(event)){
            if(event.type == sf::Event::Closed)
                window.close();

			if(event.type == sf::Event::KeyPressed){
				if(event.key.code == sf::Keyboard::Left){
					player_a -= 2*M_PI/360;
				} else if(event.key.code == sf::Keyboard::Right){
					player_a += 2*M_PI/360;
				}
			}
        }

		int map_w = 16;
		int map_h = 16;

		// map 0-wall, " " - nothing
		const char map[] = "0022222222220000"
							"0              0"
							"0      00000   0"
							"1     0        0"
							"1     0  0000000"
							"1     0        0"
							"1   04440      0"
							"0   0   0000   0"
							"0   5   3      0"
							"0   0   3  00000"
							"0       2      0"
							"0       2      0"
							"0       0      0"
							"0 1105500      0"
							"0              0"
							"0000000000000000";

		assert(map_w * map_h + 1 == sizeof(map));

		const size_t rect_w = w / (map_w * 2);
		const size_t rect_h = h / map_h;

		std::vector<uint32_t> walltext;
		size_t walltext_cnt;
		size_t walltext_size;

		if(!load_texture(walltext, "./walltext.png", walltext_size, walltext_cnt)){
			std::cerr << "Something wrong happened. Check errors before this" << std::endl;
			return -1;
		}

		double player_x = 6.5;
		double player_y = 8.5;
		const double fov = M_PI / 3;

		std::vector<uint32_t> framebuffer(w * h, pack_color(255, 255, 255, 255));

		for (size_t j = 0; j < map_h; j++){
			for (size_t i = 0; i < map_w; i++){
				if (map[i + j * map_w] == ' ')
					continue;

				size_t rect_x = i * rect_w;
				size_t rect_y = j * rect_h;
				size_t color_idx = map[i+j*map_w] - '0';
				assert(color_idx < walltext_cnt);

				draw_rectangle(framebuffer, w, h, rect_x, rect_y, rect_w, rect_h, walltext[color_idx*walltext_size]);
			}
		}

		// player is rectangle with 5x5 size
		draw_rectangle(framebuffer, w, h, player_x * rect_w, player_y * rect_h, 5, 5, pack_color(255, 255, 255, 255));

		for (size_t i = 0; i < w / 2; i++) {
			double ang = player_a - fov / 2 + fov * i / float(w);

			for (float t = 0; t < 20; t += .01){
				float cx = player_x + t * cos(ang);
				float cy = player_y + t * sin(ang);

				size_t pix_x = cx * rect_w;
				size_t pix_y = cy * rect_h;
				framebuffer[pix_x + pix_y * w] = pack_color(160, 160, 160, 255);

				if (map[int(cx) + int(cy) * map_w] != ' ') {
					size_t column_height = h / (t * cos(ang - player_a));
					size_t color_idx = map[int(cx) + int(cy) * map_w] - '0';
					assert(color_idx < walltext_cnt);

					// draw_rectangle(framebuffer, w, h, w / 2 + i, h / 2 - column_height / 2, 1, column_height, walltext[color_idx*walltext_size]);

					float hitx = cx - (long)cx;
					float hity = cy - (long)cy;

					int x_texcoord = (std::abs(hity) > std::abs(hitx)) ? hity * walltext_size : hitx * walltext_size;

					assert(x_texcoord >= 0 && x_texcoord < (int)walltext_size);

					std::vector<uint32_t> col = texture_column(walltext, walltext_size, walltext_cnt, color_idx, x_texcoord, column_height);

					pix_x = w/2 + i;

					for(size_t j = 0; j < column_height; j++){
						pix_y = j + h/2 - column_height/2;
						if(pix_y < 0 || pix_y >= (int) h) continue;

						framebuffer[pix_x + pix_y * w] = col[j];
					}
					
					break;
				}
			}
		}

		sf::Vertex **rendermap = new sf::Vertex*[h];

        for(int i = 0; i < h; i++){
            rendermap[i] = new sf::Vertex[w];
        }

        for(int i = 0; i < h; i++){
            for(int j = 0; j < w; j++){
				uint8_t r, g, b, a;
				unpack_color(framebuffer[j+i*w], r, g, b, a);
				sf::Color color(r,g,b,a);
                rendermap[i][j] = sf::Vertex(sf::Vector2f(j+.5f, i+5.f), color);
            }
        }

        for(int i = 0; i < h; i++){
            window.draw(rendermap[i], w, sf::Points);
        }

        window.display();
		
		// do not forget OR GG BB COMPUTER
		
		for(int i = 0; i < h; i++){
			delete[] rendermap[i];
        }

		delete[] rendermap;
	}

	return 0;
}
