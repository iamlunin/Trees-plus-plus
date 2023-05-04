#pragma once

// TODO

// сделать вероятность мутации генетически детерменированную
// добавить регулировку скорости
// сделать энергию
// сделать свет
// сделать настраиваемую консоль

// записать правила мутаций в более удобном виде
// сделать возможность отключения записи колор мапа для ускорения симуляции с выключенной отрисовкой



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

uint8_t r(int out, int to) { // от 0 до 255
	return gRAND.u8q() % (to - out) + out;
}



class Tree {
public:
	int age;
	int energy;
	bool alive;
	int cell_counter;
	uint32_t color_deviation = 0u;

	class Genom {
	public:
		std::vector<std::vector<uint8_t>> old_genom;

		ivec3 color = ivec3(gRAND.u8(), gRAND.u8(), gRAND.u8());
	};
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
		genom.color = clamp(genom.color + ivec3(vec3(gRAND.nf(), gRAND.nf(), gRAND.nf()) * vec3(20, 10, 5)), ivec3(0), ivec3(255));
		color_deviation = color_u32(ivec4(genom.color[0], genom.color[1], genom.color[2], 0u));
		for (int i = 0; i < mutation; i++) {
			int index1 = gRAND.u16() % genom.old_genom.size();
			int index2 = gRAND.u16() % genom.old_genom[index1].size();
			if (index1 == genom.old_genom.size() - 1)
				genom.old_genom[index1][index2] = r(1, 101);
			else if (index2 == (genom.old_genom[index1].size() - 2))
				genom.old_genom[index1][index2] = r(0, 2);
			else if (index2 == (genom.old_genom[index1].size() - 1))
				genom.old_genom[index1][index2] = r(0, 8);
			else
				genom.old_genom[index1][index2] = r(0, genom.old_genom.size() * 2);
		}
		_init();
	}

	void init(Genom& from_this) {
		genom = from_this;
		_init();
	}

	void init() {
		int s = 10; // размер генома
		// геном это массив
		// ген это вложенный в геном массив
		// каждая клетка имеет свой активный ген который определяет ее поведение
		// первые четыре числа каждого гена отвечают за то клетку с каким активным гЕном она отрастит в каждую из сторон
		// если число выходит за пределы размера генома, то новая клетка не будет отрощена
		// после отращивания клетка деревенеет
		// предпоследнее число каждого гена определяет тип клетки
		// последнее число определяет какой должен быть возраст клетки чтобы она приступила к отращиванию других клеток
		// последний ген является особым, он не содержит указатели на другие гены и тип клетки
		// последний ген определяет некоторые другие свойства дерева
		// первое число последнего гена это процент от максимально возможной величины жизни при котором дерево умрет
		genom.old_genom = {
			{r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,s * 2),semen,0},
			{r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,2),r(0,8)},
			{r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,2),r(0,8)},
			{r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,2),r(0,8)},
			{r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,2),r(0,8)},
			{r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,2),r(0,8)},
			{r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,2),r(0,8)},
			{r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,2),r(0,8)},
			{r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,2),r(0,8)},
			{r(1,101),r(0,s * 2),r(0,s * 2),r(0,s * 2),r(0,2),r(0,8)},

		};
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

		gRAND.ini(12280166710233847645);
		max_age = h * 2;
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
					trees.storage[c.index_tree].cell_counter--;
					become_live(c, index, green, trees.push(Tree(trees.storage[c.index_tree].genom, 1)));
					trees.storage[c.index_tree].cell_counter++;
					break;

					// СЕМЕНЬ УМИРАЕТ
				default:
					kill_cell(ind, c, index);
				}
			}
			else {
				// КЛЕТКА УМИРАЕТ
				if (trees.storage[c.index_tree].alive == false) {
					kill_cell(ind, c, index);
					continue;
				}

				// КЛЕТКА ПРОРОСТАЕТ
				if (c.type == green) {
					c.age++;
					if (c.age > trees.storage[c.index_tree].genom.old_genom[c.gen_index][5])
						try_grow(c.index_tree, index, c.gen_index);
				}
			}
		}
	}

	void become(Cell& cell, int index, int type) {
		static const uint32_t color_arr[] = {
				color_u32(ivec4(0, 0, 0, 255)),			// semen = 0,
				color_u32(ivec4(215, 103, 61, 255)),	// green = 1,
				color_u32(ivec4(0, 0, 0, 255)),	// wood = 2,
				color_u32(ivec4(255)),					// air = 3,
				color_u32(ivec4(60, 50, 40, 255))		// ground = 4,
		};

		if (type == air || type == ground)
			color_map_u32[index] = color_arr[type];
		else
			color_map_u32[index] = color_arr[type] + trees.storage[cell.index_tree].color_deviation;

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
			auto& tree = trees.storage[trees.enabled[i]];
			if (tree.age >= max_age * tree.genom.old_genom[tree.genom.old_genom.size() - 1][0] / 100 || tree.cell_counter <= 0) {
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
		trees.storage[cell.index_tree].cell_counter--;
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
		trees.storage[ind_tree].cell_counter++;
	}

	void try_grow(int index_tree, int index, int gen_index) {
		const auto& tree = trees.storage[index_tree];
		const auto& gen = tree.genom.old_genom[gen_index];
		// ПО КАЖДОМУ ИЗ НАПРАВЛЕНИЙ
		for (int i = 0; i < 4; i++) {
			// ЕСЛИ ЕСТЬ ЖЕЛАНИЕ РАЗМНОЖИТСЯ
			const auto& g = gen.at(i);
			if (g < tree.genom.old_genom.size() - 1) {
				// ЕСЛИ ЕСТЬ ВОЗМОЖНОСТЬ РАЗМНОЖИТСЯ
				int index_n = index + directions[i];
				if (world_map[index_n].type == air) {
					// РАЗМНОЖИТСЯ
					spawn(tree.genom.old_genom[g][4], index_n, index_tree, g);
				}
			}
		}



		become(world_map[index], index, wood); // одеревенение 

	}


};