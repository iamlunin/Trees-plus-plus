#pragma once

using namespace osl;

#include "CA.h"



static const std::vector<std::string> timemarks_name = { "обработка эвентов", "симуляция", "отрисовка" };

int poz(int x, int y) {
	return (x < 0) ? (x + y) : ((x >= y) ? (x - y) : (x));
}

int saw(int x, int y) { return(((x%y) + y) % y); }

inline uint32_t color_u32(ivec4 c) {
	return uint8_t(c[0]) + (uint8_t(c[1]) << 8) + (uint8_t(c[2]) << 16) + (uint8_t(c[3]) << 24);
}

class BlockType;
class CellularAutomation;

class WorldCommand {
public:
	static WorldCommand paintBlock(ivec2 ipos, uint32_t color) {
		WorldCommand r;
		r.type = PAINT_BLOCK;
		r.data = color;
		r.pos = ipos;
		return r;
	}

	enum {
		PAINT_BLOCK
	};

	int type;
	uint32_t data;
	ivec2 pos;
};

// -123 32 -123 25 46 34 -123 24 
// 1 3 4 7 5
// 0 2 6

// -123 -123 -123 25 46 34 -123 24 
// -1 3 4 7 5
// 0 2 6 1





class WorldCS {
public:
	CellularAutomation CA;

	WorldCS(int w, int h) : update_time(1000), 
		timemarks(timemarks_name.size(), fastLinearFilter(300)), 
		timemarks_ll(timemarks_name.size(), fastLinearFilter(10)),
		CA(w, h)
	{
		size = ivec2(w, h);
	}


	struct View {
		vec2 pos = vec2(0, 0); // Позиция центра камеры (0 - центр субстрата)
		frac mst = 1.; // Масштаб зрения (чем больше, тем меньше выглядит мир)
	};
	View view;

	

	int task = 0;
	bool auto_run = 1;

	bool is_open;

	ivec2 size;

	
	void iniWorld();

	void renderWorld();
	void shutdown() {
		is_open = 0;
	};

	//std::vector<uint8_t> color_map;
	uint8_t* color_map;




	std::queue<WorldCommand> event_pool;


	enum { down, left, right, up };
	std::vector<int> Direction;
	class Mode;
	class Genome;
	class Bank;
	class Particle;
	class FreeParticle;
	class Trees;
	


	int count_of_updates = 0;
	fastLinearFilter update_time;
	std::vector<fastLinearFilter> timemarks;
	std::vector<fastLinearFilter> timemarks_ll;

private:
	
};


// Инициализация/сброс мира
void WorldCS::iniWorld() {
	view.pos = size / 2.;
	view.mst = size[1] + 2;

	//color_map.resize(size[0] * size[1]*4, 255);

	color_map = &CA.color_map[0];
}


void WorldCS::renderWorld() {
	DeltaTimeMark dtm, upd_tm;
	is_open = 1;

	while (is_open) {
		
		if (task > 0 || auto_run == 1) {
			if (task > 0)
				task--;
			count_of_updates++;
			dtm.get();
			int dtm_id = 0;

			update_time.push(upd_tm.get());

			// Обновление таймеров
			if (count_of_updates % 100000 == 0) {
				update_time.resum();
				for (int i = 0; i < timemarks.size(); i++)
					timemarks[i].resum(), timemarks_ll[i].resum();
			}

			// обработка эвентов
			while (!event_pool.empty()) {
				auto e = event_pool.front();
				event_pool.pop();
				switch (e.type) {
				case WorldCommand::PAINT_BLOCK:
					//if (getIndex(e.pos) >= 0 && getIndex(e.pos) < static_map.size())
						//dynamic_pool.storage[dynamic_pool.get_new()] = getIndex(e.pos);
	//					paintBlock(getIndex(e.pos), e.data);
					break;
				}
			}
			
			timemarks_ll[dtm_id].push(timemarks[dtm_id++].push(dtm.get()));
			// отчистка пулов
			
			CA.step();

			timemarks_ll[dtm_id].push(timemarks[dtm_id++].push(dtm.get()));
			

			enum {
				semen = 0,
				green = 1,
				wood = 2,
				air = 3,
				ground = 4,
			};
			static const uint32_t color_arr[] = { 
				color_u32(ivec4(0, 0, 0, 255)),
				color_u32(ivec4(255, 103, 61, 255)),
				color_u32(ivec4(128, 143, 255, 255)),
				color_u32(ivec4(255)), 
				color_u32(ivec4(60, 50, 40, 250))
			};
			
			for (int x = 0; x < size[0]; x++)
				for (int y = 0; y < size[1]; y++)
					CA.color_map_u32[x + y * size[0]] = color_arr[CA.get_world_cell_type(x, y)];
			timemarks_ll[dtm_id].push(timemarks[dtm_id++].push(dtm.get()));

			//std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}