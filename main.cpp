#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cmath>

uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
	return (a << 24) + (b << 16) + (g << 8) + r;
}

void unpack_color(const uint32_t color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a){
	r = (color >> 0) & 255;
	g = (color >> 8) & 255;
	b = (color >> 16) & 255;
	a = (color >> 24) & 255;
}

void draw_ppm(const std::string filename, const std::vector<uint32_t>& img, const size_t w, const size_t h){
	assert(img.size() == w * h);
	
	std::ofstream ofs(filename, std::ios::binary);
	
	ofs << "P6\n" << w << " " << h << "\n255\n";
	
	for(size_t i = 0; i < w*h; i++){
		uint8_t r,g,b,a;
		
		unpack_color(img[i], r, g, b, a);
		
		ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b); 
	}
	
	ofs.close();
}

void draw_rectangle(std::vector<uint32_t>& img, const size_t w, const size_t h, const size_t x, const size_t y, const size_t rect_w, const size_t rect_h, const uint32_t color){
	assert(img.size() == w*h);
	
	for(size_t i = 0; i < rect_w; i++){
		for(size_t j = 0; j < rect_h; j++){
			size_t cx = x + i;
			size_t cy = y + j;
			if(cx >= w || cy >= h) continue;
			
			img[cx + cy*w] = color;
		}
	}
}

int main() {
    int w = 1024;
    int h = 512;
    
    int map_w = 16;
    int map_h = 16;
    
    // map 0-wall, " " - nothing
    const char map[] = "0000000000000000"\
                       "0              0"\
                       "0      00000   0"\
                       "0     0        0"\
                       "0     0  0000000"\
                       "0     0        0"\
                       "0   00000      0"\
                       "0   0   0000   0"\
                       "0   0   0      0"\
                       "0   0   0  00000"\
                       "0       0      0"\
                       "0       0      0"\
                       "0       0      0"\
                       "0 0000000      0"\
                       "0              0"\
                       "0000000000000000";
			
    assert(map_w * map_h+1 == sizeof(map));
            
    const size_t rect_w = w /(map_w*2);
    const size_t rect_h = h / map_h;
    
    double player_x = 6.5;
    double player_y = 8.5;
	double player_a = 1.234;
	const double fov = M_PI/3;

	for(size_t j = 0; j < 360; j++){
    	std::vector<uint32_t> framebuffer(w*h, pack_color(255,255,255,255));   
		
    
		for(size_t j = 0; j < map_h; j++){
			for(size_t i = 0; i < map_w; i++){
				if(map[i + j*map_w] != ' ')
					continue;
					
				size_t rect_x = i * rect_w;
				size_t rect_y = j * rect_h;
				
				draw_rectangle(framebuffer, w, h, rect_x, rect_y, rect_w, rect_h, pack_color(255,0,0,255));
			}
		}
		
		// player is rectangle with 5x5 size
		draw_rectangle(framebuffer, w, h, player_x*rect_w, player_y*rect_h, 5, 5, pack_color(255,255,255,255));

		player_a += 2*M_PI/360;
    
		for(size_t i = 0; i < w/2; i++){
			double ang = player_a - fov/2 + fov*i/float(w);

			for (float t=0; t<20; t+=.01) {
				float cx = player_x + t*cos(ang);
				float cy = player_y + t*sin(ang);
				if (map[int(cx)+int(cy)*map_w]!=' '){
					size_t column_height = h/t;

					draw_rectangle(framebuffer, w, h, w/2+i, h/2-column_height/2, 1, column_height, pack_color(255,0,0,255));
					break;
				}

				size_t pix_x = cx*rect_w;
				size_t pix_y = cy*rect_h;
				framebuffer[pix_x + pix_y*w] = pack_color(160, 160, 160, 255);
			}
		}
    
    	draw_ppm("./out" + std::to_string(j) + ".ppm", framebuffer, w, h);
	}

	return 0;
}
