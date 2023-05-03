#pragma once


using namespace osl;

static const std::vector<std::string> timemarks_name = { "обработка эвентов", "отчистка пулов", "динамические объекты" };

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



#include "CA.h"

class WorldCS {
public:
	WorldCS() : update_time(60), 
		timemarks(timemarks_name.size(), fastLinearFilter(300)), 
		timemarks_ll(timemarks_name.size(), fastLinearFilter(10)),
		CA(1200, 200)
	{}

	Randomaizer RAND;

	struct View {
		vec2 pos = vec2(0, 0); // Позиция центра камеры (0 - центр субстрата)
		frac mst = 1.; // Масштаб зрения (чем больше, тем меньше выглядит мир)
	};
	View view;

	struct Rules {
		std::vector<BlockType> static_blocks;
		std::vector<BlockType> dynamic_blocks;
		std::vector<BlockType> grow_and_extra_blocks;

		int min_terra_y = 2, max_terra_y = 120;
		uint64_t seed;
	};
	Rules rules;

	int task = 0;
	bool auto_run = 1;

	bool is_open;
	ivec2 size = ivec2(1200, 200);

	std::string name = "Unnamed";
	void iniWorld();
	void genMap();
	void renderWorld();
	void shutdown() {
		is_open = 0;
	};

	std::vector<uint8_t> color_map;
	uint32_t* color_map_u32;
	std::vector<uint8_t> static_map;
	std::vector<uint32_t> dynamic_map;
	std::vector<uint32_t> grow_map;

	PoolContainer<int> dynamic_pool;

	std::queue<WorldCommand> event_pool;

	inline int getIndex(ivec2 p) {
		return (p[0] + p[1] * size[0]);
	}
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
	CellularAutomation CA;
};

#include "life.h"

// Инициализация/сброс мира
void WorldCS::iniWorld() {
	RAND.ini();

	view.pos = size / 2.;
	view.mst = size[1] + 2;

	Direction = {-size[0], -1, +1, size[0] };

	rules.static_blocks.clear();
	BlockType b;
	b.name = "земля";
	b.light_absorption = 0.;
	b.type = b.TPS::STATIC_BLOCK;
	rules.static_blocks.push_back(b);
	genMap();

	
	b.name = "семечко 1";
	b.light_absorption = 1.;
	b.type = b.TPS::DYNAMIC_BLOCK;

//	dynamic_map.resize(size[0] * size[1]);
//	std::fill(dynamic_map.begin(), dynamic_map.end(), -1);
}

// Генерация мира
void WorldCS::genMap() {
	color_map.resize(size[0] * size[1] * 4);
	std::fill(color_map.begin(), color_map.end(), 255);
	color_map_u32 = (uint32_t *)(&color_map[0]);

	static_map.resize(size[0] * size[1]);
	std::fill(static_map.begin(), static_map.end(), -1);


	//генерация земли
	if (0) {
		
		uint32_t Ymin = rules.min_terra_y, Ymax = rules.max_terra_y; //пределы высоты поверхности
		int Xp = size[0], Yp = size[1];
		double mst = 30.f;

		double Yminf = 1e6f, Ymaxf = -1e6f;
		std::vector<double> Ydirtf(Xp * 2);
		std::vector<uint32_t> Ydirt(Xp), Ydirt2(Xp);


		for (int x = 0; x < Xp; x++) {
			Ydirtf[x] = badPrlRand(x * 1, mst, (RAND.getMainSeed() / 4294967296.)) * 2.;
			Yminf = fmin(Yminf, Ydirtf[x]);
			Ymaxf = fmax(Ymaxf, Ydirtf[x]);
		}

		{
			double trig = (Ymaxf - Yminf) / 10.f;
			double comp = (Ymaxf + Yminf) / 2.f;
			for (int x = 0; x < Xp; x++) {
				if ((Ydirtf[x] - comp) > trig || (Ydirtf[x] - comp) < -trig)

					Ydirtf[x] = (Ydirtf[x] + comp) / 2.f;
			}
		}

		for (int x = 0; x < Xp; x++) {
			double sum = 0;
			for (int x1 = -40; x1 < 40; x1++)
				sum += Ydirtf[saw(x - x1, Xp)];
			Ydirtf[saw(x, Xp) + Xp] = sum / 80.;
		}

		Yminf = 1e6f; Ymaxf = -1e6f;
		for (int x = 0; x < Xp; x++) {
			Yminf = fmin(Yminf, Ydirtf[x]);
			Ymaxf = fmax(Ymaxf, Ydirtf[x]);
		}

		for (int x = 0; x < Xp; x++) {

			Ydirtf[x + Xp] = (Ydirtf[x + Xp] - Yminf) / (Ymaxf - Yminf);
			Ydirt[x] = int(floor(Ydirtf[x + Xp] * (Ymax*1.)));

		}


		for (int x = 0; x < Xp; x++)
			if (Ydirt[x] < Ymin) { Ydirt[x] = Ymin; }
			else
				if (Ydirt[x] > Ymax) { Ydirt[x] = Ymax; }
		Ydirt2 = Ydirt;
		for (int x = 1; x < Xp - 1; x++)
			Ydirt2[x] = std::min(std::min(Ydirt[x - 1], Ydirt[x + 1]), Ydirt2[x]) - 1;

		
		for (int x = 0; x < Xp; x++)
			for (int y = 0; y < Yp; y++) {
				if (uint32_t(y) <= Ydirt[x]) {
					static_map[x + y * size[0]] = 0;
					if (uint32_t(y) < Ydirt2[x]) {
						color_map[(x + y * Xp) * 4 + 0] = 50 + badPrlRand(x, 1, y + sin(x * 0.01) * 0.01) * 15 - badPrlRand(x, 1, y + 3) * 3;
						color_map[(x + y * Xp) * 4 + 1] = 40 + badPrlRand(x, 1, y + sin(x * 0.01) * 0.01) * 15 - badPrlRand(x, 1, y + 5) * 3;
						color_map[(x + y * Xp) * 4 + 2] = 35 + badPrlRand(x, 1, y + sin(x * 0.01) * 0.01) * 15 - badPrlRand(x, 1, y + 1) * 3;
					}
					else if (uint32_t(y) <= Ydirt2[x]) {
						color_map[(x + y * Xp) * 4 + 0] = 50 - badPrlRand(x, 2, y + sin(x * 0.01) * 0.01) * 15;
						color_map[(x + y * Xp) * 4 + 1] = 70 - badPrlRand(x, 2, y + sin(x * 0.01) * 0.01) * 15;
						color_map[(x + y * Xp) * 4 + 2] = 50 - badPrlRand(x, 2, y + sin(x * 0.01) * 0.01) * 15;
					}
					else {
						color_map[(x + y * Xp) * 4 + 0] = 55;
						color_map[(x + y * Xp) * 4 + 1] = 100;
						color_map[(x + y * Xp) * 4 + 2] = 55;
					}
				} else
					static_map[x + y * size[0]] = -1;
			}
	}

}

void WorldCS::renderWorld() {
	DeltaTimeMark dtm, upd_tm;
	is_open = 1;
	//	CA.spawn(size[0] / 2, size[1] / 2);
	//	CA.spawn(size[0] / 2 + 1, size[1] / 2);
	//for (int x = size[0] / 2 - 4; x < size[0] / 2 + 4; x++)
	//	CA.spawn(x, size[1] / 5);
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
			dynamic_pool.erase();
			
			timemarks_ll[dtm_id].push(timemarks[dtm_id++].push(dtm.get()));
			
			// динамические объекты
			for (int i = 0; i < dynamic_pool.enabled.size(); i++) {
				int index = dynamic_pool.enabled[i];
				auto &e = dynamic_pool.storage[index];

				color_map[e * 4 + 3] = color_u32(ivec4(255));

				if (static_map[e + Direction[down]] == 255) {
					e += Direction[down];
					color_map[e * 4 + 3] = color_u32(ivec4(150, 150, 189, 255));
				}
				else
					dynamic_pool.erase(i);
			}
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
			CA.step();
			for (int x = 0; x < size[0]; x++)
				for (int y = 0; y < size[1]; y++)
					color_map_u32[x + y * size[0]] = color_arr[CA.get_world_cell_type(x, y)];
			timemarks_ll[dtm_id].push(timemarks[dtm_id++].push(dtm.get()));

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}





// // Пример использования класса PoolContainer
//pc.storage[pc.get_new()] = 12;
//pc.storage[pc.get_new()] = 13;
//pc.storage[pc.get_new()] = 14;
//
//for (int i = 0; i < pc.enabled.size(); i++) {
//	int index = pc.enabled[i];
//	auto& elem = pc.storage[index];
//	if (elem == 14)
//		pc.erase(i);
//}
//
//pc.erase();
//
//for (int i = 0; i < pc.enabled.size(); i++) {
//	int index = pc.enabled[i];
//	auto& elem = pc.storage[index];
//	std::cout << (elem) << std::endl;
//}









/*
int main()
{

	//auto seed = time(NULL);
	//srand(5);

	CellularAutomation a(120, 30);
	a.spawn(40, 10);

	std::vector<char> str(a.length + 1, ' ');
	str[a.length] = '\0';

	const char gradient[] = ".oO T";
	while (true) {

		a.step();
		for (int x = 0; x < 120; x++)
			for (int y = 0; y < 30; y++) {
				str[x + y * 120] = gradient[a.get_world_cell_type(x, 29 - y)];
			}

		std::cout << &str[0];
		//Sleep(200);
	}


	return 0;
}*/