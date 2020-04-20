#include <exception>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
    vtf header fuzzer
    can cause crashes when players see sprays produced by this program
    https://github.com/destoer/vtf_fuzzer

*/


//https://developer.valvesoftware.com/wiki/Valve_Texture_Format
enum TextureFormat
{
	IMAGE_FORMAT_NONE = -1,
	IMAGE_FORMAT_RGBA8888 = 0,
	IMAGE_FORMAT_ABGR8888,
	IMAGE_FORMAT_RGB888,
	IMAGE_FORMAT_BGR888,
	IMAGE_FORMAT_RGB565,
	IMAGE_FORMAT_I8,
	IMAGE_FORMAT_IA88,
	IMAGE_FORMAT_P8,
	IMAGE_FORMAT_A8,
	IMAGE_FORMAT_RGB888_BLUESCREEN,
	IMAGE_FORMAT_BGR888_BLUESCREEN,
	IMAGE_FORMAT_ARGB8888,
	IMAGE_FORMAT_BGRA8888,
	IMAGE_FORMAT_DXT1,
	IMAGE_FORMAT_DXT3,
	IMAGE_FORMAT_DXT5,
	IMAGE_FORMAT_BGRX8888,
	IMAGE_FORMAT_BGR565,
	IMAGE_FORMAT_BGRX5551,
	IMAGE_FORMAT_BGRA4444,
	IMAGE_FORMAT_DXT1_ONEBITALPHA,
	IMAGE_FORMAT_BGRA5551,
	IMAGE_FORMAT_UV88,
	IMAGE_FORMAT_UVWQ8888,
	IMAGE_FORMAT_RGBA16161616F,
	IMAGE_FORMAT_RGBA16161616,
	IMAGE_FORMAT_UVLX8888
};



struct tagVTFHEADER
{
	char		signature[4];		// File signature ("VTF\0"). (or as little-endian integer, 0x00465456)
	unsigned int	version[2];		// version[0].version[1] (currently 7.2).
	unsigned int	headerSize;		// Size of the header struct  (16 byte aligned; currently 80 bytes) + size of the resources dictionary (7.3+).
	unsigned short	width;			// Width of the largest mipmap in pixels. Must be a power of 2.
	unsigned short	height;			// Height of the largest mipmap in pixels. Must be a power of 2.
	unsigned int	flags;			// VTF flags.
	unsigned short	frames;			// Number of frames, if animated (1 for no animation).
	unsigned short	firstFrame;		// First frame in animation (0 based).
	unsigned char	padding0[4];		// reflectivity padding (16 byte alignment).
	float		reflectivity[3];	// reflectivity vector.
	unsigned char	padding1[4];		// reflectivity padding (8 byte packing).
	float		bumpmapScale;		// Bumpmap scale.
	unsigned int	highResImageFormat;	// High resolution image format.
	unsigned char	mipmapCount;		// Number of mipmaps.
	unsigned int	lowResImageFormat;	// Low resolution image format (always DXT1).
	unsigned char	lowResImageWidth;	// Low resolution image width.
	unsigned char	lowResImageHeight;	// Low resolution image height.

	// 7.2+
	unsigned short	depth;			// Depth of the largest mipmap in pixels.
						            // Must be a power of 2. Is 1 for a 2D texture.
};

template<typename T>
void read_file(const std::string &filename, T *buf, size_t size)
{
    std::ifstream fp(filename,std::ios::binary);

    if(!fp)
    {
        auto err = "failed to open file: " + filename;
        std::cout << err;
        exit(1);
    }

    fp.read(reinterpret_cast<char*>(buf),size);

}


template<typename T>
void write_file(const std::string &filename, const T *data,size_t size)
{
    std::ofstream fp(filename,std::ios::binary);
	fp.write(reinterpret_cast<const char*>(data),size);
    fp.close();
}

size_t get_file_size(const std::string &filename)
{
    std::ifstream fp(filename);
    fp.seekg(0,fp.end);
	return fp.tellg();
}


bool is_set(int flag, int bit)
{
    return (flag >> bit) & 1;
}

int set_bit(int flag,int bit) noexcept
{
    return flag | (1 << bit);
}

int deset_bit(int flag,int bit) noexcept
{
    return flag & ~(1 << bit);
}

template<typename T>
bool in_range(T v, T l, T h)
{
    return (v <= h && v >= l);
}

int main(int argc,char *argv[])
{
    if(argc != 2)
    {
        printf("usage: %s <vtf file to inspect>",argv[0]);
    }

    const std::string filename = argv[1];

    tagVTFHEADER vtf_header;

    auto size = get_file_size(filename);
    read_file(filename,&vtf_header,sizeof(vtf_header));

    std::vector<uint8_t> buf(size);
    read_file(filename,buf.data(),size);
    

    auto header_buf = reinterpret_cast<tagVTFHEADER*>(buf.data());

    // print our header
    printf("size: %d\n",size);
    printf("Magic: %s\n",vtf_header.signature);
    printf("version %d.%d\n",vtf_header.version[0],vtf_header.version[1]);
    printf("header size %d\n",vtf_header.headerSize);
    printf("width %d\n",vtf_header.width);
    printf("height %d\n",vtf_header.height);
    printf("flags: %08x\n",vtf_header.flags);
    printf("frames %d\n",vtf_header.frames);
    printf("first frame %d\n",vtf_header.firstFrame);
    printf("reflectivity %f %f %f\n",vtf_header.reflectivity[0],vtf_header.reflectivity[1],vtf_header.reflectivity[2]);
    printf("bumpmapscale: %f\n",vtf_header.bumpmapScale);
    printf("highresimagefmt: %d\n",vtf_header.highResImageFormat);
    printf("mipmapcount %d\n",vtf_header.mipmapCount);
    printf("lowresimagefmt: %d\n",vtf_header.lowResImageFormat);
    printf("lowreswidth: %d\n",vtf_header.lowResImageWidth);
    printf("lowresheight: %d\n",vtf_header.lowResImageHeight);
    printf("depth: %d\n",vtf_header.depth);


    printf("point sampling: %d\n",is_set(vtf_header.flags,0));
    printf("trilinear sampling: %d\n",is_set(vtf_header.flags,1));
    printf("clamp s: %d\n",is_set(vtf_header.flags,2));
    printf("clamp t: %d\n",is_set(vtf_header.flags,3));
    printf("anisotropic sampling: %d\n",is_set(vtf_header.flags,4));
    printf("hint dxt5: %d\n",is_set(vtf_header.flags,5));
    printf("unknown: %d\n",is_set(vtf_header.flags,6));
    printf("normal map: %d\n",is_set(vtf_header.flags,7));
    printf("no mipmaps: %d\n",is_set(vtf_header.flags,8));
    printf("no level of detail: %d\n",is_set(vtf_header.flags,9));
    printf("no min mipmap: %d\n",is_set(vtf_header.flags,10));
    printf("procedural: %d\n",is_set(vtf_header.flags,11));
    printf("one bit alpha: %d\n",is_set(vtf_header.flags,12));
    printf("eight bit alpha: %d\n",is_set(vtf_header.flags,13));
    printf("env map: %d\n",is_set(vtf_header.flags,14));
    printf("render target: %d\n",is_set(vtf_header.flags,15));
    printf("depth render target: %d\n",is_set(vtf_header.flags,16));
    printf("no debug override: %d\n",is_set(vtf_header.flags,17));
    printf("single copy: %d\n",is_set(vtf_header.flags,18));
    printf("pre srgb: %d\n",is_set(vtf_header.flags,19));
    printf("premultiply color by one over mipmap level: %d\n",is_set(vtf_header.flags,20));
    printf("normal to dudv: %d\n",is_set(vtf_header.flags,21));
    printf("alpha test mipmap generation: %d\n",is_set(vtf_header.flags,22));
    printf("no depth buffer: %d\n",is_set(vtf_header.flags,23));
    printf("nice filtered: %d\n",is_set(vtf_header.flags,24));
    printf("clamp u: %d\n",is_set(vtf_header.flags,25));
    printf("vertex texture: %d\n",is_set(vtf_header.flags,26));
    printf("ssbump: %d\n",is_set(vtf_header.flags,27));
    printf("border: %d\n",is_set(vtf_header.flags,29));


    // fiddle the data and write it back out
    // a zerod out header will happily be loaded
    // even if the console complains about it a little 
    memset(header_buf,0,sizeof(tagVTFHEADER));
    header_buf->version[0] = 7; header_buf->version[1] = 2;
    header_buf->headerSize = 0x50;
    header_buf->frames = rand();
    header_buf->highResImageFormat = rand() % (IMAGE_FORMAT_UVLX8888 + 1);


    // DXT compressed textures must be a multiple of 4
    // cheers ficool2
    if(in_range<int>(header_buf->highResImageFormat,IMAGE_FORMAT_DXT1,IMAGE_FORMAT_DXT5))
    {
        header_buf->width = rand() & ~3;
        header_buf->height = rand() & ~ 3;
    }

    else
    {
        header_buf->width = rand();
        header_buf->height = rand();      
    }

    // width and height must be from 0 to 8192
    header_buf->width %= 8192;
    header_buf->height %= 8192;

    header_buf->flags = rand() % (1 << 30);
    header_buf->lowResImageFormat = IMAGE_FORMAT_DXT1;
    header_buf->mipmapCount = rand();
    header_buf->depth = rand();
    header_buf->bumpmapScale = rand();
    header_buf->reflectivity[0] = rand();
    header_buf->reflectivity[1] = rand();
    header_buf->reflectivity[2] = rand();
    strcpy(header_buf->signature,"VTF");

    write_file("test.vtf",buf.data(),size);

}