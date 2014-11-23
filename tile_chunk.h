#pragma once

#include <vector>
#include <string>
#include <functional>

enum ChunkType {
	Single, Group
};

struct Chunk {
private:
	std::string name;
	int width, height;

public:
	Chunk(const std::string& name, int w, int h);

	std::string get_name() const;
	int get_width() const;
	int get_height() const;

	virtual char tile(int x, int y) const = 0;
	virtual void tile(int x, int y, char value) = 0;
	virtual ChunkType type() = 0;
};

//A single chunk has a width and a height, and accessible tiles through a string.
struct SingleChunk : public Chunk {
private:
	std::string data;

public:
	SingleChunk(const std::string& name, const std::string& data, int w, int h);

	virtual char tile(int x, int y) const override;
	virtual void tile(int x, int y, char value) override;
	virtual ChunkType type() override;

	const std::string& get_data() const;
	void set_data(const std::string& data);
};

//A group of chunks is a group of chunks stacked on the y-axis.
//Height: Sum of chunk heights.
//Width: Taken of first chunk in vector.
struct GroupChunk : public Chunk {
private:
	std::vector<Chunk*> chunks;

public:
	GroupChunk(const std::vector<Chunk*>& chunks);

private:
	virtual void tileref(int x, int y, std::function<void(Chunk*, int x, int y)>);

public:
	virtual ChunkType type() override;
	virtual char tile(int x, int y) const override;
	virtual void tile(int x, int y, char value) override;

public:
	std::vector<Chunk*> get_chunks();
};

