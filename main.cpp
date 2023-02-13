#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>

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
			assert(cx < w && cy < h);
			
			img[cx + cy*w] = color;
		}
	}
}

int main() {
    int w = 512;
    int h = 512;
    
    int map_w = 16;
    int map_h = 16;
    
    // map 0-wall, " " - nothing
    const char map[] =  "0000000000000000"
			"0000000000000000"
			"0000    00000000"
			"0000    00000000"
			"0000          00"
			"000    000000000"
			"000    000000000"
			"000    000000000"
			"000    000000000"
			"000    000000000"
			"000         0000"
			"0000000     0000"
			"000000000   0000"
			"0000   00   0000"
			"0000        0000"
			"0000000000000000";
			
    assert(map_w * map_h+1 == sizeof(map));
    
    std::vector<uint32_t> framebuffer(w*h, 255);   
    
    for(size_t j = 0; j < h; j++){
    	for(size_t i = 0; i < w; i++){
    		uint8_t r = 255*j/float(w);
    		uint8_t g = 0;
    		uint8_t b = 255*i/float(h);
    		
    		framebuffer[i + j*h] = pack_color(r,g,b,255);
    	}
    }
    
    const size_t rect_w = w / map_w;
    const size_t rect_h = h / map_h;
    
    double player_x = 8;
    double player_y = 6;
    
    for(size_t j = 0; j < map_h; j++){
    	for(size_t i = 0; i < map_w; i++){
    		if(map[i + j*map_w] != ' ')
    			continue;
    			
    		size_t rect_x = i * rect_w;
    		size_t rect_y = j * rect_h;
    		
    		draw_rectangle(framebuffer, w, h, rect_x, rect_y, rect_w, rect_h, pack_color(255,255,255,255));
    	}
    }
    
    // player is rectangle with 5x5 size
    draw_rectangle(framebuffer, w, h, player_x*rect_w, player_y*rect_h, 5, 5, pack_color(255,255,255,255));
    
    draw_ppm("./out.ppm", framebuffer, w, h);
}
