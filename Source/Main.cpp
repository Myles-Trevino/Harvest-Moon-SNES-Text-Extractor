/*
	Copyright 2021 Myles Trevino
	Licensed under the Apache License, Version 2.0
	http://www.apache.org/licenses/LICENSE-2.0
*/


#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <sstream>


constexpr int start{0x1B0200};
constexpr int end{0x1DBFF8};
constexpr int maximum_line_width{28};
constexpr int name_reference_token{0xFFFC};
constexpr int count_reference_token{0xFFFD};
constexpr int entry_end_token_1{0xFFFE};
constexpr int entry_end_token_2{0xFFFF};
constexpr int paragraph_separator{0xA2};


std::unordered_map<uint16_t, char> character_map
{
	{0x0, 'a'},
	{0x1, 'b'},
	{0x2, 'c'},
	{0x3, 'd'},
	{0x4, 'e'},
	{0x5, 'f'},
	{0x6, 'g'},
	{0x7, 'h'},
	{0x8, 'i'},
	{0x9, 'j'},
	{0xA, 'k'},
	{0xB, 'l'},
	{0xC, 'm'},
	{0xD, 'n'},
	{0xE, 'o'},
	{0xF, 'p'},
	{0x10, 'q'},
	{0x11, 'r'},
	{0x12, 's'},
	{0x13, 't'},
	{0x14, 'u'},
	{0x15, 'v'},
	{0x16, 'w'},
	{0x17, 'x'},
	{0x18, 'y'},
	{0x19, 'z'},

	{0x1A, 'A'},
	{0x1B, 'B'},
	{0x1C, 'C'},
	{0x1D, 'D'},
	{0x1E, 'E'},
	{0x1F, 'F'},
	{0x20, 'G'},
	{0x21, 'H'},
	{0x22, 'I'},
	{0x23, 'J'},
	{0x24, 'K'},
	{0x25, 'L'},
	{0x26, 'M'},
	{0x27, 'N'},
	{0x28, 'O'},
	{0x29, 'P'},
	{0x2A, 'Q'},
	{0x2B, 'R'},
	{0x2C, 'S'},
	{0x2D, 'T'},
	{0x2E, 'U'},
	{0x2F, 'V'},
	{0x30, 'W'},
	{0x31, 'X'},
	{0x32, 'Y'},
	{0x33, 'Z'},

	{0x34, '\''},
	{0x35, '!'},
	{0x36, '.'},
	{0x37, '?'},
	{0x38, '"'},
	{0x39, '/'},
	{0x3A, ','},
	{0x3B, '('},
	{0x3C, ')'},
	{0x3D, '%'},
	{0x3E, '-'},
	{0x3F, '$'},
	{0x40, '&'},
	{0x41, ':'},
	{0x42, '@'},

	{0xB1, ' '},

	{0xB2, '0'},
	{0xB3, '1'},
	{0xB4, '2'},
	{0xB5, '3'},
	{0xB6, '4'},
	{0xB7, '5'},
	{0xB8, '6'},
	{0xB9, '7'},
	{0xBA, '8'},
	{0xBB, '9'}
};

std::unordered_map<int, int> skip_map
{
	{211, 60},
	{419, 13},
	{651, 54},
	{855, 69}
};


// Main.
int main()
{
	// Load the file.
	std::ifstream input_file{"Harvest Moon.smc", std::ifstream::binary};
	if(!input_file) throw std::runtime_error{"Input file not found.\n"};
	input_file.seekg(start);

	std::ofstream output_file{"Output.txt"};

	// Read the file.
	uint16_t character_code;
	bool first_entry{true};
	bool new_entry{true};
	int entry_number{1};
	int line_width{};

	while(input_file.tellg() < end)
	{
		// Skip undesired areas.
		const auto skip_map_find_result{skip_map.find(entry_number)};

		if(new_entry && skip_map_find_result != skip_map.end())
			for(int i{}; i < skip_map_find_result->second; ++i)
				input_file.read(reinterpret_cast<char*>(&character_code), 2);

		// Read in the next character (two bytes).
		input_file.read(reinterpret_cast<char*>(&character_code), 2);
		++line_width;

		// Put a new entry marker if appropriate.
		if(new_entry && !first_entry) output_file<<"\n\n";
		if(new_entry) output_file<<"--- Entry "<<entry_number<<" ---\n";

		// If this is a reference token...
		while(character_code == name_reference_token || character_code == count_reference_token)
		{
			uint16_t id, reference_width;
			input_file.read(reinterpret_cast<char*>(&reference_width), 2);
			input_file.read(reinterpret_cast<char*>(&id), 2);

			for(int i{}; i < reference_width; ++i)
			{
				output_file<<"#";
				++line_width;
			}

			input_file.read(reinterpret_cast<char*>(&character_code), 2);
		}

		// If this is an entry end marker...
		if(character_code == entry_end_token_1 || character_code == entry_end_token_2)
		{
			if(character_code == entry_end_token_1)
				input_file.read(reinterpret_cast<char*>(&character_code), 2);

			++entry_number;
			new_entry = true;
			line_width = 0;
			continue;
		}

		// If this is a paragraph separator...
		if(character_code == paragraph_separator)
		{
			output_file<<"\n";
			line_width = 0;
			continue;
		}

		// Output the character if there is a mapping for it.
		const auto character_map_find_result{character_map.find(character_code)};
		if(character_map_find_result != character_map.end())
		{
			const char character{character_map_find_result->second};
			output_file<<character;
			new_entry = false;
			first_entry = false;
		}

		// Otherwise, if the character has no mapping...
		else
		{
			std::stringstream character_hex;
			character_hex<<"0x"<<std::hex<<std::uppercase<<character_code;
			output_file<<"<"<<character_hex.str()<<">";
			std::cout<<"Could not convert character "<<character_hex.str()<<".\n";
		}

		// If the line width has been exceeded...
		if(line_width >= maximum_line_width)
		{
			output_file<<"\n";
			line_width = 0;
		}
	}

	// Done message.
	std::cout<<"Done.";
}