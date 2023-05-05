#pragma once

// TODO

// сделать вероятность мутации генетически детерменированную
// сделать энергию
// сделать свет
// сделать настраиваемую консоль
// кнопка убить всех, ползунок упс, счетчик времени с последнего спавна
// записать правила мутаций в более удобном виде
// сделать раздельный вызов функций become




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
	wood = 2,
	air = 3,
	ground = 4,
};

uint8_t r8(int out, int to) { // от 0 до 255 
	return gRAND.u32q() % (to - out) + out;
}

float fr() {
	return gRAND.pf();
}


uint8_t generate_gen(int size) {
	return r8(0, size * 2);
}

uint8_t generate_type() {
	return r8(0, 2);
}

uint8_t generate_breeding_age() {
	return r8(0, 8);
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
		max_age = fr();
		color = ivec3(gRAND.u8(), gRAND.u8(), gRAND.u8());
		modes.resize(size, Mode(size));
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
	int energy;
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
		energy = 0;
		cell_counter = 0;
		alive = true;
	}
};



class Cell {
public:
	uint8_t type;
	uint8_t gen_index;
	int age;
	int index_tree = -1;


	Cell(int t) {
		type = t;
	}


};


class CellularAutomation {
public:
	std::vector<uint8_t> color_map;
	uint32_t* color_map_u32;

	PoolContainer<Tree> trees;
	std::vector<int> index_live_arr;
	int max_age;
	int max_cell;
	int max_semen;
	int width;
	int height, extended_height;
	int length;
	std::vector<Cell> world_map;
	int frame_count = 0;
	int great_spawn_counter = -1;
	enum { up, right, down, left };
	std::vector<int> directions;


	

	CellularAutomation(int w, int h) {

		color_map.resize(w * h * 4, 255);
		color_map_u32 = (uint32_t*)&color_map[0];

		gRAND.ini();
		max_cell = h / 2;
		max_age = h * 10;
		max_semen = 2;
		width = w;
		height = h, extended_height = h + 1;
		length = w * h;
		world_map.resize(width * extended_height, Cell(air));
	
		for (int ind = 0; ind < world_map.size(); ind++) {

			int x = (ind % width);
			int y = (ind / width);

			if (y < 1)
				become(world_map[ind], ind, ground);

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
		cells_handler();
		clean_up_index_live_arr();
		trees_handler();
		frame_count++;
	}

private:





	void cells_handler() {
		int old_length = index_live_arr.size();
		for (int ind = 0; ind < old_length; ind++) {
			int index = index_live_arr[ind];
			auto& c = world_map[index];

			// пофиксить эту строчку
			if (c.type == semen) {
				int index_d = index + directions[down];

				switch (world_map[index_d].type) {

					// СЕМЕНЬ ПАДАЕТ
				case air:
					index_live_arr[ind] = -index_live_arr[ind];
					become(c, index, air);
					index_live_arr.push_back(index_d);
					become_live(world_map[index_d], index_d, semen, c.index_tree, c.gen_index);
					break;

					// СЕМЕНЬ ПРОРОСТАЕТ
				case ground:
					trees[c.index_tree].cell_counter--;
					become_live(c, index, green, trees.push(Tree(trees[c.index_tree].genom, 1)));
					trees[c.index_tree].cell_counter++;
					break;

					// СЕМЕНЬ УМИРАЕТ
				default:
					kill_cell(ind, c, index);
				}
			}
			else {
				// КЛЕТКА УМИРАЕТ
				if (trees[c.index_tree].alive == false || trees[c.index_tree].cell_counter > max_cell) {
					kill_cell(ind, c, index);
					continue;
				}

				// КЛЕТКА ПРОРОСТАЕТ
				if (c.type == green) {
					c.age++;
					// можно попробовать пофиксить эту строчку
					if (c.age > trees[c.index_tree].genom[c.gen_index].breeding_age)
						try_grow(c.index_tree, index, c.gen_index);


				}
			}
		}
	}

	void become(Cell& cell, int index, int type) {
		static const uint32_t color_arr[] = {
				color_u32(ivec4(0, 0, 0, 255)),			// semen = 0,
				color_u32(ivec4(215, 103, 61, 255)),	// green = 1,
				color_u32(ivec4(0, 0, 0, 255)),	        // wood = 2,
				color_u32(ivec4(255)),					// air = 3,
				color_u32(ivec4(60, 50, 40, 255))		// ground = 4,
		};

		if (type != wood)
			color_map_u32[index] = color_arr[type];
		else
			color_map_u32[index] = color_arr[type] + trees[cell.index_tree].color_deviation;

		cell.type = type;
	}

	void become_live(Cell& cell, int index, int type, int index_tree) {
		become(cell, index, type);
		cell.index_tree = index_tree;
		cell.age = 0;
	}

	void become_live(Cell& cell, int index, int type, int index_tree, int gen_index) {
		become(cell, index, type);
		cell.gen_index = gen_index;
		cell.index_tree = index_tree;
		cell.age = 0;
	}


	void spawn_starting_seeds_if_needed() {
		// спавн новых семечек при пустом поле
		if (index_live_arr.size() == 0) {
			great_spawn_counter++;
			for (int i = 0; i < width / 2; i++)
				spawn(rand() % width, rand() % height / 2);
		}
	}

	void clean_up_index_live_arr() {
		// убрать умершие клетки из списка живых
		index_live_arr.erase(std::remove_if(index_live_arr.begin(), index_live_arr.end(),
			// если клетка не живая, в ней будет отрицательный индекс, равный положительному эквиваленту
			[&](const auto& a) { if (a < 0) { return true; } return false; }
		), index_live_arr.end());
	}


	void trees_handler() {
		for (int i = 0; i < trees.enabled.size(); i++) {
			auto& tree = trees[trees.enabled[i]];
			if (tree.age >= max_age * tree.genom.max_age || tree.cell_counter <= 0) {
				tree.alive = false;
				if (tree.cell_counter <= 0)
					trees.erase(i);
			}
			else
				tree.age++;
		}
		trees.erase();
	}


	void kill_cell(int index, Cell& cell, int index_world_map) {
		index_live_arr[index] = -index_live_arr[index];
		become(cell, index_world_map, air);
		trees[cell.index_tree].cell_counter--;
	}

	void try_spawn_semen(int ind) {
		if (world_map[ind].type != air)
			return;
		// спавним виртуальное дерево родитель
		spawn(semen, ind, trees.push(Tree()));
	}



	void spawn(int type, int ind, int ind_tree, int ind_gen = 0) {
		index_live_arr.push_back(ind);
		become_live(world_map[ind], ind, type, ind_tree, ind_gen);
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



		become(world_map[index], index, wood); // одеревенение 

	}


};