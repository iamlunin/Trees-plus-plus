#pragma once

class WorldCS;

// ��� �����
class BlockType {
public:
	BlockType() {}
	std::string name;

	frac32 grow_price; // ��������� ��������� � �������� �����
	frac32 life_support_price; // ��������� ��������� ���������� � �������� ����� / ���
	frac32 light_absorption; // ����������� ���������� ����� ������ ��� �����������
	frac32 weight; // ��� �����
	frac32 durability; // ��������� �����


	uint8_t type; // ����� �����
	// ���� ������
	enum TPS { 
		STATIC_BLOCK, // �����, ����� � �����
		DYNAMIC_BLOCK, // �������� ������, ����
		GROW_BLOCK, // ��������� (�����), �����
		EXTRA_BLOCK // ������, �����
	};

	frac32 effectivity; // ������ ���������� ��� ������� �����

	frac32 probability_of_mutation_selection; // ������� ���� � ���� �������
};

// ������� ������
class WorldCS::Mode {
public:
	BlockType *own_transform_type;
	std::vector<BlockType *> other_transform_type;
	int own_genome_index;
	std::vector<int> other_genome_index;
};

// ��� ������
class WorldCS::Genome {
public:
	std::vector<Mode> mods;
};

// ��������� ��������� ������
class WorldCS::Bank {
public:
	std::vector<Particle> particles;
};

// �������
class WorldCS::Particle {
public:
	int current_mode_id;
	int map_index;
};

// ��������� �������
class WorldCS::FreeParticle {
public:
	Genome* current_mode;
};

// ������
class WorldCS::Trees {
public:
	std::vector<Bank> storges;
	Genome* genome;
};