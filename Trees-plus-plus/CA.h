#pragma once

// TODO

// сделать энергию
// сделать свет
// сделать настраиваемую консоль
// сделать ползунок скорости
// можно сделать регулировку полей класса СА через гуи
// можно сделать бенчмарки
// можно сделать раздельный вызов функций become
// можно сделать вероятность мутации генетически детерменированную
// рипит мод
// килл семена
// отображать возраст жизни в более понятном значении


Randomaizer gRAND;

inline uint32_t color_u32(ivec4 c) {
	return uint8_t(c[0]) + (uint8_t(c[1]) << 8) + (uint8_t(c[2]) << 16) + (uint8_t(c[3]) << 24);
}


template<typename T>
class PoolContainer {
public:
	std::vector<T> storage;

	int get_new() {
		if (disabled.empty()) {
			enabled.push_back(storage.size());
			storage.push_back(T());
		}
		else {
			enabled.push_back(disabled.back());
			disabled.pop_back();
		}
		return enabled.back();
	}

	int push(T obj) {
		int index = get_new();
		storage[index] = obj;
		return index;
	}

	void erase(int index) {
		if (index >= 0 && index < enabled.size()) {
			disabled.push_back(enabled[index]), enabled[index] = -1;
		}
	}

	// убирает мусор из enabled и возвращает в disabled
	void erase() {
		enabled.erase(std::remove_if(enabled.begin(), enabled.end(),
			[&](const int& a) { return a < 0; }
		), enabled.end());
	}

	T& operator[] (const int index) {
		return storage[index];
	};

	std::vector<int> enabled;
	std::vector<int> disabled;
};

template <typename T>
void remove(std::vector<T>& v, size_t index) {
	v.erase(v.begin() + index);
}


enum {
	semen = 0,
	green = 1,
	bollet = 2,
	wood = 3,
	air = 4,
	ground = 5,
};

uint8_t r8(int out, int to) { // от 0 до 255 
	return gRAND.u32q() % (to - out) + out;
}



uint8_t generate_gen(int size) {
	return r8(0, size * 2);
}

uint8_t generate_type() {
	return r8(0, 2);
}

uint8_t generate_breeding_age() {
	return r8(0, 5);
}

class Mode {
public:
	std::vector<uint8_t> dir;
	uint8_t type;
	uint8_t breeding_age;
	
	Mode(){

	}

	Mode(int size) {
		dir = { generate_gen(size), generate_gen(size), generate_gen(size), generate_gen(size) };
		type = generate_type();
		breeding_age = generate_breeding_age();
	}

	uint8_t& operator[] (const int index) {
		return dir[index];
	};

	const uint8_t& operator[] (const float index) {
		return dir[int(index)];
	};

};

class Genom {
public:
	int size = 10; // сайз не надо делать более чем 255/2
	std::vector<Mode> modes;
	float max_age;
	ivec3 color;

	

	Genom() {
		max_age = gRAND.pf();
		color = ivec3(gRAND.u8(), gRAND.u8(), gRAND.u8());
		modes.resize(size);
		for (int i = 0; i < size; i++)
			modes[i] = Mode(size);
	}

	Mode& operator[] (const int index) {
		return modes[index];
	};

	const Mode& operator[] (const float index) {
		return modes[int(index)];
	};
};



class Tree {
public:
	int age;
	bool alive;
	int cell_counter;

	uint32_t color_deviation = 0u;
	Genom genom;

	Tree() {
		init();
	}

	Tree(Genom& from_this) {
		init(from_this);
	}

	Tree(Genom& from_this, int mutation) {
		init(from_this, mutation);
	}

	void init(Genom& from_this, int mutation) {
		genom = from_this;
		genom.color = clamp(genom.color + ivec3(vec3(gRAND.nf(), gRAND.nf(), gRAND.nf()) * vec3(10, 10, 10)), ivec3(0), ivec3(255));
		genom.max_age = clamp(genom.max_age + gRAND.nf() * 0.02, 0, 1);
		color_deviation = color_u32(ivec4(genom.color[0], genom.color[1], genom.color[2], 0u));
		for (int i = 0; i < mutation; i++) {
			int index1 = gRAND.u16() % genom.size;
			int index2 = gRAND.u16() % 6;

			if (index2 == 5)
				genom[index1].type = generate_type();
			else if (index2 == 4)
				genom[index1].breeding_age = generate_breeding_age();
			else
				genom[index1][index2] = generate_gen(genom.size);
		
		}
		_init();
	}

	void init(Genom& from_this) {
		genom = from_this;
		_init();
	}

	void init() {
		genom = Genom();
		_init();
	}



private:
	void _init() {
		age = 0;
		cell_counter = 0;
		alive = true;
	}
};



class LiveCell {
public:
	int index;

	uint8_t gen_index;
	int age;
	int index_tree;

	LiveCell(int ind_map, int ind_tree, uint8_t mode) {
		init(ind_map, ind_tree, mode);
	}

	void init(int ind_map, int ind_tree, uint8_t mode) {
		index = ind_map;
		index_tree = ind_tree;
		gen_index = mode;
		age = 0;
	}
};

class Cell {
public:
	uint8_t type;


	Cell(int t) {
		type = t;
	}


};




class CellularAutomation {
public:
	std::vector<uint8_t> color_map;
	uint32_t* color_map_u32;
	PoolContainer<Tree> trees;
	bool task_kill_all = false;
	int max_age;
	int max_cell;
	int width;
	int height;
	int extended_height;
	int length;
	std::vector<Cell> world_map;
	int frame_count = 0;
	int great_spawn_counter = -1;
	enum { up, right, down, left };
	std::vector<int> directions;
	std::vector<LiveCell> live_cell_arr;

	CellularAutomation(int w, int h) {

		color_map.resize(w * h * 4, 255);
		color_map_u32 = (uint32_t*)&color_map[0];
		gRAND.ini();
		max_cell = h * 2;
		max_age = h * 4;
		width = w;
		height = h, extended_height = h + 1;
		length = w * h;
		world_map.resize(width * extended_height, Cell(air));
		
		for (int ind = 0; ind < world_map.size(); ind++) {

			int x = (ind % width);
			int y = (ind / width);

			if (y < 1)
				become(ind, ground);

			if (y >= height)
				world_map[ind].type = ground;
		}

		directions = { width , 1, -width, -1 };
	}



	void spawn(int x, int y) {
		try_spawn_semen(x + y * width);
	}


	int get_world_cell_type(int x, int y) {
		return world_map[x + y * width].type;
	}


	void step() {
		spawn_starting_seeds_if_needed();
		if (task_kill_all) {
			kill_all();
			task_kill_all = false;
		}
		clean_up_index_live_arr();
		cells_handler();
		trees_handler();
		frame_count++;
	}


	void kill_all() {
		for (int ind = live_cell_arr.size()-1; ind >= 0; ind--) {
			LiveCell live_cell = live_cell_arr[ind];
			int index = live_cell.index;
			if (index > 0) {
				kill_cell(ind, live_cell, index);
			}
		}
	}



private:


	void cells_handler() {

		int old_length = live_cell_arr.size();
		for (int ind = 0; ind < old_length; ind++) {

			LiveCell& live_cell = live_cell_arr[ind];
			int index_map = live_cell.index;
			int type = world_map[index_map].type;

			// пофиксить эту строчку
			if (type == semen) {
				int index_target = index_map + directions[down];

				switch (world_map[index_target].type) {

					// СЕМЕНЬ ПАДАЕТ
				case air:
					become(index_map, air);
					
					live_cell.index = index_target;
					become(index_target, semen);
					break;

					// СЕМЕНЬ ПРОРОСТАЕТ
				case ground:
					trees[live_cell.index_tree].cell_counter--;
					live_cell.index_tree = trees.push(Tree(trees[live_cell.index_tree].genom, 1));
					become(index_map, green);
					trees[live_cell.index_tree].cell_counter++;
					break;

					// СЕМЕНЬ УМИРАЕТ
				default:
					kill_cell(ind, live_cell, index_map);
				}
			}
			else {
				// КЛЕТКА УМИРАЕТ
				if (trees[live_cell.index_tree].alive == false
					|| trees[live_cell.index_tree].cell_counter >= max_cell
					) { 
					kill_cell(ind, live_cell, index_map);
					continue;
				}

				// КЛЕТКА ПРОРОСТАЕТ
				if (type == green) {
					live_cell.age++;
					// можно попробовать пофиксить эту строчку
					if (live_cell.age > trees[live_cell.index_tree].genom[live_cell.gen_index].breeding_age)
						try_grow(live_cell.index_tree, index_map, live_cell.gen_index);


				}
			}
		}
	}



	void become(int index, int type, uint32_t color_deviation = 0) {
		static const uint32_t color_arr[] = {
				color_u32(ivec4(0, 0, 0, 255)),			// semen = 0,
				color_u32(ivec4(103, 215, 61, 255)),	// green = 1,
				color_u32(ivec4(233, 103, 61, 255)),	// bullet = 2,
				color_u32(ivec4(0, 0, 0, 255)),	        // wood = 2,
				color_u32(ivec4(255)),					// air = 3,
				color_u32(ivec4(60, 50, 40, 255))		// ground = 4,
		};

		if (type != wood)
			color_map_u32[index] = color_arr[type];
		else
			color_map_u32[index] = color_arr[type] + color_deviation;

		world_map[index].type = type;
	}

	


	void spawn_starting_seeds_if_needed() {
		// спавн новых семечек при пустом поле
		if (live_cell_arr.size() == 0) {
			frame_count = 0;
			great_spawn_counter++;
			for (int i = 0; i < width / 2; i++)
				spawn(rand() % width,  height / 2 + rand() % height / 4 - height / 8);
		}
	}

	void clean_up_index_live_arr() {
		live_cell_arr.erase(std::remove_if(live_cell_arr.begin(), live_cell_arr.end(),
			// если клетка не живая, в ней будет отрицательный индекс, равный положительному эквиваленту
			[&](const auto& a) { if (a.index < 0) { return true; } return false; }
		), live_cell_arr.end());
	}


	void trees_handler() {
		for (int i = 0; i < trees.enabled.size(); i++) {
			auto& tree = trees[trees.enabled[i]];
			if (
				tree.age >= max_age * tree.genom.max_age 
				|| tree.cell_counter <= 0
				//|| tree.cell_counter > max_cell
				) {
				tree.alive = false;
				if (tree.cell_counter <= 0)
					trees.erase(i);
			}
			else
				tree.age++;
		}
		trees.erase();
	}


	void kill_cell(int index, LiveCell& cell, int index_world_map) {
		live_cell_arr[index].index = -live_cell_arr[index].index;
		become(index_world_map, air);
		trees[cell.index_tree].cell_counter--;
	}

	void try_spawn_semen(int ind) {
		if (world_map[ind].type != air)
			return;
		// спавним виртуальное дерево родитель
		spawn(semen, ind, trees.push(Tree()));
	}



	void spawn(int type, int ind, int ind_tree, int ind_gen = 0) {
		live_cell_arr.push_back(LiveCell(ind, ind_tree, ind_gen));
		become(ind, type);
		trees[ind_tree].cell_counter++;
	}

	void try_grow(int index_tree, int index, int gen_index) {
		const Tree& tree = trees[index_tree]; // зачем тут конст
		const Mode& mode = tree.genom.modes[gen_index]; // зачем тут конст

		// ПО КАЖДОМУ ИЗ НАПРАВЛЕНИЙ
		for (int i = 0; i < 4; i++) {
			// ЕСЛИ ЕСТЬ ЖЕЛАНИЕ РАЗМНОЖИТСЯ
			const uint8_t& g = mode.dir[i];
			if (g < tree.genom.size) {
				// ЕСЛИ ЕСТЬ ВОЗМОЖНОСТЬ РАЗМНОЖИТСЯ
				int index_n = index + directions[i];
				if (world_map[index_n].type == air) {
					// РАЗМНОЖИТСЯ
					spawn(tree.genom.modes[g].type, index_n, index_tree, g);
				}
			}
		}



		become(index, wood, tree.color_deviation); // одеревенение 

	}


};