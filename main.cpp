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

int main() {
    int w = 512;
    int h = 512;
    
    std::vector<uint32_t> framebuffer(w*h, 255);
    
    for(size_t j = 0; j < h; j++){
    	for(size_t i = 0; i < w; i++){
    		uint8_t r = 255*j/float(w);
    		uint8_t g = 0;
    		uint8_t b = 255*i/float(h);
    		
    		framebuffer[i + j*h] = pack_color(r,g,b,255);
    	}
    }
    
    draw_ppm("./out.ppm", framebuffer, w, h);
}
