#include <algorithm>

#include "spritemanager.hpp"
#include "basesprite.hpp"

#include <fstream>
#include <vector>
#include <stdexcept>
#include <iostream>
#define DEBUG
static std::vector<std::uint8_t> read_spv(const char* path) { //we are going to output a uint8_t vector such that it can be read by cpu then passed into gpu so it has the shader data we will use. Only to have a function thats used, not a called for api public.
	std::ifstream file(path, std::ios::binary | std::ios::ate); //open the file we find at path (spv), reading it in binary and starting at the end. Since we start at the end, when we get position, we know the size.
	if (!file) {
		std::cout << "Cannot open shader file"; //checks if the file works opening exist etc.
	}
	auto size = static_cast<std::streamsize>(file.tellg()); //Get size since we at the end bc we get our position. we turn it into stream size bc otherwise it might be a strange datatype. idk .-.
	file.seekg(0, std::ios::beg);  //we move to the start woohoo
	std::vector<std::uint8_t> data(static_cast<std::size_t>(size)); // we make the vector we will return with exactly the amount of elements we need. They all empty and we will write into them.
	if (!file.read(reinterpret_cast<char*>(data.data()), size)) { //read the file and place the data inside our uint8_t vector, treating the pointer (data.data()) as char*, it's compatible since they the same size and behave the same, and we reinterpret since read expects a c char pointer.
		std::cout << "Failed to read shader file: " ;
	}
	return data; //we give back the uint8_t vector

}
struct spriteshad {
	SDL_GPUGraphicsPipeline* pipeline;
	SDL_GPUSampler* sampler;
	SDL_GPUBuffer* buffer;
	int currbuffsize;
};

spriteshad Spritehandler;
SDL_Window* window;
SDL_GPUDevice* device{};
SDL_GPUCommandBuffer* cmd;

void loadGraphic(const std::string& path, SDL_GPUTexture*& texture, int& sizex, int& sizey) {
	std::string name = path;

	if (false) {}
	else {
		SDL_Surface* surface = SDL_LoadPNG(path.c_str());
		if (!surface) {
			std::cout << "COULD NOT LOAD PNG. PLEASE TRY AGAIN";
			SDL_Quit();
		}
		else {
			SDL_Surface* converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
#ifdef DEBUG
			std::cout << "yay";
#endif
			surface = converted;
			SDL_LockSurface(surface);
			SDL_GPUTextureCreateInfo textinfo{};
			textinfo.type = SDL_GPU_TEXTURETYPE_2D;
			textinfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
			textinfo.width = surface->w;
			textinfo.height = surface->h;
			textinfo.layer_count_or_depth = 1;
			textinfo.num_levels = 1;
			textinfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

#ifdef DEBUG
			std::cout << "TEXTINFO DATA";
			std::cout << textinfo.height;
			std::cout << textinfo.width;
#endif


			SDL_GPUTexture* my_texture = SDL_CreateGPUTexture(device, &textinfo);


#ifdef DEBUG
			std::cout << "CREATING TRANS BUFFER!";
#endif

			
			SDL_GPUTransferBufferCreateInfo transinfo{};
			transinfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
			transinfo.size = surface->w * surface->h * 4;
			sizex = surface->w;
			sizey = surface->h;
			auto* transfer = SDL_CreateGPUTransferBuffer(device, &transinfo);
#ifdef DEBUG
			std::cout << "ABOUT TO MAP!";
#endif

			void* map = SDL_MapGPUTransferBuffer(device, transfer, false);

			Uint8* imagecurr = static_cast<Uint8*>(surface->pixels);
			Uint8* end = static_cast<Uint8*>(map);
			for (int y = 0; y < surface->h; ++y) {
				int truebottom = surface->h - 1 - y;
				SDL_memcpy(end + truebottom * surface->w * 4, imagecurr + y * surface->pitch, surface->w * 4);
			}
			SDL_UnmapGPUTransferBuffer(device, transfer);

			SDL_GPUCommandBuffer* upload = SDL_AcquireGPUCommandBuffer(device);
			SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(upload);
#ifdef DEBUG
			std::cout << "COPYPASS BEGIN";
#endif

			SDL_GPUTextureTransferInfo thingy;
			thingy.transfer_buffer = transfer;
			thingy.pixels_per_row = surface->w;
			thingy.rows_per_layer = surface->h;
			thingy.offset = 0;

			SDL_GPUTextureRegion final{};
		final.texture = my_texture;
		final.w = surface->w;
		final.h = surface->h;
		final.d = 1;
#ifdef DEBUG
		std::cout << "final texture data";
		std::cout << final.h;
		std::cout << final.w;
#endif
		
			
			SDL_UploadToGPUTexture(copy, &thingy, &final, false);
			SDL_EndGPUCopyPass(copy);
			SDL_SubmitGPUCommandBuffer(upload);
			SDL_ReleaseGPUTransferBuffer(device, transfer);
			texture = my_texture;
			if (!texture) {
				std::cout << "Texture is wrong here! GAYSEX GAYSEX GAYSEX GAYSEX";
			}
			std::cout << texture;

		}
	}

}
void getbuffer(uint32_t size) {
	SDL_ReleaseGPUBuffer(device,Spritehandler.buffer);
	SDL_GPUBufferCreateInfo buffinf{};
	buffinf.size = size;
	buffinf.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
	Spritehandler.buffer = SDL_CreateGPUBuffer(device, &buffinf);
	Spritehandler.currbuffsize = size;
}
void setupspritehandlershader() {

	SDL_GPUSamplerCreateInfo samplerinfo{};
	samplerinfo.min_filter = SDL_GPU_FILTER_LINEAR;
	samplerinfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	samplerinfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
	samplerinfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplerinfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplerinfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;


	Spritehandler.sampler = SDL_CreateGPUSampler(device, &samplerinfo);
	SDL_GPUBufferCreateInfo buffinf{};
	buffinf.size = 64;
	buffinf.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
	Spritehandler.buffer = SDL_CreateGPUBuffer(device, &buffinf);
	Spritehandler.currbuffsize = 64;
	std::vector < uint8_t > vert;
	std::vector < uint8_t > frag;
	
		vert = read_spv("shaders/spritehandler.vert.spv"); //now we have the spv data.
		frag = read_spv("shaders/spritehandler.frag.spv");

	
	
	
	
	
	SDL_GPUShaderCreateInfo vertinfo{};
	SDL_GPUShaderCreateInfo faginfo {};
	vertinfo.code_size = vert.size(); //so the gpu knows how many bytes to read
	vertinfo.code = vert.data(); //data
	vertinfo.entrypoint = "main"; //in spv, where we start
	vertinfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
	vertinfo.stage = SDL_GPU_SHADERSTAGE_VERTEX;
	vertinfo.num_samplers = 0;  //Just setting a rect
	vertinfo.num_storage_textures = 0;
	vertinfo.num_storage_buffers = 1;
	vertinfo.num_uniform_buffers = 1;
	vertinfo.props = 0;

	SDL_GPUShader* vert_shader = SDL_CreateGPUShader(device, &vertinfo);

	faginfo.code_size = frag.size();
	faginfo.code = frag.data();
	faginfo.entrypoint = "main";
	faginfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
	faginfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	faginfo.num_samplers = 1; //because we only use one sampler thingy
	faginfo.num_storage_textures = 0;
	faginfo.num_storage_buffers = 0;
	faginfo.num_uniform_buffers = 0;
	faginfo.props = 0;
	SDL_GPUShader* frag_shader = SDL_CreateGPUShader(device, &faginfo);




	SDL_GPUGraphicsPipelineCreateInfo pipeinf{ };
	pipeinf.vertex_shader = vert_shader;
	pipeinf.fragment_shader = frag_shader;

	SDL_GPUVertexBufferDescription vertbuff_desc{}; //i dont yet understand ts :broken heart:
	vertbuff_desc.slot = 0;
	vertbuff_desc.pitch = 0;
	vertbuff_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

	pipeinf.vertex_input_state.num_vertex_buffers = 0;
	pipeinf.vertex_input_state.vertex_buffer_descriptions = nullptr;
	pipeinf.vertex_input_state.num_vertex_attributes = 0;
	pipeinf.vertex_input_state.vertex_attributes = nullptr;

	
	pipeinf.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST; //tri angel

	
	pipeinf.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	pipeinf.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
	pipeinf.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;


	pipeinf.depth_stencil_state.enable_depth_test = false;
	pipeinf.depth_stencil_state.enable_depth_write = false;

	
	pipeinf.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;

	
	SDL_GPUColorTargetDescription colordesc{};
	colordesc.format = SDL_GetGPUSwapchainTextureFormat(device, window); //copied bc idk this
	colordesc.blend_state.enable_blend = true;
	colordesc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
	colordesc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
	colordesc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	colordesc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	colordesc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	colordesc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	pipeinf.target_info.num_color_targets = 1; //we only have one window lol and we only draw to one texture at a time
	pipeinf.target_info.color_target_descriptions = &colordesc;
	pipeinf.target_info.has_depth_stencil_target = false;

	Spritehandler.pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeinf);
	if (!Spritehandler.pipeline) {

		std::exit(EXIT_FAILURE);
	}
}
	

void setupGPUrender(SDL_GPUDevice* dev, SDL_Window* wind) {

	if (dev == NULL) {
		device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, 1, NULL);
		window = wind;
		if (!SDL_ClaimWindowForGPUDevice(device, wind)) {
			std::cerr << "Could not set up GPU for render";
		}
		else {
			std::cout << "GPU SET UP PROPERLY";
		}
	}
	else {
		device = dev;
		window = wind;
		if (!SDL_ClaimWindowForGPUDevice(device, wind)) {
			std::cout << "Could not set up GPU for render";
		}

	}
	setupspritehandlershader();
}
SpriteManager sprites;

void SpriteManager::add(FunkinSprite* sprite)
{
    dasprites.push_back(sprite);
}

void SpriteManager::remove(FunkinSprite* sprite)
{
    dasprites.erase(std::remove(dasprites.begin(), dasprites.end(), sprite), dasprites.end());
}

void SpriteManager::clear()
{
    for (FunkinSprite* sprite : dasprites)delete sprite;
    dasprites.clear();
}

void SpriteManager::update(float elapsed)
{
    for (FunkinSprite* sprite : dasprites)
    {
        sprite -> update(elapsed);
    }
}


void SpriteManager::render()
{
	
	SDL_GPUTexture* atlastext = nullptr;

	SDL_GPUCommandBuffer* buffer = SDL_AcquireGPUCommandBuffer(device);

	std::cout << "\n\nbufferpointer: " << buffer << "\n\n";
	SDL_GPUTexture* swapchain = nullptr;
	if (SDL_WaitAndAcquireGPUSwapchainTexture(buffer, window, &swapchain, NULL, NULL)) {

	}
	else {
		SDL_SubmitGPUCommandBuffer(buffer);
		return;
	}


	std::cout << "\n\n" << sizeof(spritedata) * dasprites.size();
	std::cout << "\n\n" << Spritehandler.currbuffsize << "\n\n";
	if (Spritehandler.currbuffsize < sizeof(spritedata) * dasprites.size()) { //gotta make some functions that can change size of the buffer
		getbuffer(sizeof(spritedata) * dasprites.size());
	}
	else {
		
	}
	struct windowsize {
		std::pair <float, float> windsize;
	};
	windowsize currwind;
	int W = 0;
	int H = 0;

	SDL_GetWindowSize(window, &W, &H);

	currwind.windsize = { (float)W,(float)H };
	SDL_GPUTransferBufferCreateInfo trans{};
	trans.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;

	trans.size = sizeof(spritedata) * dasprites.size();

	trans.props = 0;
	SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(device, &trans);
	if (!transfer) std::cout << "CreateTB: " << SDL_GetError() << "\n";
	void* map = SDL_MapGPUTransferBuffer(device, transfer, false);
	if (!map) std::cout << "MapTB: " << SDL_GetError() << "\n";

	uint8_t* pos = static_cast<uint8_t*> (map);
	for (int i = 0; i < dasprites.size(); i++) {
		auto* sprite = dasprites.at(i);
		auto spritedata = sprite->sprite->getspritedata();

		SDL_memcpy(pos + (i * sizeof(spritedata)), &spritedata, sizeof(spritedata));

	}
	SDL_UnmapGPUTransferBuffer(device, transfer);

	SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(buffer); SDL_GetError();
	SDL_GPUTransferBufferLocation original{ transfer, 0 };
	SDL_GPUBufferRegion tobuff{ Spritehandler.buffer, 0, sizeof(spritedata) * dasprites.size() };


	SDL_UploadToGPUBuffer(copy, &original, &tobuff, false);
		SDL_EndGPUCopyPass(copy); SDL_GetError();

		

		
		SDL_GPURenderPass* pass{};

		SDL_GPUColorTargetInfo colortarget{};
		colortarget.texture = swapchain;
		colortarget.store_op = SDL_GPU_STOREOP_STORE;

		colortarget.load_op = SDL_GPU_LOADOP_CLEAR;

		colortarget.clear_color = SDL_FColor(0.0f, 0.0f, 0.0f, 1.0f);
		pass = SDL_BeginGPURenderPass(
			buffer, &colortarget, 1, nullptr);


		

		SDL_BindGPUGraphicsPipeline(pass, Spritehandler.pipeline);

		SDL_PushGPUVertexUniformData(buffer, 0, &currwind, sizeof(currwind));
		
		SDL_BindGPUVertexStorageBuffers(pass, 0, &Spritehandler.buffer, 1);
		
			int i = 0;
			for (auto sprite : dasprites) {
				
				atlastext = sprite->sprite->gettexture();

				if (!atlastext) {
					std::cerr << "the texture is wrong";
				}
				if (!buffer) {
					std::cerr << "da buffer is rong";
				}



				
				//set the drawing board to none because we still don't know which one we're allowed to use










				SDL_GPUTextureSamplerBinding binding{};
				binding.texture = atlastext;
				binding.sampler = Spritehandler.sampler;
				

				SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);
				
		
				SDL_DrawGPUPrimitives(pass, 6, 1, 0, i); SDL_GetError();
				
				









				
				i++;
			}
		
		SDL_EndGPURenderPass(pass);
		
		SDL_SubmitGPUCommandBuffer(buffer);
		SDL_ReleaseGPUTransferBuffer(device, transfer);

	}


