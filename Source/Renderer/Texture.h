#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <string>
#include <vector>
#include <map>
#include <stb_image.h>

class TextureData
{
public:
	TextureData(std::string& path);
	virtual ~TextureData();

	bool LoadFromPath(std::string& path);

	inline void AddRefCount() { ref_count++; }
	inline void DelRefCount() { ref_count--; }
	inline uint32_t RefCount() { return ref_count; }

	inline std::string Path() { return tex_path; }

private:
	stbi_uc* pixels;
	int32_t tex_width;
	int32_t tex_height;
	int32_t tex_channel;

	uint32_t ref_count;

	std::string tex_path;	/// load from path
};

class Texture
{
public:
	Texture(std::string& path);
	virtual ~Texture();

	virtual bool InitWithPath(std::string& path);
	virtual void InitWithTextureData(TextureData* texData);

private:
	TextureData* tex_data;
	static std::map<std::string, TextureData*> texture_cache;
};

#endif // !__TEXTURE_H__
